#pragma once
#include "Core\Math.h"
#include "CHDef.h"
#include "CHGraphics.h"

struct CameraPolar
{
	float azimuth;
	float polar;
	float distance;
	Matrix4x4 projection;
};

struct Camera
{
	Vector3 position;
	Vector3 direction;
	Matrix4x4 projection;
};

struct Material
{
	Color diffuseColor;
	float metalness;
	float roughness;

	Vector2 padding;
};

struct ModelData
{
	Material material;
	Matrix4x4 modelMatrix;
};

struct Entity
{
	Mesh mesh;
	Material material;
	Vector3 position;
	float scale;

	bool transparent;
	bool castsShadow;
};

template <typename T, int C>
struct List
{
	T *items;
	uint32 length;
	List():
	length(0)
	{
		items = (T *)malloc(sizeof(T) * C);
	}
};

namespace CHList
{
	template <typename T, int C>
	void Reset(List<T, C> *list)
	{
		list->length = 0;
	}
	template <typename T, int C>
	void Add(List<T, C> *list, T *item)
	{
		if (list->length < C)
		{
			list->items[list->length++] = *item;
		}
	}
}

#define SCENE_SLOT 0
#define LIGHT_SLOT 1
#define SHADOW_VP_SLOT 3

#define MAIN_TEX_SLOT 0
#define SHADOWMAP_DIR_SLOT 1
#define SHADOWMAP_POINTCUBE_SLOT 2

struct Light
{
	Vector3 position;
	float intensity;
};

struct Engine
{
	Window *window;

	Program renderPass;
	Program shadowMapPassDirection;
	Program shadowMapPassPoint;

	Program fxaaPass;
	Program passthroughPass;

	ConstantBuffer modelBuffer;
	ConstantBuffer sceneBuffer;
	ConstantBuffer materialBuffer;
	ConstantBuffer lightBuffer;
	ConstantBuffer fxaaBuffer;
	ConstantBuffer shadowVPBuffer;

	List<ModelData, 1000> modelData[3];
	List<ModelData, 1000> modelDataTransparent[3];
	List<ModelData, 1000> modelDataShadows[3];

	Mesh meshes[3];
	Mesh quad;

	BlendState solidState;
	BlendState transparentState;

	Texture3D ambientTexDiff;
	Texture3D ambientTexSpec;
};

struct Scene3D
{
	List<Entity, 1000> entityList;
	Light light;
	Light pointLight;

	DepthBuffer depthBuffer;
	ColorBuffer colorBuffer;
	DepthBuffer shadowMapBufferDirection;
	DepthBuffer shadowMapBufferPoint;
	ColorBufferCubeMap colorCube;

	CameraPolar *camera;
};

namespace CHEngine
{
	Engine GetEngine(Window *window);
	Scene3D GetScene(Engine *engine);
	void PresentScene(Engine *engine, Scene3D *scene);
	void RenderSceneEntity(Scene3D *scene, Entity *entity);

	Light GetLight(Vector3 position, float intensity);
	void UseDirectionalLight(Scene3D *scene, Light *light);
	void UsePointLight(Scene3D *scene, Light *light);

	CameraPolar GetCameraPolar(Engine *engine);
	Vector3 GetPositionFromCamera(CameraPolar *camera);
	Matrix4x4 GetViewFromCamera(CameraPolar *camera);
	Matrix4x4 GetProjectionFromCamera(CameraPolar *camera);
	void UpdateCamera(CameraPolar *camera, Input *input);
	void SetSceneCamera(Scene3D *scene, CameraPolar *camera);

	Camera GetCamera(Engine *engine);
	Vector3 GetPositionFromCamera(Camera *camera);
	Matrix4x4 GetViewFromCamera(Camera *camera);
	Matrix4x4 GetProjectionFromCamera(Camera *camera);

	void BlitToTexture(Window *target, Engine *engine, Program *program);
	void BlitToTexture(Window *target, Engine *engine, ColorBuffer *buffer, Program *program);

	Material GetMaterial(Color color, float metalness, float roughness);

	Mesh GetMeshFromFile(Window *window, wchar_t *fileName);

	void Release(Scene3D *scene);
	void Release(Engine *engine);

	void ResetInput(Input *input);
}