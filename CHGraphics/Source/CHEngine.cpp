#include "CHEngine.h"
#include "CHWin32.h"
#include "Core\Math.h"
#include "OBJLoader.h"
#include "Lightning.h"

static float quadVertices[] = 
{
	-1,-1,0,0,1,
	-1,1,0,0,0,
	1,1,0,1,0,
	1,-1,0,1,1,
};

static uint16 quadIndices[] = 
{
	0,1,2,
	2,3,0
};

struct ComputeData
{
	uint32 startIndex;
	uint32 endIndex;
	float *ambientTexDiffData;
	float *ambientTexSpecData;
};

DWORD WINAPI ComputeAmbientBRDF(LPVOID param)
{
	ComputeData *data = (ComputeData *)param;
	uint32 startIndex = data->startIndex;
	uint32 endIndex = data->endIndex;
	for (uint32 NV = startIndex; NV < endIndex; ++NV)
	{
		float nv = NV / 50.0f;
		if (NV == 0)
		{
			nv = 0.001f;
		}
		for (uint32 metal = 0; metal <= 50; ++metal)
		{
			float met = metal / 50.0f;
			if (metal == 0)
			{
				met = 0.001f;
			}
			for (uint32 rough = 0; rough <= 200; ++rough)
			{
				float roughness = rough / 50.0f;
				if (rough == 0)
				{
					roughness = 0.001f;
				}
				Vector2 val = Lightning::CalculateAmbientBRDF(nv, met, roughness);
				data->ambientTexDiffData[NV * 51 * 201 + metal * 201 + rough] = val.x;
				data->ambientTexSpecData[NV * 51 * 201 + metal * 201 + rough] = val.y;
			}
		}
	}
	return 0;
}

struct SceneData
{
	Matrix4x4 projection;
	Matrix4x4 view;
	Vector3 eyePos;
};

Engine CHEngine::GetEngine(Window *window)
{
	Engine result;
	result.window = window;

	result.solidState = CHGraphics::GetBlendState(window, BLEND_SOLID);
	result.transparentState = CHGraphics::GetBlendState(window, BLEND_TRANSPARENT);

	result.sceneBuffer = CHGraphics::GetConstantBuffer(window, sizeof(SceneData));
	result.modelBuffer = CHGraphics::GetConstantBuffer(window, sizeof(ModelData) * 600);
	result.lightBuffer = CHGraphics::GetConstantBuffer(window, sizeof(Light) * 2);
	result.shadowVPBuffer = CHGraphics::GetConstantBuffer(window, sizeof(Matrix4x4));
	result.materialBuffer = CHGraphics::GetConstantBuffer(window, sizeof(Material));
	result.fxaaBuffer = CHGraphics::GetConstantBuffer(window, sizeof(Vector4));
	Vector4 fxaaData = Vector4(1.0f / window->width, 1.0f / window->height, 0, 0);
	CHGraphics::BindConstantBuffer(window, &result.fxaaBuffer, &fxaaData, 10);

	result.renderPass = CHGraphics::GetProgram(window, L"RenderPassVS.cso", L"RenderPassPS.cso",
													VERTEX_INPUT_POSITION_NORMAL);
	result.fxaaPass = CHGraphics::GetProgram(window, L"QuadVertexShader.cso", L"FXAAPixelShader.cso", 
											 VERTEX_INPUT_POSITION_TEXCOORD);
	result.passthroughPass = CHGraphics::GetProgram(window, L"QuadVertexShader.cso", L"TextureDisplayPixelShader.cso",
												 VERTEX_INPUT_POSITION_TEXCOORD);
	result.shadowMapPassDirection = CHGraphics::GetProgram(window, L"ShadowMapDirVS.cso", NULL, VERTEX_INPUT_POSITION_NORMAL);
	result.shadowMapPassPoint = CHGraphics::GetProgram(window, L"ShadowMapPointVS.cso", L"ShadowMapPointPS.cso", 
													  VERTEX_INPUT_POSITION_NORMAL);

	result.quad = CHGraphics::GetMesh(window, quadVertices, 5 * sizeof(float), 4, quadIndices, 6);


	for (uint32 i = 0; i < ARRAYSIZE(result.modelData); ++i)
	{
		CHList::Reset(&result.modelData[i]);
	}

	float *ambientTexDiffData = new float[51 * 51 * 201];
	float *ambientTexSpecData = new float[51 * 51 * 201];
	//#define PRECOMPUTE
#ifdef PRECOMPUTE
	HANDLE threadArray[4];
	ComputeData cmpData1 = { 0,13,ambientTexDiffData, ambientTexSpecData };
	threadArray[0] = CreateThread(0, 0, ComputeAmbientBRDF, &cmpData1, 0, NULL);
	ComputeData cmpData2 = { 13,26,ambientTexDiffData, ambientTexSpecData };
	threadArray[1] = CreateThread(0, 0, ComputeAmbientBRDF, &cmpData2, 0, NULL);
	ComputeData cmpData3 = { 26,39,ambientTexDiffData, ambientTexSpecData };
	threadArray[2] = CreateThread(0, 0, ComputeAmbientBRDF, &cmpData3, 0, NULL);
	ComputeData cmpData4 = { 39,51,ambientTexDiffData, ambientTexSpecData };
	threadArray[3] = CreateThread(0, 0, ComputeAmbientBRDF, &cmpData4, 0, NULL);

	WaitForMultipleObjects(4, threadArray, TRUE, INFINITE);

	Win32FileWrite(L"diffMap", ambientTexDiffData, 51 * 51 * 201 * sizeof(float));
	Win32FileWrite(L"specMap", ambientTexSpecData, 51 * 51 * 201 * sizeof(float));
#endif
	FileData diff = Win32ReadFile(L"diffMap");
	memcpy(ambientTexDiffData, diff.data, diff.size);
	Win32ReleaseFile(diff);
	FileData spec = Win32ReadFile(L"specMap");
	memcpy(ambientTexSpecData, spec.data, spec.size);
	Win32ReleaseFile(spec);

	result.ambientTexDiff = CHGraphics::GetTexture3D(window, 201, 51, 51, ambientTexDiffData);
	result.ambientTexSpec = CHGraphics::GetTexture3D(window, 201, 51, 51, ambientTexSpecData);
	CHGraphics::BindTexture3D(window, &result.ambientTexDiff, 8);
	CHGraphics::BindTexture3D(window, &result.ambientTexSpec, 7);

	return result;
}

