#include "CHGraphics.h"
#include "CHDef.h"
#include "CHWin32.h"
#include <D3D11.h>
#include <windowsx.h>

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void CHGraphics::ProcessEvents(Window *window, Input *input)
{
	MSG message;
	while (PeekMessageA(&message, window->windowHandle, 0, 0, PM_REMOVE))
	{
		switch (message.message)
		{
			case WM_CLOSE:
				window->running = false;
				break;
			case WM_SYSKEYDOWN:
			case WM_KEYDOWN:
			{
				if (message.wParam == VK_ESCAPE)
				{
					window->running = false;
				}
				if (message.wParam == 'R')
				{
				}
				switch (message.wParam)
				{
					case 'W':
						break;
					case 'A':
						break;
					case 'S':
						input->pressedS = true;
						break;
					case 'D':
						break;
					case 'G':
						break;
					case 'Q':
						break;
				}
			} break;
			case WM_MOUSEMOVE:
			{
				input->touchPosition.x = (float)GET_X_LPARAM(message.lParam);
				input->touchPosition.y = (float)GET_Y_LPARAM(message.lParam);
			} break;
			case WM_LBUTTONDOWN:
			{
				input->leftButtonDown = true;
			} break;
			case WM_RBUTTONDOWN:
			{
			} break;
			case WM_LBUTTONUP:
			{
				input->leftButtonDown = false;
			} break;
			case WM_MOUSEWHEEL:
			{
				input->wheelDelta = GET_WHEEL_DELTA_WPARAM(message.wParam);

			} break;
			default:
			{
				TranslateMessage(&message);
				DispatchMessageA(&message);
			} break;
		}
	}
}

void CHGraphics::ClearWindow(Window *window, float r, float g, float b, float a)
{
	float color[4] = {r,g,b,a};
	window->context->ClearRenderTargetView(window->renderTarget, color);
}

void CHGraphics::ClearColorBuffer(Window *window, ColorBuffer *buffer, float r, float g, float b, float a)
{
	float color[4] = { r,g,b,a };
	window->context->ClearRenderTargetView(buffer->view, color);
}

void CHGraphics::ClearColorBufferCube(Window *window, ColorBufferCubeMap *buffer, float r, float g, float b, float a)
{
	float color[4] = { r,g,b,a };
	window->context->ClearRenderTargetView(buffer->view[0], color);
	window->context->ClearRenderTargetView(buffer->view[1], color);
	window->context->ClearRenderTargetView(buffer->view[2], color);
	window->context->ClearRenderTargetView(buffer->view[3], color);
	window->context->ClearRenderTargetView(buffer->view[4], color);
	window->context->ClearRenderTargetView(buffer->view[5], color);
}

BlendState CHGraphics::GetBlendState(Window *window, BlendType type)
{
	BlendState result;
	D3D11_BLEND_DESC blendDesc = { 0 };

	D3D11_RENDER_TARGET_BLEND_DESC rtbd = { 0 };

	if (type == BLEND_TRANSPARENT)
	{
		rtbd.BlendEnable = true;
		rtbd.SrcBlend = D3D11_BLEND_SRC_ALPHA;
		rtbd.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		rtbd.BlendOp = D3D11_BLEND_OP_ADD;
		rtbd.SrcBlendAlpha = D3D11_BLEND_INV_DEST_ALPHA;
		rtbd.DestBlendAlpha = D3D11_BLEND_ONE;
		rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
		rtbd.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	}
	else if (type == BLEND_SOLID)
	{
		rtbd.BlendEnable = true;
		rtbd.SrcBlend = D3D11_BLEND_ONE;
		rtbd.DestBlend = D3D11_BLEND_ZERO;
		rtbd.BlendOp = D3D11_BLEND_OP_ADD;
		rtbd.SrcBlendAlpha = D3D11_BLEND_ONE;
		rtbd.DestBlendAlpha = D3D11_BLEND_ZERO;
		rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
		rtbd.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	}

	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.RenderTarget[0] = rtbd;

	HRESULT hr = window->device->CreateBlendState(&blendDesc, &result.blendState);
	WIN_CHECK_ERROR_DEBUG_OUTPUT_CALL(hr, "ID3D11Device::CreateBlendState", "GetBlendState");
	return result;
}

void CHGraphics::SetBlendState(Window *window, BlendState *state)
{
	float factor[4] = {1,1,1,1};
	window->context->OMSetBlendState(state->blendState, factor, 0xffffffff);
}



