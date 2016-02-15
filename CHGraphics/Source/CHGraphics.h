#pragma once
#include <windows.h>
#include "Core\Math.h"

struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain;
struct ID3D11RenderTargetView;
struct ID3D11Texture2D;
struct ID3D11DepthStencilView;
struct ID3D11ComputeShader;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11InputLayout;
struct ID3D11Buffer;
struct ID3D11Texture3D;
struct ID3D11ShaderResourceView;
struct ID3D11UnorderedAccessView;
struct ID3D11BlendState;
struct ID3D11DepthStencilState;

struct Window
{
	ID3D11Device *device;
	ID3D11DeviceContext *context;
	IDXGISwapChain *swapChain;
	ID3D11RenderTargetView *renderTarget;

	UINT width;
	UINT height;

	ID3D11DepthStencilState *depthOn;
	ID3D11DepthStencilState *depthOff;

	HWND windowHandle;
	bool running;
};

struct Input
{
	float wheelDelta;
	bool leftButtonDown;
	Vector2 touchPosition;
	Vector2 touchPositionPrev;

	bool pressedS;
};

enum BlendType
{
	BLEND_SOLID,
	BLEND_TRANSPARENT
};

struct BlendState
{
	ID3D11BlendState *blendState;
};

struct ColorBuffer
{
	ID3D11Texture2D *resource;
	ID3D11RenderTargetView *view;
	ID3D11ShaderResourceView *shaderResource;
	UINT width;
	UINT height;
};

struct ColorBufferCubeMap
{
	ID3D11Texture2D *resource;
	ID3D11RenderTargetView *view[6];
	ID3D11ShaderResourceView *shaderResource;
	UINT width;
	UINT height;
};

struct DepthBuffer
{
	ID3D11Texture2D *resource;
	ID3D11DepthStencilView *view;
	ID3D11ShaderResourceView *shaderResource;
	UINT width;
	UINT height;
};

struct DepthBufferCubeMap
{
	ID3D11Texture2D *resource;
	ID3D11DepthStencilView *view;
	ID3D11ShaderResourceView *texture;
	UINT width;
	UINT height;
};

enum VertexInputLayout
{
	VERTEX_INPUT_POSITION_NORMAL,
	VERTEX_INPUT_POSITION_TEXCOORD
};

struct Program
{
	ID3D11VertexShader *vertexShader;
	ID3D11PixelShader *pixelShader;
	ID3D11InputLayout *inputLayout;
};

struct ComputeProgram
{
	ID3D11ComputeShader *computeShader;
};

struct Mesh
{
	ID3D11Buffer *vertexBuffer;
	UINT vertexStride;
	ID3D11Buffer *indexBuffer;
	UINT indexCount;
};

struct ConstantBuffer
{
	ID3D11Buffer *buffer;
};

struct Texture2D
{
	ID3D11ShaderResourceView *texture;
	UINT width;
	UINT height;
};

struct Texture3D
{
	ID3D11ShaderResourceView *texture;
	UINT width;
	UINT height;
	UINT depth;
};

namespace CHGraphics
{
	Window GetWindow(HINSTANCE instance, UINT width, UINT height);
	void ProcessEvents(Window *window, Input *input);

	BlendState GetBlendState(Window *window, BlendType type);
	void SetBlendState(Window *window, BlendState *state);

	Texture2D GetTexture2Dint(Window *window, UINT width, UINT height, void *data);
	Texture3D GetTexture3D(Window *window, UINT width, UINT height, UINT depth,
					   void *data);
	Program GetProgram(Window *window, void *vertexShaderFile, SIZE_T vertexShaderSize,
					 void *pixelShaderFile, SIZE_T pixelShaderSize, VertexInputLayout layout);
	Program GetProgram(Window *window, wchar_t *vertexShaderFileName, wchar_t *pixelShaderFileName, 
					   VertexInputLayout layout);

	void TurnOnDepthWrite(Window *window);
	void TurnOffDepthWrite(Window *window);

	Mesh GetMesh(Window *window, void *vertexData, UINT vertexStride, UINT vertexCount,
				   void *indexData, UINT indexCount);
	DepthBuffer GetDepthBuffer(Window *window, UINT width, UINT height);
	DepthBufferCubeMap GetDepthBufferCubeMap(Window *window, UINT width, UINT height);
	ColorBuffer GetColorBuffer(Window *window, UINT width, UINT height);
	ColorBufferCubeMap GetColorBufferCubeMap(Window *window, UINT width, UINT height);
	ConstantBuffer GetConstantBuffer(Window *window, UINT size);

	void Downsample(Window *window, ColorBuffer *bufferMS, ColorBuffer *buffer);

	void BindProgram(Window *window, Program *program);
	void BindConstantBuffer(Window *window, ConstantBuffer *buffer, void *data, UINT slot);
	void RenderMesh(Window *window, Mesh *model);
	void RenderMeshInstanced(Window *window, Mesh *model, UINT instanceCount);

	void ClearWindow(Window *window, float r, float g, float b, float a);
	void ClearColorBuffer(Window *window, ColorBuffer *buffer, float r, float g, float b, float a);
	void ClearColorBufferCube(Window *window, ColorBufferCubeMap *buffer, float r, float g, float b, float a);
	void ClearDepthBuffer(Window *window, DepthBuffer *buffer);
	void BindTargetWindow(Window *window, DepthBuffer *depthBuffer = NULL);
	void BindTarget(Window *window, ColorBuffer *buffer, DepthBuffer *depthBuffer = NULL);
	void BindTarget(Window *window, ColorBufferCubeMap *bufferCube, UINT index, DepthBuffer *depthBuffer = NULL);
	void BindTarget(Window *window, DepthBuffer *depthBuffer);
	void BindTargets(Window *window, ColorBuffer *buffers, UINT bufferCount, DepthBuffer *depthBuffer = NULL);
	void UnbindTarget(Window *window);
	void UnbindTexture2D(Window *window, UINT slot);
	void BindTexture2D(Window *window, ColorBuffer *buffer, UINT slot);
	void BindTexture2D(Window *window, Texture2D *texture, UINT slot);
	void BindTexture2D(Window *window, DepthBuffer *buffer, UINT slot);
	void BindTexture3D(Window *window, Texture3D *texture, UINT slot);
	void BindTextureCubeMap(Window *window, ColorBufferCubeMap *bufferCube, UINT slot);
	void SwapBuffers(Window *window);

	ComputeProgram GetComputeProgram(Window *window, void *computeShaderFile, SIZE_T computeShaderSize);
	ComputeProgram GetComputeProgram(Window *window, wchar_t *computeShaderFileName);
	void DispatchComputeProgram(Window *window, ComputeProgram *computeProgram, 
								UINT groupCountX, UINT groupCountY, UINT groupCountZ);
	void Release(Texture2D *texture2D);
	void Release(Texture3D *texture3D);
	void Release(Program *program);
	void Release(ComputeProgram *program);
	void Release(Mesh *mesh);
	void Release(DepthBuffer *buffer);
	void Release(ColorBuffer *buffer);
	void Release(ColorBufferCubeMap *buffer);
	void Release(ConstantBuffer *buffer);
	void Release(BlendState *state);
	void Release(Window *window);
}