Scene3D CHEngine::GetScene(Engine *engine)
{
	Window *window = engine->window;
	Scene3D scene;
	scene.camera = NULL;

	scene.colorBuffer = CHGraphics::GetColorBuffer(window, window->width, window->height);
	scene.depthBuffer = CHGraphics::GetDepthBuffer(window, window->width, window->height);

	scene.shadowMapBufferDirection = CHGraphics::GetDepthBuffer(window, 2048, 2048);
	scene.shadowMapBufferPoint = CHGraphics::GetDepthBuffer(window, 2048, 2048);
	scene.colorCube = CHGraphics::GetColorBufferCubeMap(window, 2048, 2048);

	scene.light.intensity = 0.0f;
	scene.pointLight.intensity = 0.0f;

	CHList::Reset(&scene.entityList);
	return scene;
}

template <int N>
uint32 ProcessEntities(Engine *engine, List<Entity, N> *eList)
{
	uint32 meshCounter = 0;
	for (uint32 i = 0; i < eList->length; ++i)
	{
		Entity *entity = &eList->items[i];
		Matrix4x4 modelMatrix = Math::GetTranslation(entity->position.x, entity->position.y, entity->position.z) *
			Math::GetScale(entity->scale, entity->scale, entity->scale);
		ModelData newModel = { entity->material, modelMatrix };
		bool alreadyUsed = false;
		for (uint32 i = 0; i < ARRAYSIZE(engine->modelData) && i < meshCounter; ++i)
		{
			if (entity->mesh.vertexBuffer == engine->meshes[i].vertexBuffer)
			{
				if (entity->transparent)
				{
					CHList::Add(&engine->modelDataTransparent[i], &newModel);
				}
				else
				{
					CHList::Add(&engine->modelData[i], &newModel);
				}

				if (entity->castsShadow)
				{
					CHList::Add(&engine->modelDataShadows[i], &newModel);
				}
				alreadyUsed = true;
			}
		}
		if (!alreadyUsed)
		{
			if (entity->transparent)
			{
				CHList::Add(&engine->modelDataTransparent[i], &newModel);
			}
			else
			{
				CHList::Add(&engine->modelData[i], &newModel);
			}
			if (entity->castsShadow)
			{
				CHList::Add(&engine->modelDataShadows[i], &newModel);
			}
			engine->meshes[meshCounter++] = entity->mesh;
		}
	}
	return meshCounter;
}