void CHGraphics::ClearDepthBuffer(Window *window, DepthBuffer *buffer)
{
	window->context->ClearDepthStencilView(buffer->view, D3D11_CLEAR_DEPTH, 1.0f, 0);
}


void CHGraphics::SwapBuffers(Window *window)
{
	window->swapChain->Present(0, 0);
}

DepthBufferCubeMap CHGraphics::GetDepthBufferCubeMap(Window *window, UINT width, UINT height)
{
	DepthBufferCubeMap result;
	result.width = width;
	result.height = height;

	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 6;
	texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	texDesc.CPUAccessFlags = 0;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	HRESULT hr = window->device->CreateTexture2D(&texDesc, NULL, &result.resource);
	WIN_CHECK_ERROR_DEBUG_OUTPUT_CALL(hr, "ID3D11Device::CreateTexture2D", "GetTextureCubeMap");

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderViewDesc;
	shaderViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	shaderViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	shaderViewDesc.TextureCube.MipLevels = texDesc.MipLevels;
	shaderViewDesc.TextureCube.MostDetailedMip = 0;

	hr = window->device->CreateShaderResourceView(result.resource, &shaderViewDesc, &result.texture);
	WIN_CHECK_ERROR_DEBUG_OUTPUT_CALL(hr, "ID3D11Device::CreateShaderResourceView", "GetTextureCubeMap");

	CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2DARRAY, DXGI_FORMAT_D24_UNORM_S8_UINT);
	hr = window->device->CreateDepthStencilView(result.resource, &depthStencilViewDesc, &result.view);
	WIN_CHECK_ERROR_DEBUG_OUTPUT_CALL(hr, "ID3D11Device::CreateDepthStencilView", "GetDepthBuffer");
	return result;
}

Texture3D CHGraphics::GetTexture3D(Window *window, UINT width, UINT height, UINT depth, void *data)
{
	Texture3D result;
	result.width = width;
	result.height = height;
	result.depth = depth;

	D3D11_TEXTURE3D_DESC texDesc = {};
	texDesc.Depth = depth;
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.Format = DXGI_FORMAT_R32_FLOAT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.MipLevels = 1;

	D3D11_SUBRESOURCE_DATA initialData = {};
	initialData.pSysMem = data;
	initialData.SysMemPitch = width * sizeof(float);
	initialData.SysMemSlicePitch = width * height * sizeof(float);

	ID3D11Texture3D *tex3D;
	HRESULT hr = window->device->CreateTexture3D(&texDesc, &initialData, &tex3D);
	WIN_CHECK_ERROR_DEBUG_OUTPUT_CALL(hr, "ID3D11Device::CreateTexture3D", "GetTexture3D");

	D3D11_SHADER_RESOURCE_VIEW_DESC resourceDesc = {};
	resourceDesc.Format = DXGI_FORMAT_R32_FLOAT;
	resourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
	resourceDesc.Texture3D.MostDetailedMip = 0;
	resourceDesc.Texture3D.MipLevels = -1;

	hr = window->device->CreateShaderResourceView(tex3D, &resourceDesc, &result.texture);
	WIN_CHECK_ERROR_DEBUG_OUTPUT_CALL(hr, "ID3D11Device::CreateShaderResourceView", "GetTexture3D");
	RELEASE_DX_RESOURCE(tex3D);
	return result;
}

Texture2D CHGraphics::GetTexture2Dint(Window *window, UINT width, UINT height, void *data)
{
	Texture2D result;
	result.width = width;
	result.height = height;

	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.Format = DXGI_FORMAT_R32_SINT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.SampleDesc.Count = 1;

	D3D11_SUBRESOURCE_DATA initialData = {};
	initialData.pSysMem = data;
	initialData.SysMemPitch = width * sizeof(float);
	initialData.SysMemSlicePitch = width * height * sizeof(float);

	ID3D11Texture2D *tex2D;
	HRESULT hr = window->device->CreateTexture2D(&texDesc, data ? &initialData : NULL, &tex2D);
	WIN_CHECK_ERROR_DEBUG_OUTPUT_CALL(hr, "ID3D11Device::CreateTexture2D", "GetTexture2D");

	D3D11_SHADER_RESOURCE_VIEW_DESC resourceDesc = {};
	resourceDesc.Format = DXGI_FORMAT_R32_SINT;
	resourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	resourceDesc.Texture2D.MostDetailedMip = 0;
	resourceDesc.Texture2D.MipLevels = -1;

	hr = window->device->CreateShaderResourceView(tex2D, &resourceDesc, &result.texture);
	WIN_CHECK_ERROR_DEBUG_OUTPUT_CALL(hr, "ID3D11Device::CreateShaderResourceView", "GetTexture2D");
	RELEASE_DX_RESOURCE(tex2D);

	return result;
}


