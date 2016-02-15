#include <windows.h>
#include "CHDef.h"
#include "CHEngine.h"
#include <windowsx.h>
#include <d3d11.h>
#include "Win32Clock.h"
#include "CHWin32.h"
#include "OBJLoader.h"
#include "CHGraphics.h"
#include "Core\Math.h"

#define _CRTDBG_MAP_ALLOC
//#define FULLSCREEN
#ifdef FULLSCREEN
#define SCREEN_X 1920
#define SCREEN_Y 1080
#elif defined FULLSCREEN_SMALL
#define SCREEN_X 1366
#define SCREEN_Y 768
#define FULLSCREEN
#else
#define SCREEN_X 800
#define SCREEN_Y 600
#endif


bool isRunning = false;

void DebugLiveObjects(ID3D11Device *device);



float rough = 0.00001f;
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	Window window = CHGraphics::GetWindow(hInstance, SCREEN_X, SCREEN_Y);
	Engine engine = CHEngine::GetEngine(&window);
	Scene3D scene = CHEngine::GetScene(&engine);

	Mesh model = CHEngine::GetMeshFromFile(&window, L"sphere.object");
	Mesh floor = CHEngine::GetMeshFromFile(&window, L"plane.object");
	Mesh cube = CHEngine::GetMeshFromFile(&window, L"cube.object");

	CameraPolar camera = CHEngine::GetCameraPolar(&engine);
	CHEngine::SetSceneCamera(&scene, &camera);
	Clock clock;
	Win32Clock::Init(&clock);

	Vector3 *positions = new Vector3[590];
	Vector3 *velocity = new Vector3[590];
	Color *colors = new Color[590];
	float *temp = new float[590];
	float *weight = new float[590];
	float *scales = new float[590];

	Vector3 *rockPos = new Vector3[15];
	Vector3 *rockVel = new Vector3[15];
	float *rockSize = new float[15];

	Vector3 *smokePos = new Vector3[150];
	float *smokeSize = new float[150];
	uint32 smokeCounters[15] = { 0 };

	for (int i = 0; i < 15; ++i)
	{
		rockPos[i] = Vector3(0, 0, 0);
		float randomPolar = (rand() % 1000) / 1000.0f * (PIHALF - 0.8f) + 0.4f;
		float randomAzimuth = (rand() % 1000) / 1000.0f * PI2;
		float randomSpeed = (rand() % 1000) / 1000.0f * 280.0f + 40.0f;
		rockVel[i] = Vector3(Math::Cos(randomAzimuth) * Math::Sin(randomPolar), Math::Cos(randomPolar), Math::Sin(randomAzimuth) * Math::Sin(randomPolar)) * randomSpeed;
		rockSize[i] = (rand() % 1000) / 1000.0f * .5f + 0.2f;
		smokeCounters[i] = 0;
	}
	for (int i = 0; i < 590; ++i)
	{
		positions[i] = Vector3(0, 0, 0);
		float randomPolar = (rand() % 1000) / 1000.0f * PIHALF;
		float randomAzimuth = (rand() % 1000) / 1000.0f * PI2;
		float randomSpeed = (rand() % 1000) / 1000.0f * 250.0f + 1.0f;
		colors[i] = Color(1.0f, 0, 0, 1.0f);
		colors[i].g = (rand() % 1000) / 1000.0f * 1.0f;// * 0.1f;
		temp[i] = 1.0f;
		velocity[i] = Vector3(Math::Cos(randomAzimuth) * Math::Sin(randomPolar), Math::Cos(randomPolar), Math::Sin(randomAzimuth) * Math::Sin(randomPolar)) * randomSpeed;
		weight[i] = (rand() % 1000) / 1000.0f;
		weight[i] *= weight[i];
		scales[i] = (rand() % 1000) / 1000.0f * 3.5f + 1.0f;
	}
	for (int i = 0; i < 150; ++i)
	{
		smokeSize[i] = 0.0f;
	}

	float time = 0;

	Vector3 vortex = Vector3(0, 1, 0);
	Vector3 pos = Vector3(80, 80, 0);
	Vector3 vel = Vector3(-100.0f, -100.0f, 0.0f);

	Input input = { 0 };
	bool spawn = false;
	while (window.running)
	{
		float dt = Win32Clock::GetDeltaTime(&clock);
		CHGraphics::ProcessEvents(&window, &input);
				
		if (input.pressedS)
		{
			for (int i = 0; i < 15; ++i)
			{
				rockPos[i] = Vector3(0, 0, 0);
				float randomPolar = (rand() % 1000) / 1000.0f * (PIHALF - 0.8f) + 0.4f;
				float randomAzimuth = (rand() % 1000) / 1000.0f * PI2;
				float randomSpeed = (rand() % 1000) / 1000.0f * 280.0f + 40.0f;
				rockVel[i] = Vector3(Math::Cos(randomAzimuth) * Math::Sin(randomPolar), Math::Cos(randomPolar), Math::Sin(randomAzimuth) * Math::Sin(randomPolar)) * randomSpeed;
				rockSize[i] = (rand() % 1000) / 1000.0f * .5f + 0.2f;
				smokeCounters[i] = 0;
			}
			for (int i = 0; i < 590; ++i)
			{
				positions[i] = Vector3(0, 0, 0);
				float randomPolar = (rand() % 1000) / 1000.0f * PIHALF;
				float randomAzimuth = (rand() % 1000) / 1000.0f * PI2;
				float randomSpeed = (rand() % 1000) / 1000.0f * 250.0f + 1.0f;
				colors[i] = Color(1.0f, 0, 0, 1.0f);
				colors[i].g = (rand() % 1000) / 1000.0f * 1.0f;// * 0.1f;
				temp[i] = 1.0f;
				velocity[i] = Vector3(Math::Cos(randomAzimuth) * Math::Sin(randomPolar), Math::Cos(randomPolar), Math::Sin(randomAzimuth) * Math::Sin(randomPolar)) * randomSpeed;
				weight[i] = (rand() % 1000) / 1000.0f;
				weight[i] *= weight[i];
				scales[i] = (rand() % 1000) / 1000.0f * 3.5f + 1.0f;
			}
			for (int i = 0; i < 150; ++i)
			{
				smokeSize[i] = 0.0f;
			}
			time = 0;
			vortex = Vector3(0,1,0);
			pos = Vector3(80, 80, 0);
			vel = Vector3(-100.0f, -100.0f, 0.0f);
		}

		
		Light light1 = CHEngine::GetLight(Vector3(10, 50, 0), 3000.0f);//
		CHEngine::UseDirectionalLight(&scene, &light1);
		CHEngine::UpdateCamera(&camera, &input);

		Color floorColor = Color(1.f, 1.0f, 1.0f, 1);
		Material floorMat = { floorColor, 0.01f, 3.0f };
		Vector3 floorPos = Vector3(0,0,0);
		Entity floorEntity = {floor, floorMat, floorPos, 1.0f, true};
		CHEngine::RenderSceneEntity(&scene, &floorEntity);

		Color cubeColor2 = Color(.1f, .1f, .1f, 1.0f);
		Material cubeMat2 = { cubeColor2, 1.0f, 0.1f};
		Vector3 cubePos2 = Vector3(-1, 1, 0);
		Entity cubeEntity2 = { model, cubeMat2, cubePos2, 1.0f, true};
		Vector3 avgPosition = Vector3(0,0,0);
		float count = 0;
		if (pos.y < 0)
		{
			time += dt;

			Light pointLight = { Vector3(0,2,0), Math::Clamp(2000.0f * Math::ExponentialFunction(1, -3, time), 0, 1000000) };
			CHEngine::UsePointLight(&scene, &pointLight);

			for (int32 i = 0; i < 15; ++i)
			{
				Color color = Color(1.0f,0.8f,0,1);
				cubeEntity2.material.diffuseColor = color;
				Vector3 drag = -2.2f * Math::Length(rockVel[i]) * Math::Normalize(rockVel[i]);
				rockVel[i] += Vector3(0,-9.8f,0) * dt * 2.0f + drag * dt;
				rockPos[i] += rockVel[i] * dt;
				if (rockPos[i].y <= 0)
				{
					rockVel[i].y = -(rockVel[i].y * 0.6f);
				}
				cubeEntity2.position = rockPos[i];
				cubeEntity2.scale = rockSize[i];
				CHEngine::RenderSceneEntity(&scene, &cubeEntity2);
			}

			for (int32 i = 0; i < 150; ++i)
			{
				Vector3 distanceToVortex = vortex - positions[i];
				Vector3 fire = temp[i] * 1.0f * Vector3(0,1,0)+ Math::Normalize(distanceToVortex) * 15000.0f * temp[i] / (Math::Length(distanceToVortex)); 
				count += weight[i];
				fire *= weight[i];// * weight[i];
				avgPosition += positions[i] * weight[i];
				Vector3 drag = -12.0f * Math::Length(velocity[i]) * Math::Normalize(velocity[i]);
				Vector3 a = drag + fire;
				a += Vector3(0,-9.8f,0) * weight[i] + Vector3(0,-2.0f, 0);
				velocity[i] += a * dt;
				positions[i] += velocity[i] * dt;// * (Math::ExponentialFunction(0.4f, -4.0f, time));
				temp[i] = Math::Clamp(1.0f - (positions[i].y - (vortex.y - 2.0f)) / 2.0f, 0, 3);
				cubeEntity2.position = positions[i];
				cubeEntity2.scale = scales[i];

				Color color = colors[i] * Math::ExponentialFunction(1.0f, -3.0f, time);// - 2.0f);
				color.a = Math::Clamp(Math::ExponentialFunction(1.0f, -1.0f, time - 1.5f), 0, 1);
				cubeEntity2.material.diffuseColor = color;
				cubeEntity2.castsShadow = false;
				cubeEntity2.transparent = true;
				CHEngine::RenderSceneEntity(&scene, &cubeEntity2);
			}
			avgPosition = avgPosition / count;
			vortex = avgPosition;
		}
		vel += Vector3(0,-9.8, 0) * dt;
		//if (time > 2.0f)
		{
			pos += vel * dt;
		}

		cubeEntity2.castsShadow = true;
		cubeEntity2.transparent = false;
		cubeEntity2.position = Vector3(-8,2,-5);//pos;
		cubeEntity2.scale = 2.0f;
		cubeEntity2.material.diffuseColor = Color(1,0,1,1);
		CHEngine::RenderSceneEntity(&scene, &cubeEntity2);

		cubeEntity2.position = Vector3(0, 2, 6);//pos;
		cubeEntity2.scale = 2.0f;
		cubeEntity2.material.diffuseColor = Color(0, 0, 0, 1);
		cubeEntity2.material.metalness = 0.1f;
		cubeEntity2.material.roughness = 1.0f;
		CHEngine::RenderSceneEntity(&scene, &cubeEntity2);

		cubeEntity2.position = Vector3(4, 2, 2);//pos;
		cubeEntity2.scale = 2.0f;
		cubeEntity2.material.diffuseColor = Color(0, 0, 0, 1);

		CHEngine::RenderSceneEntity(&scene, &cubeEntity2);

		cubeEntity2.position = pos;
		cubeEntity2.scale = 2.0f;
		cubeEntity2.material.diffuseColor = Color(0, 0, 0, 1);

		CHEngine::RenderSceneEntity(&scene, &cubeEntity2);

		CHEngine::PresentScene(&engine, &scene);
		CHGraphics::SwapBuffers(&window);
		CHEngine::ResetInput(&input);
	}
	CHEngine::Release(&scene);
	CHEngine::Release(&engine);
	CHGraphics::Release(&model);
	CHGraphics::Release(&floor);
	CHGraphics::Release(&cube);

	//DebugLiveObjects(window.device);
	CHGraphics::Release(&window);
}

void DebugLiveObjects(ID3D11Device *device)
{
	ID3D11Debug *pDebug;
	HRESULT hr = device->QueryInterface(IID_PPV_ARGS(&pDebug));
	WIN_CHECK_ERROR_DEBUG_OUTPUT_CALL(hr, "ID3D11Device::QueryInterface", "DebugLiveObjects");

	pDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);

	RELEASE_DX_RESOURCE(pDebug);
}