template <int N>
void RenderEntityList(Window *window, Engine *engine, List<Entity, N> *entityList, uint32 modelCount)
{
	for (uint32 i = 0; i < modelCount; ++i)
	{
		CHGraphics::BindConstantBuffer(window, &engine->modelBuffer, entityList[i].items, 2);
		CHGraphics::RenderMeshInstanced(window, &engine->meshes[i], entityList[i].length);
	}
}


void SetSceneDataFromCamera(Engine *engine, CameraPolar *camera)
{
	SceneData sceneData = { CHEngine::GetProjectionFromCamera(camera),
							CHEngine::GetViewFromCamera(camera),
							CHEngine::GetPositionFromCamera(camera) };
	CHGraphics::BindConstantBuffer(engine->window, &engine->sceneBuffer, &sceneData, SCENE_SLOT);
}

void SetSceneDataFromCamera(Engine *engine, Camera *camera)
{
	SceneData sceneData = { CHEngine::GetProjectionFromCamera(camera),
		CHEngine::GetViewFromCamera(camera),
		CHEngine::GetPositionFromCamera(camera) };
	CHGraphics::BindConstantBuffer(engine->window, &engine->sceneBuffer, &sceneData, SCENE_SLOT);
}

void CHEngine::PresentScene(Engine *engine, Scene3D *scene)
{
	Window *window = engine->window;
	CameraPolar *camera = scene->camera;
	if (!camera)
	{
		return;
	}
	
	List<Entity, 1000> *eList = &scene->entityList;
	uint32 meshCount = ProcessEntities(engine, eList);

	Camera shadowCam = CHEngine::GetCamera(engine);
	shadowCam.position = scene->light.position;
	shadowCam.direction = -scene->light.position;

	{
		CHGraphics::ClearDepthBuffer(window, &scene->shadowMapBufferDirection);
		CHGraphics::BindTarget(window, &scene->shadowMapBufferDirection);
		CHGraphics::BindProgram(window, &engine->shadowMapPassDirection);

		SetSceneDataFromCamera(engine, &shadowCam);
		RenderEntityList(window, engine, (List<Entity, 1000> *)engine->modelDataShadows, meshCount);
	}

	{
		CHGraphics::ClearColorBufferCube(window, &scene->colorCube, 100 * 100, 100, 100, 100);
		CHGraphics::BindProgram(window, &engine->shadowMapPassPoint);

		Vector3 dirs[6] = {Vector3(1,0,0), Vector3(-1,0,0), Vector3(0,1,0), Vector3(0,-1,0), Vector3(0,0,1), Vector3(0,0,-1)};
		Vector3 ups[6] = { Vector3(0,1,0), Vector3(0,1,0), Vector3(0,0,-1), Vector3(0,0,1), Vector3(0,1,0), Vector3(0,1,0) };

		Camera cubeMapCam = {};
		cubeMapCam.projection = Math::GetPerspectiveProjectionDXLH(90 * PI / 180.0f, 1, .1f, 90.0f);
		cubeMapCam.position = scene->pointLight.position;
		for (uint32 i = 0; i < 6; ++i)
		{
			CHGraphics::ClearDepthBuffer(window, &scene->shadowMapBufferPoint);
			CHGraphics::BindTarget(window, &scene->colorCube, i, &scene->shadowMapBufferPoint);

			cubeMapCam.direction = dirs[i];
			SetSceneDataFromCamera(engine, &cubeMapCam);
			RenderEntityList(window, engine, (List<Entity, 1000> *)engine->modelDataShadows, meshCount);
		}
	}

	{
		CHGraphics::ClearColorBuffer(window, &scene->colorBuffer, 1, 1, 1, 1);
		CHGraphics::ClearDepthBuffer(window, &scene->depthBuffer);
		CHGraphics::BindTarget(window, &scene->colorBuffer, &scene->depthBuffer);
		CHGraphics::BindProgram(window, &engine->renderPass);

		CHGraphics::BindTexture2D(window, &scene->shadowMapBufferDirection, SHADOWMAP_DIR_SLOT);
		CHGraphics::BindTextureCubeMap(window, &scene->colorCube, SHADOWMAP_POINTCUBE_SLOT);

		Light lights[2] = { scene->light, scene->pointLight };
		CHGraphics::BindConstantBuffer(window, &engine->lightBuffer, &lights, LIGHT_SLOT);

		Matrix4x4 shadowVP = CHEngine::GetProjectionFromCamera(&shadowCam) * CHEngine::GetViewFromCamera(&shadowCam);
		CHGraphics::BindConstantBuffer(window, &engine->shadowVPBuffer, &shadowVP, SHADOW_VP_SLOT);

		SetSceneDataFromCamera(engine, scene->camera);

		RenderEntityList(window, engine, (List<Entity, 1000> *)engine->modelData, meshCount);

		CHGraphics::SetBlendState(window, &engine->transparentState);
		CHGraphics::TurnOffDepthWrite(window);

		RenderEntityList(window, engine, (List<Entity, 1000> *)engine->modelDataTransparent, meshCount);

		CHGraphics::UnbindTarget(window);
		CHGraphics::SetBlendState(window, &engine->solidState);
		CHGraphics::TurnOnDepthWrite(window);

		CHGraphics::UnbindTexture2D(window, SHADOWMAP_DIR_SLOT);
		CHGraphics::UnbindTexture2D(window, SHADOWMAP_POINTCUBE_SLOT);
	}

	{
		CHGraphics::UnbindTarget(window);
		CHGraphics::BindTexture2D(window, &scene->colorBuffer, 0);
		CHEngine::BlitToTexture(window, engine, &engine->fxaaPass);
		CHGraphics::UnbindTexture2D(window, MAIN_TEX_SLOT);
	}

	CHList::Reset(&scene->entityList);
	for (uint32 i = 0; i < ARRAYSIZE(engine->modelData); ++i)
	{
		CHList::Reset(&engine->modelData[i]);
		CHList::Reset(&engine->modelDataShadows[i]);
		CHList::Reset(&engine->modelDataTransparent[i]);
	}
}