void CHGraphics::UnbindTexture2D(Window *window, UINT slot)
{
	ID3D11ShaderResourceView *shaderResources[] = { NULL };
	window->context->PSSetShaderResources(slot, 1, shaderResources);
	window->context->CSSetShaderResources(slot, 1, shaderResources);

}

void CHGraphics::BindTexture2D(Window *window, ColorBuffer *buffer, UINT slot)
{
	window->context->PSSetShaderResources(slot, 1, &buffer->shaderResource);
	window->context->CSSetShaderResources(slot, 1, &buffer->shaderResource);
}

void CHGraphics::BindTexture2D(Window *window, Texture2D *texture, UINT slot)
{
	window->context->PSSetShaderResources(slot, 1, &texture->texture);
	window->context->CSSetShaderResources(slot, 1, &texture->texture);
}


void CHGraphics::BindTexture2D(Window *window, DepthBuffer *buffer, UINT slot)
{
	window->context->PSSetShaderResources(slot, 1, &buffer->shaderResource);
	window->context->CSSetShaderResources(slot, 1, &buffer->shaderResource);
}

void CHGraphics::BindTextureCubeMap(Window *window, ColorBufferCubeMap *bufferCube, UINT slot)
{
	window->context->PSSetShaderResources(slot, 1, &bufferCube->shaderResource);
}

void CHGraphics::BindTexture3D(Window *window, Texture3D *texture, UINT slot)
{
	window->context->PSSetShaderResources(slot, 1, &texture->texture);
	window->context->CSSetShaderResources(slot, 1, &texture->texture);
}


DepthBuffer CHGraphics::GetDepthBuffer(Window *window, UINT width, UINT height)
{
	DepthBuffer result;
	result.width = width;
	result.height = height;
	CD3D11_TEXTURE2D_DESC depthStencilDesc(DXGI_FORMAT_R24G8_TYPELESS, width, height,
										   1, 1, D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE,
										   D3D11_USAGE_DEFAULT, 0, 1);
	HRESULT hr = window->device->CreateTexture2D(&depthStencilDesc, NULL, &result.resource);
	WIN_CHECK_ERROR_DEBUG_OUTPUT_CALL(hr, "ID3D11Device::CreateTexture2D", "GetDepthBuffer");

	CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D, DXGI_FORMAT_D24_UNORM_S8_UINT);
	hr = window->device->CreateDepthStencilView(result.resource, &depthStencilViewDesc, &result.view);
	WIN_CHECK_ERROR_DEBUG_OUTPUT_CALL(hr, "ID3D11Device::CreateDepthStencilView", "GetDepthBuffer");

	CD3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc(D3D11_SRV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R24_UNORM_X8_TYPELESS);
	hr = window->device->CreateShaderResourceView(result.resource, &shaderResourceViewDesc, &result.shaderResource);
	WIN_CHECK_ERROR_DEBUG_OUTPUT_CALL(hr, "ID3D11Device::CreateShaderResourceView", "GetDepthBuffer");
	return result;
}

ColorBufferCubeMap CHGraphics::GetColorBufferCubeMap(Window *window, UINT width, UINT height)
{
	ColorBufferCubeMap result;
	result.width = width;
	result.height = height;

	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 6;
	texDesc.Format = DXGI_FORMAT_R32_FLOAT;
	texDesc.CPUAccessFlags = 0;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	HRESULT hr = window->device->CreateTexture2D(&texDesc, NULL, &result.resource);
	WIN_CHECK_ERROR_DEBUG_OUTPUT_CALL(hr, "ID3D11Device::CreateTexture2D", "GetColorBufferCubeMap");

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderViewDesc;
	shaderViewDesc.Format = DXGI_FORMAT_R32_FLOAT;
	shaderViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	shaderViewDesc.TextureCube.MipLevels = texDesc.MipLevels;
	shaderViewDesc.TextureCube.MostDetailedMip = 0;

	hr = window->device->CreateShaderResourceView(result.resource, &shaderViewDesc, &result.shaderResource);
	WIN_CHECK_ERROR_DEBUG_OUTPUT_CALL(hr, "ID3D11Device::CreateShaderResourceView", "GetColorBufferCubeMap");

	for (int i = 0; i < 6; ++i)
	{
		CD3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc(D3D11_RTV_DIMENSION_TEXTURE2DARRAY, DXGI_FORMAT_R32_FLOAT, 0, i, 1);
		hr = window->device->CreateRenderTargetView(result.resource, &renderTargetViewDesc, &result.view[i]);
		WIN_CHECK_ERROR_DEBUG_OUTPUT_CALL(hr, "ID3D11Device::CreateDepthStencilView", "GetColorBufferCubeMap");
	}

	return result;
}