CameraPolar CHEngine::GetCameraPolar(Engine *engine)
{
	CameraPolar camera;
	camera.distance = 90.0f;
	camera.polar = 1.0f; 
	camera.azimuth = 0.0f;
	camera.projection = 
		Math::GetPerspectiveProjectionDXLH(45 * PI / 180.0f, (float)engine->window->width / (float)engine->window->height,
		2.0f, 190.0f);
	return camera;
}

Camera CHEngine::GetCamera(Engine *engine)
{
	Camera camera;
	camera.position = Vector3(0, 0, 0);
	camera.direction = Vector3(0, 0, 1);
	camera.projection = Math::GetOrthographicsProjectionLH(-50, 50, -50, 50, -100, 100);
		
	return camera;
}


Matrix4x4 CHEngine::GetViewFromCamera(CameraPolar *camera)
{
	Vector3 eyePosition = Vector3(Math::Cos(camera->azimuth) * Math::Sin(camera->polar) * camera->distance,
								  Math::Cos(camera->polar) * camera->distance,
								  Math::Sin(camera->azimuth) * Math::Sin(camera->polar) * camera->distance);
	Matrix4x4 result = Math::GetScale(0.2f, 0.2f, 0.2f) * Math::LookAtLH(eyePosition, Vector3(0, 0, 0), Vector3(0, 1, 0));
	return result;
}

Matrix4x4 CHEngine::GetViewFromCamera(Camera *camera)
{
	Matrix4x4 result = Math::LookAtLH(camera->position, camera->position + camera->direction, Vector3(0, 1, 0));
	return result;
}

Matrix4x4 CHEngine::GetProjectionFromCamera(CameraPolar *camera)
{
	Matrix4x4 result = camera->projection;
	return result;
}

Matrix4x4 CHEngine::GetProjectionFromCamera(Camera *camera)
{
	Matrix4x4 result = camera->projection;
	return result;
}

Vector3 CHEngine::GetPositionFromCamera(CameraPolar *camera)
{
	Vector3 result = Vector3(Math::Cos(camera->azimuth) * Math::Sin(camera->polar) * camera->distance,
							 Math::Cos(camera->polar) * camera->distance,
							 Math::Sin(camera->azimuth) * Math::Sin(camera->polar) * camera->distance);
	return result;
}

Vector3 CHEngine::GetPositionFromCamera(Camera *camera)
{
	Vector3 result = camera->position;
	return result;
}

void CHEngine::UpdateCamera(CameraPolar *camera, Input *input)
{
	Vector2 inputDelta = input->touchPosition - input->touchPositionPrev;
	if (input->leftButtonDown)
	{
		camera->azimuth -= inputDelta.x / 50.0f;
		camera->polar -= inputDelta.y / 50.0f;
	}
	camera->distance -= input->wheelDelta / 24.f;
}