ColorBuffer CHGraphics::GetColorBuffer(Window *window, UINT width, UINT height)
{
	ColorBuffer result;
	result.width = width;
	result.height = height;
	CD3D11_TEXTURE2D_DESC colorBufferDesc(DXGI_FORMAT_R8G8B8A8_UNORM, width, height,
										   1, 1, 
										  D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS,
										  D3D11_USAGE_DEFAULT, 0, 1);
	HRESULT hr = window->device->CreateTexture2D(&colorBufferDesc, NULL, &result.resource);
	WIN_CHECK_ERROR_DEBUG_OUTPUT_CALL(hr, "ID3D11Device::CreateTexture2D", "GetColorBuffer");

	CD3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc(D3D11_RTV_DIMENSION_TEXTURE2D);
	hr = window->device->CreateRenderTargetView(result.resource, &renderTargetViewDesc, &result.view);
	WIN_CHECK_ERROR_DEBUG_OUTPUT_CALL(hr, "ID3D11Device::CreateRenderTargetView", "GetColorBuffer");

	CD3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc(D3D11_SRV_DIMENSION_TEXTURE2D);
	hr = window->device->CreateShaderResourceView(result.resource, &shaderResourceViewDesc, &result.shaderResource);
	WIN_CHECK_ERROR_DEBUG_OUTPUT_CALL(hr, "ID3D11Device::CreateShaderResourceView", "GetColorBuffer");

	return result;
}

Window CHGraphics::GetWindow(HINSTANCE hInstance, UINT width, UINT height)
{
	Window result;
	result.width = width;
	result.height = height;

	WNDCLASSEXA windowClass = {};
	windowClass.cbSize = sizeof(WNDCLASSEXA);
	windowClass.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
	windowClass.lpfnWndProc = WndProc;
	windowClass.hInstance = hInstance;
	windowClass.hCursor = LoadCursor(0, IDC_ARROW);
	windowClass.lpszClassName = "CHGraphics";

	DWORD winFlags = WS_VISIBLE;
#ifdef FULLSCREEN
	winFlags |= WS_POPUP;
#else
	winFlags |= WS_OVERLAPPED;
#endif

	if (RegisterClassExA(&windowClass))
	{
		result.windowHandle = CreateWindowExA(0, windowClass.lpszClassName, "CHGraphics", winFlags,
											  CW_USEDEFAULT, CW_USEDEFAULT, width, height,
											  NULL, NULL, hInstance, NULL);
		if (result.windowHandle == INVALID_HANDLE_VALUE)
		{
			WIN_ERROR_DEBUG_OUTPUT_CALL("CreateWindowExA", "GetWindow");
		}

	}
	else
	{
		WIN_ERROR_DEBUG_OUTPUT_CALL("RegisterClassExA", "GetWindow");
	}

	UINT flags = 0;
#ifdef _DEBUG
	flags = D3D11_CREATE_DEVICE_DEBUG;
#endif
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.OutputWindow = result.windowHandle;
	swapChainDesc.Windowed = true;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	D3D_FEATURE_LEVEL supportedFeatureLevel;
	HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, flags, 
											   &featureLevel, 1, D3D11_SDK_VERSION,
											   &swapChainDesc, &result.swapChain, &result.device, 
											   &supportedFeatureLevel, &result.context);
	WIN_CHECK_ERROR_DEBUG_OUTPUT_CALL(hr, "D3D11CreateDeviceAndSwapChain", "GetWindow");

	ID3D11Texture2D *backBuffer;
	hr = result.swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&backBuffer);
	WIN_CHECK_ERROR_DEBUG_OUTPUT_CALL(hr, "DXGISwapChain::GetBuffer", "GetWindow");

	hr = result.device->CreateRenderTargetView(backBuffer, NULL, &result.renderTarget);
	WIN_CHECK_ERROR_DEBUG_OUTPUT_CALL(hr, "ID3D11Device::CreateRenderTargetView", "GetWindow");
	RELEASE_DX_RESOURCE(backBuffer);

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.BorderColor[0] = .999f;
	samplerDesc.BorderColor[1] = .999f;
	samplerDesc.BorderColor[2] = .999f;
	samplerDesc.BorderColor[3] = .999f;
	samplerDesc.MinLOD = -D3D11_FLOAT32_MAX;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	ID3D11SamplerState *texSampler;
	hr = result.device->CreateSamplerState(&samplerDesc, &texSampler);
	WIN_CHECK_ERROR_DEBUG_OUTPUT_CALL(hr, "ID3D11Device::CreateSamplerState", "GetWindow");
	result.context->PSSetSamplers(0, 1, &texSampler);
	result.context->CSSetSamplers(0, 1, &texSampler);
	RELEASE_DX_RESOURCE(texSampler);

	D3D11_DEPTH_STENCIL_DESC depthOffDesc = { 0 };
	depthOffDesc.DepthEnable = true;
	depthOffDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depthOffDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	depthOffDesc.StencilEnable = false;

	hr = result.device->CreateDepthStencilState(&depthOffDesc, &result.depthOff);
	WIN_CHECK_ERROR_DEBUG_OUTPUT_CALL(hr, "ID3D11Device::CreateDepthStencilState", "GetWindow");

	D3D11_DEPTH_STENCIL_DESC depthOnDesc = { 0 };
	depthOnDesc.DepthEnable = true;
	depthOnDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthOnDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	depthOnDesc.StencilEnable = false;

	hr = result.device->CreateDepthStencilState(&depthOnDesc, &result.depthOn);
	WIN_CHECK_ERROR_DEBUG_OUTPUT_CALL(hr, "ID3D11Device::CreateDepthStencilState", "GetWindow");

	result.running = true;
	return result;
}

void CHGraphics::TurnOnDepthWrite(Window *window)
{
	window->context->OMSetDepthStencilState(window->depthOn, 1);
}

void CHGraphics::TurnOffDepthWrite(Window *window)
{
	window->context->OMSetDepthStencilState(window->depthOff, 1);
}

Program CHGraphics::GetProgram(Window *window, void *vertexShaderFile, SIZE_T vertexShaderSize,
				 void *pixelShaderFile, SIZE_T pixelShaderSize, VertexInputLayout layout)
{
	Program result;
	ID3D11Device *device = window->device;
	HRESULT hr = device->CreateVertexShader(vertexShaderFile, vertexShaderSize,
											NULL, &result.vertexShader);
	WIN_CHECK_ERROR_DEBUG_OUTPUT_CALL(hr, "ID3D11Device::CreateVertexShader", "GetProgram");

	switch (layout)
	{
		case VERTEX_INPUT_POSITION_NORMAL:
		{
			D3D11_INPUT_ELEMENT_DESC positionDesc = {};
			positionDesc.SemanticName = "POSITION";
			positionDesc.SemanticIndex = 0;
			positionDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
			positionDesc.InputSlot = 0;
			positionDesc.AlignedByteOffset = 0;
			positionDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			positionDesc.InstanceDataStepRate = 0;

			D3D11_INPUT_ELEMENT_DESC normalDesc = {};
			normalDesc.SemanticName = "NORMAL";
			normalDesc.SemanticIndex = 0;
			normalDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
			normalDesc.InputSlot = 0;
			normalDesc.AlignedByteOffset = 12;
			normalDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			normalDesc.InstanceDataStepRate = 0;

			D3D11_INPUT_ELEMENT_DESC vertexDesc[] = { positionDesc, normalDesc };
			hr = device->CreateInputLayout(vertexDesc, ARRAYSIZE(vertexDesc),
										   vertexShaderFile, vertexShaderSize,
										   &result.inputLayout);
			WIN_CHECK_ERROR_DEBUG_OUTPUT_CALL(hr, "ID3D11Device::CreateInputLayout", "GetProgram");
		} break;
		case VERTEX_INPUT_POSITION_TEXCOORD:
		{
			D3D11_INPUT_ELEMENT_DESC positionDesc = {};
			positionDesc.SemanticName = "POSITION";
			positionDesc.SemanticIndex = 0;
			positionDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
			positionDesc.InputSlot = 0;
			positionDesc.AlignedByteOffset = 0;
			positionDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			positionDesc.InstanceDataStepRate = 0;

			D3D11_INPUT_ELEMENT_DESC normalDesc = {};
			normalDesc.SemanticName = "TEXCOORD";
			normalDesc.SemanticIndex = 0;
			normalDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
			normalDesc.InputSlot = 0;
			normalDesc.AlignedByteOffset = 12;
			normalDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			normalDesc.InstanceDataStepRate = 0;

			D3D11_INPUT_ELEMENT_DESC vertexDesc[] = { positionDesc, normalDesc };
			hr = device->CreateInputLayout(vertexDesc, ARRAYSIZE(vertexDesc),
										   vertexShaderFile, vertexShaderSize,
										   &result.inputLayout);
			WIN_CHECK_ERROR_DEBUG_OUTPUT_CALL(hr, "ID3D11Device::CreateInputLayout", "GetProgram");
		} break;
		default:
			WIN_ERROR_DEBUG_OUTPUT("Invalid input layout type");
			break;
	}

	if (pixelShaderFile)
	{
		hr = device->CreatePixelShader(pixelShaderFile, pixelShaderSize,
										NULL, &result.pixelShader);
		WIN_CHECK_ERROR_DEBUG_OUTPUT_CALL(hr, "ID3D11Device::CreatePixelShader", "GetProgram");
	}
	else
	{
		result.pixelShader = NULL;
	}
	return result;
}