Material CHEngine::GetMaterial(Color color, float metalness, float roughness)
{
	Material result;
	Assert(color.r >= 0 && color.r <= 1.0f);
	Assert(color.g >= 0 && color.g <= 1.0f);
	Assert(color.b >= 0 && color.b <= 1.0f);
	Assert(color.a >= 0 && color.a <= 1.0f);
	result.diffuseColor = color;
	result.metalness = metalness;
	result.roughness = roughness;
	return result;
}

Mesh CHEngine::GetMeshFromFile(Window *window, wchar_t *fileName)
{
	FileData sphereFile = Win32ReadFile(fileName);
	UINT vertexCount = 0;
	UINT indexCount = 0;
	OBJLoader::GetSizes(&sphereFile, &vertexCount, &indexCount);
	float *vertexData = new float[vertexCount * 6];
	UINT16 *indexData = new UINT16[indexCount];
	OBJLoader::Load(&sphereFile, vertexData, indexData, POSITION_NORMAL, 6 * sizeof(float), 0);

	Mesh model = CHGraphics::GetMesh(window, vertexData, 6 * sizeof(float), vertexCount,
									 indexData, indexCount);
	Win32ReleaseFile(sphereFile);
	return model;
}


void CHEngine::RenderSceneEntity(Scene3D *scene, Entity *entity)
{
	CHList::Add(&scene->entityList, entity);
}

Light CHEngine::GetLight(Vector3 position, float intensity)
{
	Light light = {position, intensity};
	return light;
}

void CHEngine::UseDirectionalLight(Scene3D *scene, Light *light)
{
	scene->light = *light;
}

void CHEngine::UsePointLight(Scene3D *scene, Light *light)
{
	scene->pointLight = *light;
}


void CHEngine::SetSceneCamera(Scene3D *scene, CameraPolar *camera)
{
	scene->camera = camera;
}

void CHEngine::BlitToTexture(Window *window, Engine *engine, Program *program)
{
	CHGraphics::ClearWindow(window, 1.0f, 0.0f, 0.0f, 1);
	CHGraphics::BindTargetWindow(window);
	if (program)
	{
		CHGraphics::BindProgram(window, program);
	}
	else
	{
		CHGraphics::BindProgram(window, &engine->passthroughPass);
	}

	CHGraphics::RenderMesh(window, &engine->quad);
}

void CHEngine::BlitToTexture(Window *window, Engine *engine, ColorBuffer *buffer, Program *program)
{
	CHGraphics::ClearColorBuffer(window, buffer, 0.0f, 0.0f, 0.0f, 1);
	CHGraphics::BindTarget(window, buffer);
	if (program)
	{
		CHGraphics::BindProgram(window, program);
	}
	else
	{
		CHGraphics::BindProgram(window, &engine->passthroughPass);
	}

	CHGraphics::RenderMesh(window, &engine->quad);
}

void CHEngine::Release(Scene3D *scene)
{
	CHGraphics::Release(&scene->colorBuffer);
	CHGraphics::Release(&scene->depthBuffer);
	CHGraphics::Release(&scene->shadowMapBufferDirection);
	CHGraphics::Release(&scene->shadowMapBufferPoint);
	CHGraphics::Release(&scene->colorCube);

	CHList::Reset(&scene->entityList);
	scene->camera = NULL;
}

void CHEngine::Release(Engine *engine)
{	
	CHGraphics::Release(&engine->modelBuffer);
	CHGraphics::Release(&engine->sceneBuffer);
	CHGraphics::Release(&engine->materialBuffer);
	CHGraphics::Release(&engine->lightBuffer);
	CHGraphics::Release(&engine->fxaaBuffer);
	CHGraphics::Release(&engine->shadowVPBuffer);

	CHGraphics::Release(&engine->renderPass);
	CHGraphics::Release(&engine->shadowMapPassDirection);
	CHGraphics::Release(&engine->shadowMapPassPoint);
	CHGraphics::Release(&engine->fxaaPass);
	CHGraphics::Release(&engine->passthroughPass);

	CHGraphics::Release(&engine->quad);

	CHGraphics::Release(&engine->solidState);
	CHGraphics::Release(&engine->transparentState);

	CHGraphics::Release(&engine->ambientTexDiff);
	CHGraphics::Release(&engine->ambientTexSpec);

	engine->window = NULL;
}

void CHEngine::ResetInput(Input *input)
{
	input->pressedS = false;
	input->wheelDelta = 0;
	input->touchPositionPrev = input->touchPosition;
}