Program CHGraphics::GetProgram(Window *window, wchar_t *vertexShaderFileName, wchar_t *pixelShaderFileName,
							   VertexInputLayout layout)
{
	Program result;
	FileData vertexShader = Win32ReadFile(vertexShaderFileName);
	if (pixelShaderFileName)
	{
		FileData pixelShader = Win32ReadFile(pixelShaderFileName);
		result = CHGraphics::GetProgram(window, vertexShader.data, vertexShader.size,
												   pixelShader.data, pixelShader.size, layout);
		Win32ReleaseFile(pixelShader);
	}
	else
	{
		result = CHGraphics::GetProgram(window, vertexShader.data, vertexShader.size,
												NULL, 0, layout);
	}
	Win32ReleaseFile(vertexShader);
	return result;
}

ComputeProgram CHGraphics::GetComputeProgram(Window *window, void *computeShaderFile, SIZE_T computeShaderSize)
{
	ComputeProgram result;
	HRESULT hr = window->device->CreateComputeShader(computeShaderFile, computeShaderSize, 
													 NULL, &result.computeShader);
	WIN_CHECK_ERROR_DEBUG_OUTPUT_CALL(hr, "ID3D11Device::CreateComputeShader", "GetComputeProgram");
	return result;
}

ComputeProgram CHGraphics::GetComputeProgram(Window *window, wchar_t *computeShaderFileName)
{
	FileData computeShader = Win32ReadFile(computeShaderFileName);
	ComputeProgram result = CHGraphics::GetComputeProgram(window, computeShader.data, computeShader.size);
	Win32ReleaseFile(computeShader);
	return result;
}

Mesh CHGraphics::GetMesh(Window *window, void *vertexData, UINT vertexStride, 
						   UINT vertexCount, void *indexData, UINT indexCount)
{
	Mesh result;
	ID3D11Device *device = window->device;

	D3D11_SUBRESOURCE_DATA vertexBufferData = {};
	vertexBufferData.pSysMem = vertexData;

	CD3D11_BUFFER_DESC vertexBufferDesc(vertexCount * vertexStride, D3D11_BIND_VERTEX_BUFFER);
	HRESULT hr = device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &result.vertexBuffer);
	WIN_CHECK_ERROR_DEBUG_OUTPUT_CALL(hr, "ID3D11Device::CreateBuffer", "GetMesh");

	D3D11_SUBRESOURCE_DATA indexBufferData = {};
	indexBufferData.pSysMem = indexData;

	CD3D11_BUFFER_DESC indexBufferDesc(indexCount * sizeof(UINT16), D3D11_BIND_INDEX_BUFFER);
	hr = device->CreateBuffer(&indexBufferDesc, &indexBufferData, &result.indexBuffer);
	WIN_CHECK_ERROR_DEBUG_OUTPUT_CALL(hr, "ID3D11Device::CreateBuffer", "GetMesh");

	result.indexCount = indexCount;
	result.vertexStride = vertexStride;
	return result;
}

ConstantBuffer CHGraphics::GetConstantBuffer(Window *window, UINT size)
{
	ConstantBuffer result;
	UINT bufferSize = (((size - 1) / 16) + 1) * 16;

	CD3D11_BUFFER_DESC vertexBufferDesc(bufferSize, D3D11_BIND_CONSTANT_BUFFER);
	HRESULT hr = window->device->CreateBuffer(&vertexBufferDesc, NULL, &result.buffer);
	WIN_CHECK_ERROR_DEBUG_OUTPUT_CALL(hr, "ID3D11Device::CreateBuffer", "GetConstantBuffer");
	return result;
}

void CHGraphics::BindTargets(Window *window, ColorBuffer *buffers, UINT bufferCount, 
							 DepthBuffer *depthBuffer)
{
	ID3D11DepthStencilView *depthStencil = NULL;
	if (depthBuffer)
	{
		depthStencil = depthBuffer->view;
	}
	ID3D11RenderTargetView *renderTargets[] = { NULL, NULL, NULL };
	if (buffers)
	{
		for (UINT i = 0; i < bufferCount; ++i)
		{
			renderTargets[i] = buffers[i].view;
		}
		CD3D11_VIEWPORT viewport(0.0f, 0.0f, (float)buffers[0].width, (float)buffers[0].height);
		window->context->RSSetViewports(1, &viewport);
	}
	window->context->OMSetRenderTargets(bufferCount, renderTargets, depthStencil);
}

void CHGraphics::BindTarget(Window *window, DepthBuffer *depthBuffer)
{
	ID3D11RenderTargetView *targets[] = { NULL };

	window->context->OMSetRenderTargets(1, targets, depthBuffer->view);
	CD3D11_VIEWPORT viewport(0.0f, 0.0f, (float)depthBuffer->width, (float)depthBuffer->height);
	window->context->RSSetViewports(1, &viewport);
}

void CHGraphics::BindTarget(Window *window, ColorBufferCubeMap *bufferCube, UINT index, DepthBuffer *depthBuffer)
{
	ID3D11RenderTargetView *targets[] = { bufferCube->view[index] };

	window->context->OMSetRenderTargets(1, targets, depthBuffer->view);
	CD3D11_VIEWPORT viewport(0.0f, 0.0f, (float)depthBuffer->width, (float)depthBuffer->height);
	window->context->RSSetViewports(1, &viewport);
}


void CHGraphics::BindTargetWindow(Window *window, DepthBuffer *depthBuffer)
{
	ID3D11DepthStencilView *depthStencil = NULL;
	if (depthBuffer)
	{
		depthStencil = depthBuffer->view;
	}
	window->context->OMSetRenderTargets(1, &window->renderTarget, depthStencil);
	CD3D11_VIEWPORT viewport(0.0f,0.0f,(float)window->width, (float)window->height);
	window->context->RSSetViewports(1, &viewport);
}

void CHGraphics::BindTarget(Window *window, ColorBuffer *buffer, DepthBuffer *depthBuffer)
{
	ID3D11DepthStencilView *depthStencil = NULL;
	if (depthBuffer)
	{
		depthStencil = depthBuffer->view;
	}
	ID3D11RenderTargetView *renderTargets[] = {NULL};
	if (buffer)
	{
		renderTargets[0] = buffer->view;
		CD3D11_VIEWPORT viewport(0.0f, 0.0f, (float)buffer->width, (float)buffer->height);
		window->context->RSSetViewports(1, &viewport);
	}
	window->context->OMSetRenderTargets(1, renderTargets, depthStencil);
}

void CHGraphics::UnbindTarget(Window *window)
{
	ID3D11RenderTargetView *renderTargets[] = { NULL };
	window->context->OMSetRenderTargets(1, renderTargets, NULL);
}

void CHGraphics::BindProgram(Window *window, Program *program)
{
	ID3D11DeviceContext *context = window->context;
	context->IASetInputLayout(program->inputLayout);
	context->VSSetShader(program->vertexShader, 0, 0);
	context->PSSetShader(program->pixelShader, 0, 0);
}

void CHGraphics::BindConstantBuffer(Window *window, ConstantBuffer *buffer, void *data, UINT slot)
{
	window->context->UpdateSubresource(buffer->buffer, 0, 0, data, 0, 0);
	window->context->VSSetConstantBuffers(slot, 1, &buffer->buffer);
	window->context->PSSetConstantBuffers(slot, 1, &buffer->buffer);
	window->context->CSSetConstantBuffers(slot, 1, &buffer->buffer);
}

void CHGraphics::RenderMesh(Window *window, Mesh *model)
{
	ID3D11DeviceContext *context = window->context;
	UINT offset = 0;
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetVertexBuffers(0, 1, &model->vertexBuffer, &model->vertexStride, &offset);
	context->IASetIndexBuffer(model->indexBuffer, DXGI_FORMAT_R16_UINT, 0);
	context->DrawIndexed(model->indexCount, 0, 0);
}

void CHGraphics::RenderMeshInstanced(Window *window, Mesh *model, UINT instanceCount)
{
	ID3D11DeviceContext *context = window->context;
	UINT offset = 0;
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetVertexBuffers(0, 1, &model->vertexBuffer, &model->vertexStride, &offset);
	context->IASetIndexBuffer(model->indexBuffer, DXGI_FORMAT_R16_UINT, 0);
	context->DrawIndexedInstanced(model->indexCount, instanceCount, 0, 0, 0);
}

void CHGraphics::Downsample(Window *window, ColorBuffer *bufferMS, ColorBuffer *buffer)
{
	window->context->ResolveSubresource(buffer->resource, 0, bufferMS->resource, 0, DXGI_FORMAT_R8G8B8A8_UNORM);
}

void CHGraphics::DispatchComputeProgram(Window *window, ComputeProgram *computeProgram,
										UINT groupCountX, UINT groupCountY, UINT groupCountZ)
{
	window->context->CSSetShader(computeProgram->computeShader, 0, 0);
	window->context->Dispatch(groupCountX, groupCountY, groupCountZ);
}

void CHGraphics::Release(Texture2D *texture2D)
{
	RELEASE_DX_RESOURCE(texture2D->texture);
	texture2D->width = 0;
	texture2D->height = 0;
}

void CHGraphics::Release(Texture3D *texture3D)
{
	RELEASE_DX_RESOURCE(texture3D->texture);
	texture3D->width = 0;
	texture3D->height = 0;
	texture3D->depth = 0;
}

void CHGraphics::Release(Program *program)
{
	RELEASE_DX_RESOURCE(program->inputLayout);
	RELEASE_DX_RESOURCE(program->vertexShader);
	RELEASE_DX_RESOURCE(program->pixelShader);
}

void CHGraphics::Release(ComputeProgram *program)
{
	RELEASE_DX_RESOURCE(program->computeShader);
}

void CHGraphics::Release(Mesh *mesh)
{
	RELEASE_DX_RESOURCE(mesh->vertexBuffer);
	RELEASE_DX_RESOURCE(mesh->indexBuffer);
	mesh->vertexStride = 0;
	mesh->indexCount = 0;
}

void CHGraphics::Release(DepthBuffer *buffer)
{
	RELEASE_DX_RESOURCE(buffer->view);
	RELEASE_DX_RESOURCE(buffer->shaderResource);
	RELEASE_DX_RESOURCE(buffer->resource);
}

void CHGraphics::Release(ColorBuffer *buffer)
{
	RELEASE_DX_RESOURCE(buffer->view);
	RELEASE_DX_RESOURCE(buffer->shaderResource);
	RELEASE_DX_RESOURCE(buffer->resource);
}

void CHGraphics::Release(ColorBufferCubeMap *buffer)
{
	RELEASE_DX_RESOURCE(buffer->resource);
	RELEASE_DX_RESOURCE(buffer->shaderResource);
	for (uint32 i = 0; i < 6; ++i)
	{
		RELEASE_DX_RESOURCE(buffer->view[i]);
	}
}


void CHGraphics::Release(ConstantBuffer *buffer)
{
	RELEASE_DX_RESOURCE(buffer->buffer);
}

void CHGraphics::Release(Window *window)
{
	RELEASE_DX_RESOURCE(window->depthOff);
	RELEASE_DX_RESOURCE(window->depthOn);
	RELEASE_DX_RESOURCE(window->renderTarget);
	RELEASE_DX_RESOURCE(window->swapChain);
	RELEASE_DX_RESOURCE(window->context);
	RELEASE_DX_RESOURCE(window->device);
	window->width = 0;
	window->height = 0;
}

void CHGraphics::Release(BlendState *state)
{
	RELEASE_DX_RESOURCE(state->blendState);
}
