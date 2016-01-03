
#include "d11.h"
#include "Mesh.h"
#include "Texture.h"
#include <assert.h>

RenderConstantsBuffer render_constantsBuffer_cpu;
ID3D11Buffer* render_constantsBuffer;

LightingInfoBuffer lighting_InfoBuffer_cpu;
ID3D11Buffer* lighting_InfoBuffer;

ID3D11Buffer* fullScreenVertexBuffer = nullptr;
ID3D11Buffer* fullScreenIndexBuffer = nullptr;

//global variables
bool g_vsync_enabled;
int g_videoCardMemory;
char g_videoCardDescription[128];

IDXGISwapChain* g_swapChain;
ID3D11Device* g_device;
ID3D11DeviceContext* g_deviceContext;

ID3D11RenderTargetView* g_renderTargetView;
ID3D11Texture2D* g_depthStencilTexture;
ID3D11ShaderResourceView* g_depthStencilTextureSRV = nullptr;

ID3D11DepthStencilState* g_depthStencilState;
ID3D11DepthStencilView* g_depthStencilView;
ID3D11RasterizerState* g_rasterState;

TextureOutputToScreenFunctionality* texture_outputter;

ID3D11InputLayout* g_layout;

D3DXMATRIX g_projectionMatrix;
D3DXMATRIX g_worldMatrix;
D3DXMATRIX g_orthoMatrix;

extern int g_screenWidth;
extern int g_screenHeight;
extern HWND g_hwnd;
extern double g_time;

using namespace std;

static const int max_render_targets = 8;

ID3D11RenderTargetView *currentRenderTargetViews[max_render_targets];
ID3D11DepthStencilView *currentDepthStencilView = nullptr;

//font rendering
//IFW1Factory *pFW1Factory;
//IFW1FontWrapper *pFontWrapper;

ID3DUserDefinedAnnotation *pPerf;

ID3D11DepthStencilState *depth_states[number_of_depth_states];
ID3D11BlendState *blend_states[number_of_blend_states];

enum SamplerType
{
	point = 0,
	linear_mip_map,
};

ID3D11SamplerState *sampler_states[2];

void UpdateGlobalBuffers()
{
	UpdateBuffer((float*)&render_constantsBuffer_cpu, sizeof(RenderConstantsBuffer), render_constantsBuffer);
	SetConstantBufferForRendering(0, render_constantsBuffer);

	UpdateBuffer((float*)&lighting_InfoBuffer_cpu, sizeof(LightingInfoBuffer), lighting_InfoBuffer);
	SetConstantBufferForRendering(1, lighting_InfoBuffer);
}

bool init_engine()
{
	HRESULT result;
	IDXGIFactory* factory;
	IDXGIAdapter* adapter;
	IDXGIOutput* adapterOutput;
	unsigned int numModes, i, numerator, denominator, stringLength;
	DXGI_MODE_DESC* displayModeList;
	DXGI_ADAPTER_DESC adapterDesc;
	int error;
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	D3D_FEATURE_LEVEL featureLevel;
	ID3D11Texture2D* backBufferPtr;
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	D3D11_RASTERIZER_DESC rasterDesc;
	D3D11_VIEWPORT viewport;
	float fieldOfView, screenAspect;

	for (int i = 0; i < max_render_targets; i++)
	{
		currentRenderTargetViews[i] = nullptr;
	}

	g_swapChain = 0;
	g_device = 0;
	g_deviceContext = 0;
	g_renderTargetView = 0;
	g_depthStencilTexture = 0;
	g_depthStencilState = 0;
	g_depthStencilView = 0;
	g_rasterState = 0;


	// Create a DirectX graphics interface factory.
	result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**) &factory);
	if (FAILED(result))
	{
		return false;
	}

	// Use the factory to create an adapter for the primary graphics interface (video card).
	result = factory->EnumAdapters(0, &adapter);
	if (FAILED(result))
	{
		return false;
	}

	// Enumerate the primary adapter output (monitor).
	result = adapter->EnumOutputs(0, &adapterOutput);
	if (FAILED(result))
	{
		return false;
	}

	// Get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output (monitor).
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
	if (FAILED(result))
	{
		return false;
	}

	// Create a list to hold all the possible display modes for this monitor/video card combination.
	displayModeList = new DXGI_MODE_DESC[numModes];
	if (!displayModeList)
	{
		return false;
	}

	// Now fill the display mode list structures.
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
	if (FAILED(result))
	{
		return false;
	}

	// Now go through all the display modes and find the one that matches the screen width and height.
	// When a match is found store the numerator and denominator of the refresh rate for that monitor.
	for (i = 0; i < numModes; i++)
	{
		if (displayModeList[i].Width == (unsigned int) g_screenWidth)
		{
			if (displayModeList[i].Height == (unsigned int) g_screenHeight)
			{
				numerator = displayModeList[i].RefreshRate.Numerator;
				denominator = displayModeList[i].RefreshRate.Denominator;
			}
		}
	}

	// Get the adapter (video card) description.
	result = adapter->GetDesc(&adapterDesc);
	if (FAILED(result))
	{
		return false;
	}

	// Store the dedicated video card memory in megabytes.
	g_videoCardMemory = (int) (adapterDesc.DedicatedVideoMemory / 1024 / 1024);

	// Convert the name of the video card to a character array and store it.
	error = wcstombs_s(&stringLength, g_videoCardDescription, 128, adapterDesc.Description, 128);
	if (error != 0)
	{
		return false;
	}

	// Release the display mode list.
	delete [] displayModeList;
	displayModeList = 0;

	// Release the adapter output.
	adapterOutput->Release();
	adapterOutput = 0;

	// Release the adapter.
	adapter->Release();
	adapter = 0;

	// Release the factory.
	factory->Release();
	factory = 0;

	// Initialize the swap chain description.
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	// Set to a single back buffer.
	swapChainDesc.BufferCount = 1;

	// Set the width and height of the back buffer.
	swapChainDesc.BufferDesc.Width = g_screenWidth;
	swapChainDesc.BufferDesc.Height = g_screenHeight;

	// Set regular 32-bit surface for the back buffer.
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// Set the refresh rate of the back buffer.

	swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	
	// Set the usage of the back buffer.
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	// Set the handle for the window to render to.
	swapChainDesc.OutputWindow = g_hwnd;

	// Turn multisampling off.
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	// Set to full screen or windowed mode.
	swapChainDesc.Windowed = true;


	// Set the scan line ordering and scaling to unspecified.
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// Discard the back buffer contents after presenting.
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	// Don't set the advanced flags.
	swapChainDesc.Flags = 0;

	// Set the feature level to DirectX 11.
	featureLevel = D3D_FEATURE_LEVEL_11_0;

	// Create the swap chain, Direct3D device, and Direct3D device context.
	result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &featureLevel, 1, D3D11_SDK_VERSION, &swapChainDesc, &g_swapChain, &g_device, NULL, &g_deviceContext);
	if (FAILED(result))
	{
		return false;
	}


	// Get the pointer to the back buffer.
	result = g_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*) &backBufferPtr);
	if (FAILED(result))
	{
		return false;
	}

	// Create the render target view with the back buffer pointer.
	result = g_device->CreateRenderTargetView(backBufferPtr, NULL, &g_renderTargetView);
	if (FAILED(result))
	{
		return false;
	}

	// Release pointer to the back buffer as we no longer need it.
	backBufferPtr->Release();
	backBufferPtr = 0;

	// Initialize the description of the depth buffer.
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// Set up the description of the depth buffer.
	depthBufferDesc.Width = g_screenWidth;
	depthBufferDesc.Height = g_screenHeight;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	// Create the texture for the depth buffer using the filled out description.
	result = g_device->CreateTexture2D(&depthBufferDesc, NULL, &g_depthStencilTexture);
	if (FAILED(result))
	{
		return false;
	}

	g_depthStencilTextureSRV = CreateTextureResourceView(g_depthStencilTexture, DXGI_FORMAT_R32_FLOAT, 0, 1);

	const char *name = "depth texture";
	g_depthStencilTexture->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(name)-1, name);

	// Initialize the description of the stencil state.
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	// Set up the description of the stencil state.
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = false;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing.
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;

	// Stencil operations if pixel is back-facing.
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Create the depth stencil state.
	result = g_device->CreateDepthStencilState(&depthStencilDesc, &g_depthStencilState);
	if (FAILED(result))
	{
		return false;
	}

	// Set the depth stencil state.
	g_deviceContext->OMSetDepthStencilState(g_depthStencilState, 1);

	// Initialize the depth stencil view.
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	// Set up the depth stencil view description.
	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	// Create the depth stencil view.
	result = g_device->CreateDepthStencilView(g_depthStencilTexture, &depthStencilViewDesc, &g_depthStencilView);
	if (FAILED(result))
	{
		return false;
	}

	SetRenderTargetView(g_renderTargetView);
	SetDepthStencilView(g_depthStencilView);

	// Setup the raster description which will determine how and what polygons will be drawn.
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_NONE;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	// Create the rasterizer state from the description we just filled out.
	result = g_device->CreateRasterizerState(&rasterDesc, &g_rasterState);
	if (FAILED(result))
	{
		return false;
	}

	// Now set the rasterizer state. 
	g_deviceContext->RSSetState(g_rasterState);

	// Setup the viewport for rendering.
	viewport.Width = (float) g_screenWidth;
	viewport.Height = (float) g_screenHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	// Create the viewport.
	g_deviceContext->RSSetViewports(1, &viewport);

	// Setup the projection matrix.
	fieldOfView = (float) D3DX_PI / 4.0f;
	screenAspect = (float) g_screenWidth / (float) g_screenHeight;

	// Create the projection matrix for 3D rendering.
	D3DXMatrixPerspectiveFovLH(&g_projectionMatrix, fieldOfView, screenAspect, 0.01, 100);

	// Initialize the world matrix to the identity matrix.
	D3DXMatrixIdentity(&g_worldMatrix);

	// Create an orthographic projection matrix for 2D rendering.
	D3DXMatrixOrthoLH(&g_orthoMatrix, (float) g_screenWidth, (float) g_screenHeight, 0.01, 100);

	//HRESULT hResult = FW1CreateFactory(FW1_VERSION, &pFW1Factory);
	//pFW1Factory->CreateFontWrapper(g_device, L"Arial", &pFontWrapper);

	HRESULT hr = g_deviceContext->QueryInterface( __uuidof(pPerf), reinterpret_cast<void**>(&pPerf) );

	CreateDepthStencilStates();
	CreateBlendStates();
	CreateSamplerStates();

	texture_outputter = new TextureOutputToScreenFunctionality;

	render_constantsBuffer = CreateConstantBuffer(sizeof(RenderConstantsBuffer));
	lighting_InfoBuffer = CreateConstantBuffer(sizeof(LightingInfoBuffer));
}

void closeEngine()
{
	// Before shutting down set to windowed mode or when you release the swap chain it will throw an exception.
	if (g_swapChain)
	{
		g_swapChain->SetFullscreenState(false, NULL);
	}

	if (g_rasterState)
	{
		g_rasterState->Release();
		g_rasterState = 0;
	}

	if (g_depthStencilView)
	{
		g_depthStencilView->Release();
		g_depthStencilView = 0;
	}

	if (g_depthStencilState)
	{
		g_depthStencilState->Release();
		g_depthStencilState = 0;
	}

	if (g_depthStencilTexture)
	{
		g_depthStencilTexture->Release();
		g_depthStencilTexture = 0;
	}

	if (g_renderTargetView)
	{
		g_renderTargetView->Release();
		g_renderTargetView = 0;
	}

	if (g_deviceContext)
	{
		g_deviceContext->Release();
		g_deviceContext = 0;
	}

	if (g_device)
	{
		g_device->Release();
		g_device = 0;
	}

	if (g_swapChain)
	{
		g_swapChain->Release();
		g_swapChain = 0;
	}

	return;
}

void BeginScene()
{
	return;
}

void EndScene()
{
	// Present the back buffer to the screen since rendering is complete.
	g_swapChain->Present(0, 0);
	return;
}

void ClearRenderView(D3DXVECTOR4 col, int slot)
{
	float color[4];
	color[0] = col.x;
	color[1] = col.y;
	color[2] = col.z;
	color[3] = col.w;

	if (currentRenderTargetViews != nullptr)
	{
		g_deviceContext->ClearRenderTargetView(currentRenderTargetViews[slot], color);
	}
}

void clearScreen(D3DXVECTOR4 col /*=D3DXVECTOR4(0,0,0,0) */, float depth /*= 1*/)
{
	float color[4];
	color[0] = col.x;
	color[1] = col.y;
	color[2] = col.z;
	color[3] = col.w;

	if (currentRenderTargetViews != nullptr)
	{
		g_deviceContext->ClearRenderTargetView(currentRenderTargetViews[0], color);
	}

	if (currentDepthStencilView != nullptr)
	{
		// Clear the depth buffer.
		g_deviceContext->ClearDepthStencilView(currentDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

}

void OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd)
{
	char* compileErrors;
	unsigned long bufferSize, i;
	ofstream fout;


	// Get a pointer to the error message text buffer.
	compileErrors = (char*) (errorMessage->GetBufferPointer());




	// Release the error message.
	errorMessage->Release();
	errorMessage = 0;

	// Pop a message up on the screen to notify the user to check the text file for compile errors.
	MessageBox(hwnd, (LPCWSTR)compileErrors, L"", MB_OK);

	return;
}

void CreateDepthStencilStates()
{
	{
		D3D11_DEPTH_STENCIL_DESC desc;
		D3D11_DEPTH_STENCILOP_DESC st_desc;
		memset(&desc, 0, sizeof(D3D11_DEPTH_STENCIL_DESC));
		st_desc.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		st_desc.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		desc.DepthEnable = true;
		desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		g_device->CreateDepthStencilState(&desc, depth_states + depth_state_enable_test_enable_write);
	}
	
	{
		D3D11_DEPTH_STENCIL_DESC desc;
		D3D11_DEPTH_STENCILOP_DESC st_desc;
		memset(&desc, 0, sizeof(D3D11_DEPTH_STENCIL_DESC));
		st_desc.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		st_desc.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		desc.DepthEnable = false;
		desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		g_device->CreateDepthStencilState(&desc, depth_states + depth_state_disable_test_enable_write);
	}

	{
		D3D11_DEPTH_STENCIL_DESC desc;
		D3D11_DEPTH_STENCILOP_DESC st_desc;
		memset(&desc, 0, sizeof(D3D11_DEPTH_STENCIL_DESC));
		st_desc.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		st_desc.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		desc.DepthEnable = true;
		desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		desc.FrontFace = st_desc;
		desc.BackFace = st_desc;
		g_device->CreateDepthStencilState(&desc, depth_states + depth_state_enable_test_disable_write);
	}

	{
		D3D11_DEPTH_STENCIL_DESC desc;
		D3D11_DEPTH_STENCILOP_DESC st_desc;
		memset(&desc, 0, sizeof(D3D11_DEPTH_STENCIL_DESC));
		st_desc.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		st_desc.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP; 
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		desc.DepthEnable = false;
		desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		desc.FrontFace = st_desc;
		desc.BackFace = st_desc;
		g_device->CreateDepthStencilState(&desc, depth_states + depth_state_disable_test_disable_write);
	}
}

void CreateBlendStates()
{
	{
		D3D11_BLEND_DESC desc;
		D3D11_RENDER_TARGET_BLEND_DESC rt_desc;
		memset(&desc, 0, sizeof(D3D11_RENDER_TARGET_BLEND_DESC));
		memset(&rt_desc, 0, sizeof(D3D11_RENDER_TARGET_BLEND_DESC));
		rt_desc.RenderTargetWriteMask = 0;
		desc.RenderTarget[0] = rt_desc;
		g_device->CreateBlendState(&desc, blend_states + blend_state_disable_color_write);
	}

	{
		D3D11_BLEND_DESC desc;
		D3D11_RENDER_TARGET_BLEND_DESC rt_desc;
		memset(&rt_desc, 0, sizeof(D3D11_RENDER_TARGET_BLEND_DESC));
		memset(&desc, 0, sizeof(D3D11_RENDER_TARGET_BLEND_DESC));
		rt_desc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		desc.RenderTarget[0] = rt_desc;
		g_device->CreateBlendState(&desc, blend_states + blend_state_enable_color_write);
	}

	{
		D3D11_BLEND_DESC desc;
		D3D11_RENDER_TARGET_BLEND_DESC rt_desc;
		memset(&rt_desc, 0, sizeof(D3D11_RENDER_TARGET_BLEND_DESC));
		memset(&desc, 0, sizeof(D3D11_RENDER_TARGET_BLEND_DESC));
		rt_desc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		rt_desc.BlendEnable = true;
		rt_desc.SrcBlend = D3D11_BLEND_SRC_COLOR;
		rt_desc.DestBlend = D3D11_BLEND_BLEND_FACTOR;
		rt_desc.BlendOp = D3D11_BLEND_OP_ADD;
		rt_desc.SrcBlendAlpha = D3D11_BLEND_ONE;
		rt_desc.DestBlendAlpha = D3D11_BLEND_ZERO;
		rt_desc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0] = rt_desc;
		g_device->CreateBlendState(&desc, blend_states + blend_state_alpha_blend_add);
	}

	
}

ID3D11SamplerState* CreateSampler(SamplerType type)
{
	ID3D11SamplerState* sampler_state;
	D3D11_SAMPLER_DESC desc;
	// Create a texture sampler state description.
	if (type == linear_mip_map)
	{
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	}
	if (type == point)
	{
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	}

	desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.MipLODBias = 0.0f;
	desc.MaxAnisotropy = 1;
	desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	desc.BorderColor[0] = 0;
	desc.BorderColor[1] = 0;
	desc.BorderColor[2] = 0;
	desc.BorderColor[3] = 0;
	desc.MinLOD = 0;
	desc.MaxLOD = D3D11_FLOAT32_MAX;
	g_device->CreateSamplerState(&desc, &sampler_state);

	return sampler_state;
}

void CreateSamplerStates()
{
	sampler_states[point] = CreateSampler(point);
	sampler_states[linear_mip_map] = CreateSampler(linear_mip_map);
}

ID3D11Buffer* CreateFullScreenQuadVertexBuffer()
{
	Mesh::Vertex vertices[4];
	memset(vertices, 0, sizeof(Mesh::Vertex) * 4);

	vertices[0].position = D3DXVECTOR4(-1, -1, 0, 1);
	vertices[1].position = D3DXVECTOR4(1, -1, 0, 1);
	vertices[2].position = D3DXVECTOR4(1, 1, 0, 1);
	vertices[3].position = D3DXVECTOR4(-1, 1, 0, 1);

	vertices[0].texture_coord = D3DXVECTOR4(0, 1, 0, 1);
	vertices[1].texture_coord = D3DXVECTOR4(1, 1, 0, 1);
	vertices[2].texture_coord = D3DXVECTOR4(1, 0, 0, 1);
	vertices[3].texture_coord = D3DXVECTOR4(0, 0, 0, 1);

	return CreateVertexBuffer(4, vertices, sizeof(Mesh::Vertex));
}

ID3D11Buffer* CreateFullScreenQuadIndexBuffer()
{
	int indices[6];
	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;

	indices[3] = 0;
	indices[4] = 2;
	indices[5] = 3;

	return CreateIndexBuffer(6, indices, sizeof(int));

}

void RenderFullScreenQuad()
{
	if (fullScreenVertexBuffer == nullptr)
	{
		fullScreenVertexBuffer = CreateFullScreenQuadVertexBuffer();
		fullScreenIndexBuffer = CreateFullScreenQuadIndexBuffer();
	}

	SetVertexBuffer(fullScreenVertexBuffer, sizeof(Mesh::Vertex));
	SetIndexBuffer(fullScreenIndexBuffer);

	RenderIndexed(6);
}

void SetBlendState(blendState state)
{
	g_deviceContext->OMSetBlendState(blend_states[state], nullptr, 0xFFFFFFFF);
}

ID3D11Query* createQuery(D3D11_QUERY type)
{
	D3D11_QUERY_DESC queryDesc;
	queryDesc.Query = type;
	queryDesc.MiscFlags = 0;
	ID3D11Query * pQuery;
	g_device->CreateQuery(&queryDesc, &pQuery);
	return pQuery;
}

ID3D11Buffer* CreateConstantBuffer( int byteWidth )
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	ID3D11Buffer *constant_buffer;

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = byteWidth;
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	HRESULT result = g_device->CreateBuffer(&matrixBufferDesc, NULL, &constant_buffer);
	return constant_buffer;
}

ID3D11VertexShader* CreateVertexShader(std::string shader_name)
{
	HRESULT result;
	ID3D10Blob* errorMessage;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[5];
	unsigned int numElements;
	D3D11_BUFFER_DESC matrixBufferDesc;

	ID3D11VertexShader *m_vertexShader;

	// Initialize the pointers this function will use to null.
	errorMessage = 0;

	std::string full_path = "..\\Shaders\\Assemblies\\";
	full_path.append(shader_name);
	full_path.append(".cso");

	std::ifstream myFile(full_path, std::ios::in | std::ios::binary | std::ios::ate); //replace with the name of your shader
	size_t fileSize = myFile.tellg();
	myFile.seekg(0, std::ios::beg);
	char* shaderData = new char[fileSize];
	myFile.read(shaderData, fileSize);
	myFile.close();

	// Create the vertex shader from the buffer.
	result = g_device->CreateVertexShader(shaderData, fileSize, NULL, &m_vertexShader);
	if (FAILED(result))
	{
		return NULL;
	}

	// Now setup the layout of the data that goes into the shader.
	// This setup needs to match the VertexType stucture in the ModelClass and in the shader.
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "COLOR";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	polygonLayout[2].SemanticName = "TEXCOORD";
	polygonLayout[2].SemanticIndex = 0;
	polygonLayout[2].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	polygonLayout[2].InputSlot = 0;
	polygonLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[2].InstanceDataStepRate = 0;

	polygonLayout[3].SemanticName = "NORMAL";
	polygonLayout[3].SemanticIndex = 0;
	polygonLayout[3].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	polygonLayout[3].InputSlot = 0;
	polygonLayout[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[3].InstanceDataStepRate = 0;

	polygonLayout[4].SemanticName = "TANGENT";
	polygonLayout[4].SemanticIndex = 0;
	polygonLayout[4].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	polygonLayout[4].InputSlot = 0;
	polygonLayout[4].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[4].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[4].InstanceDataStepRate = 0;

	// Get a count of the elements in the layout.
	numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// Create the vertex input layout.
	result = g_device->CreateInputLayout(polygonLayout, numElements, shaderData, fileSize, &g_layout);
	if (FAILED(result))
	{
		return false;
	}

	return m_vertexShader;

}

ID3D11Buffer * CreateIndexBuffer(int index_count, void *data, int index_struct_size)
{
	ID3D11Buffer *index_buffer = nullptr;

	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = index_count * index_struct_size;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData;

	iinitData.pSysMem = data;
	g_device->CreateBuffer(&indexBufferDesc, &iinitData, &index_buffer);

	return index_buffer;

}

ID3D11PixelShader* CreatePixelShader(std::string shader_name)
{
	HRESULT result;

	ID3D10Blob* errorMessage;
	ID3D11PixelShader *m_pixelShader;

	std::string full_path = "..\\Shaders\\Assemblies\\";
	full_path.append(shader_name);
	full_path.append(".cso");

	std::ifstream myFile(full_path, std::ios::in | std::ios::binary | std::ios::ate); //replace with the name of your shader
	size_t fileSize = myFile.tellg();
	myFile.seekg(0, std::ios::beg);
	char* shaderData = new char[fileSize];
	myFile.read(shaderData, fileSize);
	myFile.close();

	// Create the pixel shader from the buffer.
	result = g_device->CreatePixelShader(shaderData, fileSize, NULL, &m_pixelShader);
	if (FAILED(result))
	{
		return NULL;
	}


	return m_pixelShader;
}

ID3D11GeometryShader* CreateGeometryShader(std::string shader_name)
{
	HRESULT result;

	ID3D10Blob* errorMessage;
	ID3D11GeometryShader *m_pixelShader;

	std::string full_path = "..\\Shaders\\Assemblies\\";
	full_path.append(shader_name);
	full_path.append(".cso");

	std::ifstream myFile(full_path.c_str(), std::ios::in | std::ios::binary | std::ios::ate); //replace with the name of your shader
	size_t fileSize = myFile.tellg();
	myFile.seekg(0, std::ios::beg);
	char* shaderData = new char[fileSize];
	myFile.read(shaderData, fileSize);
	myFile.close();

	// Create the pixel shader from the buffer.
	result = g_device->CreateGeometryShader(shaderData, fileSize, NULL, &m_pixelShader);
	if (FAILED(result))
	{
		return NULL;
	}


	return m_pixelShader;
}

ID3D11ComputeShader* CreateComputeShader(LPCTSTR name)
{
	HRESULT result;

	ID3D10Blob* errorMessage;
	ID3D11ComputeShader *m_computeShader;

	std::ifstream myFile(name, std::ios::in | std::ios::binary | std::ios::ate); //replace with the name of your shader
	size_t fileSize = myFile.tellg();
	myFile.seekg(0, std::ios::beg);
	char* shaderData = new char[fileSize];
	myFile.read(shaderData, fileSize);
	myFile.close();

	// Create the pixel shader from the buffer.
	result = g_device->CreateComputeShader(shaderData, fileSize, NULL, &m_computeShader);
	if (FAILED(result))
	{
		return NULL;
	}


	return m_computeShader;




}

ID3D11Buffer *CreateVertexBuffer(int vertex_count, void *data, int vertex_struct_size)
{
	D3D11_BUFFER_DESC vertexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData;
	HRESULT result;

	ID3D11Buffer* m_vertexBuffer;


	/*
	// Load the vertex array with data.
	for (int i = 0; i < vertex_count; i++)
	{
		vertices[i].position = D3DXVECTOR4(0.0f,-0.0f, 0.0f, 0.0f);  
		vertices[i].color = D3DXVECTOR4(0.0f, 1.0f, 0.0f, 1.0f);
	}*/

	// Load the vertex array with data.


	// Set up the description of the static vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = vertex_struct_size * vertex_count;
	vertexBufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = data;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
	result = g_device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if (FAILED(result))
	{
		return NULL;
	}

	return m_vertexBuffer;
}

ID3D11Buffer *CreateStructuredBuffer( int vertex_count, float* data, int byteWidth)
{
	ID3D11Buffer *ppBufOut = NULL;

	D3D11_BUFFER_DESC desc;
	ZeroMemory( &desc, sizeof(desc) );
	desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	desc.ByteWidth = byteWidth * vertex_count;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.StructureByteStride = byteWidth;

	if(data)
	{
		D3D11_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = data;
		g_device->CreateBuffer( &desc, &InitData, &ppBufOut );
	}
	else
	{
		g_device->CreateBuffer( &desc, NULL , &ppBufOut );
	}

	return ppBufOut;

}

ID3D11UnorderedAccessView *CreateUnorderedAccessView( ID3D11Buffer* pBuffer)
{
	ID3D11UnorderedAccessView *ppUAVOut;

    D3D11_BUFFER_DESC descBuf;
    ZeroMemory( &descBuf, sizeof(descBuf) );
    pBuffer->GetDesc( &descBuf );
        
    D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
    ZeroMemory( &desc, sizeof(desc) );
    desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    desc.Buffer.FirstElement = 0;


	if ( descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS )
	{
		// This is a Raw Buffer

		desc.Format = DXGI_FORMAT_R32_TYPELESS; // Format must be DXGI_FORMAT_R32_TYPELESS, when creating Raw Unordered Access View
		desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
		desc.Buffer.NumElements = descBuf.ByteWidth / 4; 
	} 
	else if( descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED )
	{
		// This is a Structured Buffer
		desc.Format = DXGI_FORMAT_UNKNOWN;      // Format must be must be DXGI_FORMAT_UNKNOWN, when creating a View of a Structured Buffer
		desc.Buffer.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride; 
	} 

    
    g_device->CreateUnorderedAccessView( pBuffer, &desc, &ppUAVOut );
	return ppUAVOut;
}

ID3D11ShaderResourceView *CreateShaderResourceView(ID3D11Buffer* pBuffer , int vertex_count)
{
	ID3D11ShaderResourceView *srV;
	// Create SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory( &srvDesc, sizeof(srvDesc) );
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.ElementWidth = vertex_count;
	g_device->CreateShaderResourceView( pBuffer, &srvDesc, &srV );

	return srV;

}

ID3D11ShaderResourceView * CreateTextureResourceView(ID3D11Texture2D *text, DXGI_FORMAT format, int mip_map_start, int mip_map_count)
{
	ID3D11ShaderResourceView *srV;
	// Create SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = mip_map_start;
	srvDesc.Texture2D.MipLevels = 1;
	g_device->CreateShaderResourceView(text, &srvDesc, &srV);

	return srV;
}

ID3D11Texture2D *CreateTexture(int width, int height, void *data, DXGI_FORMAT format, int mipmap_count /*= 1*/)
{
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = mipmap_count;

	D3D11_SUBRESOURCE_DATA frameData;
	frameData.pSysMem = data;
	frameData.SysMemSlicePitch = 0;
	frameData.SysMemPitch = texDesc.Width * sizeof(float) * 4;

	texDesc.ArraySize = 1;
	texDesc.Format = format;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texDesc.MiscFlags = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.CPUAccessFlags = 0;

	ID3D11Texture2D *texture;

	if (data)
	{
		g_device->CreateTexture2D(&texDesc, &frameData, &texture);
	}
	else
	{
		g_device->CreateTexture2D(&texDesc, nullptr, &texture);
	}

	return texture;

}

ID3D11Buffer *CreateReadableBuffer(int buffer_size, float* data)
{
	ID3D11Buffer *ppBufOut = NULL;

	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	//desc.BindFlags = ;
	desc.ByteWidth = buffer_size;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc.Usage = D3D11_USAGE_STAGING;

	if (data)
	{
		D3D11_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = data;
		g_device->CreateBuffer(&desc, &InitData, &ppBufOut);
	}
	else
	{
		g_device->CreateBuffer(&desc, NULL, &ppBufOut);
	}

	return ppBufOut;
}

void CopyResourceContents(ID3D11Resource *to, ID3D11Resource *from)
{
	g_deviceContext->CopyResource(to, from);
}

void* MapBuffer(ID3D11Buffer *buffer)
{
	D3D11_MAPPED_SUBRESOURCE info;
	g_deviceContext->Map(buffer, 0, D3D11_MAP_READ, 0, &info);
	return info.pData;
}

void UnMapBuffer(ID3D11Buffer *buffer)
{
	g_deviceContext->Unmap(buffer, 0);
}

void CreateComputeResourceBuffer(ID3D11Buffer** pBuffer, ID3D11UnorderedAccessView** uav, ID3D11ShaderResourceView **srv, int vertex_count, float* data, int bytewidth)
{
	*pBuffer = CreateStructuredBuffer(vertex_count, data, bytewidth);
	*uav = CreateUnorderedAccessView(*pBuffer);
	*srv = CreateShaderResourceView(*pBuffer, vertex_count);
}

void BeginQuery(ID3D11Query *query_object)
{
	g_deviceContext->Begin(query_object);
}

void EndQuery(ID3D11Query *query_object)
{
	g_deviceContext->End(query_object);
}

UINT64 GetQueryData(ID3D11Query *query_object)
{
	UINT64 StartTime = 0;
	while(g_deviceContext->GetData(query_object, &StartTime, sizeof(StartTime), 0) != S_OK);
	return StartTime;
}

void SetViewPort(int width, int height)
{
	// Set the viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = width;
	viewport.Height = height;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;

	g_deviceContext->RSSetViewports(1, &viewport);

	float dx = 0.5f / width;
	float dy = 0.5f / height;

	render_constantsBuffer_cpu.screen_texture_half_pixel.x = dx;
	render_constantsBuffer_cpu.screen_texture_half_pixel.y = dy;
	UpdateGlobalBuffers();
}

void SetVertexBuffer(ID3D11Buffer *  vertex_buffer, int vertex_size)
{
	// Set vertex buffer stride and offset.
	UINT stride = vertex_size;
	UINT offset = 0;
	g_deviceContext->IASetVertexBuffers(0, 1, &vertex_buffer, &stride, &offset);
}

void SetIndexBuffer(ID3D11Buffer * index_buffer)
{
	g_deviceContext->IASetIndexBuffer(index_buffer, DXGI_FORMAT_R32_UINT, 0);
}

void SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY state)
{
	g_deviceContext->IASetPrimitiveTopology(state);
}

void SetConstantBufferForRendering(int start_slot, ID3D11Buffer *buffer_to_set)
{
	g_deviceContext->VSSetConstantBuffers(start_slot, 1, &buffer_to_set);
	g_deviceContext->PSSetConstantBuffers(start_slot, 1, &buffer_to_set);
	g_deviceContext->GSSetConstantBuffers(start_slot, 1, &buffer_to_set);
}

void SetShaders(ID3D11VertexShader *vertex_shader, ID3D11GeometryShader *geometry_shader,  ID3D11PixelShader *pixel_shader)
{
	// Set the vertex and pixel shaders that will be used.
	g_deviceContext->VSSetShader(vertex_shader, NULL, 0);
	g_deviceContext->PSSetShader(pixel_shader, NULL, 0);
	g_deviceContext->GSSetShader(geometry_shader, NULL, 0);
	g_deviceContext->CSSetShader(NULL, NULL, 0);

}

void SetComputeShader(ID3D11ComputeShader *compute_shader)
{
	g_deviceContext->CSSetShader(compute_shader, NULL, 0);
}

void setCShaderUAVResources(ID3D11UnorderedAccessView **uav_list, int number_of_resources)
{
	g_deviceContext->CSSetUnorderedAccessViews( 0, number_of_resources, uav_list, NULL );
}

void SetCShaderRV(ID3D11ShaderResourceView **srv_list, int number_of_resources)
{
	g_deviceContext->CSSetShaderResources( 0, number_of_resources, srv_list);
}

void SetSRV(ID3D11ShaderResourceView **uav_list, int number_of_resources, shaderType shader_type, int slot)
{
	if (shader_type == shader_type_vertex)
	{
		g_deviceContext->VSSetShaderResources(slot, number_of_resources, uav_list);
	}
	else if (shader_type == shader_type_pixel)
	{
		g_deviceContext->PSSetShaderResources(slot, number_of_resources, uav_list);
	}
}

void SetSRV(Texture *texture, shaderType shader_type, int slot)
{
	ID3D11ShaderResourceView* list = texture ? texture->get_srv() : nullptr;
	SetSRV(&list, 1, shader_type, slot);
}

void SetDepthState(int depth_state)
{
	g_deviceContext->OMSetDepthStencilState(depth_states[depth_state], 1);
}

void SetBlendState(int blend_state)
{
	g_deviceContext->OMSetBlendState(blend_states[blend_state], NULL, 0xffffffff);
}

void UpdateBuffer(float *data , int byteWidth, ID3D11Buffer* buffer)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	float* dataPtr;
	unsigned int bufferNumber;

	// Lock the constant buffer so it can be written to.
	result = g_deviceContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return;
	}

	// Get a pointer to the data in the constant buffer.
	dataPtr = (float*) mappedResource.pData;

	// Copy the matrices into the constant buffer.
	memcpy(dataPtr,data,byteWidth);

	// Unlock the constant buffer.
	g_deviceContext->Unmap(buffer, 0);
}

ID3D11ShaderResourceView *GetDepthTextureSRV()
{
	return g_depthStencilTextureSRV;
}

void setComputeConstantBuffer(ID3D11Buffer* buffer, int index)
{
	g_deviceContext->CSSetConstantBuffers(index, 1, &buffer);
}

void SetSamplerState()
{
	g_deviceContext->PSSetSamplers(0, 2, sampler_states);
	//g_deviceContext->PSSetSamplers(1, 1, &sampler_state);
}


void SetRenderViews(ID3D11RenderTargetView *view, ID3D11DepthStencilView *depth_view, int rt_slot)
{
	currentRenderTargetViews[rt_slot] = view;
	currentDepthStencilView = depth_view;

	g_deviceContext->OMSetRenderTargets(1, &view, depth_view);
}


ID3D11DepthStencilView* GetDefaultDepthView()
{
	return g_depthStencilView;
}

void DispatchComputeShader(int thread_count_x, int thread_count_y, int thread_count_z)
{
	g_deviceContext->Dispatch( thread_count_x, thread_count_y, thread_count_z );
}

void Render(int vertext_count)
{
	// Set the vertex input layout.
	g_deviceContext->IASetInputLayout(g_layout);
	
	//render
	g_deviceContext->Draw(vertext_count, 0);

}

void RenderIndexed(int index_count)
{
	// Set the vertex input layout.
	g_deviceContext->IASetInputLayout(g_layout);

	//render
	g_deviceContext->DrawIndexed(index_count, 0, 0);
}

void Flush()
{
	g_deviceContext->Flush();
}

void drawText(int fps, int y_pos) 
{
	std::wstring s = std::to_wstring(fps);

	//pFontWrapper->DrawString(
	//	g_deviceContext,
	//	(const WCHAR *)(s.c_str()),// String
	//	20.0f,// Font size
	//	570.0f,// X position
	//	y_pos,// Y position
	//	0xff0099ff,// Text color, 0xAaBbGgRr
	//	0// Flags (for example FW1_RESTORESTATE to keep context states unchanged)
	//);
	
	//pFontWrapper->Release();
	//pFW1Factory->Release();
}

void OutputTextureToScreen(Texture* texture, D3DXVECTOR4 pos, D3DXVECTOR4 scale, int forced_lod, ID3D11PixelShader *enforced_pixel_shader)
{
	OutputTextureToScreen(texture->get_srv(), pos, scale, forced_lod);
}

void OutputTextureToScreen(ID3D11ShaderResourceView* texture, D3DXVECTOR4 pos, D3DXVECTOR4 scale, int forced_lod, ID3D11PixelShader *enforced_pixel_shader)
{
	SetRenderViews(GetDefaultRenderTargetView() , nullptr, 0);
	texture_outputter->OutputTextureToScreen(texture, pos, scale, forced_lod, enforced_pixel_shader);
	SetRenderViews(GetDefaultRenderTargetView(), GetDefaultDepthStencilView(), 0);
}

ID3D11RenderTargetView* CreateRenderTargetView(ID3D11Texture2D* texture, int mip_map /*= 0*/, DXGI_FORMAT format /*= DXGI_FORMAT_UNKNOWN*/)
{
	ID3D11RenderTargetView *render_view = nullptr;

	// Create the render target view with the back buffer pointer.
	if (mip_map == 0)
	{
		g_device->CreateRenderTargetView(texture, NULL, &render_view);
	}
	else
	{
		D3D11_RENDER_TARGET_VIEW_DESC desc;
		desc.Format = format;
		desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = mip_map;
		g_device->CreateRenderTargetView(texture, &desc, &render_view);
	}

	assert(render_view);

	return render_view;
}

void CopySubResource(ID3D11Texture2D* source_texture, ID3D11Texture2D* destination_texture, int width, int height, int destination_subresource, int source_subresource)
{
	D3D11_BOX sourceRegion;
	sourceRegion.left = 0;
	sourceRegion.right = width;
	sourceRegion.top = 0;
	sourceRegion.bottom = height;
	sourceRegion.front = 0;
	sourceRegion.back = 1;

	g_deviceContext->CopySubresourceRegion(destination_texture, destination_subresource, 0, 0, 0, source_texture, source_subresource, &sourceRegion);
}

void CopySubResource(Texture* source_texture, Texture* destination_texture, int width, int height, int destination_subresource, int source_subresource)
{
	CopySubResource(source_texture->get_texture_object(), destination_texture->get_texture_object(), width, height, destination_subresource, source_subresource);
}

void SetDepthStencilView(ID3D11DepthStencilView *view)
{
	currentDepthStencilView = view;
	SetViewTargetsAux();
}

void SetRenderTargetView(ID3D11RenderTargetView *view, int slot /*= 0*/)
{
	currentRenderTargetViews[slot] = view;
	SetViewTargetsAux();
}


ID3D11RenderTargetView* GetDefaultRenderTargetView()
{
	return g_renderTargetView;
}

ID3D11DepthStencilView* GetDefaultDepthStencilView()
{
	return g_depthStencilView;
}

void SetViewTargetsAux()
{
	g_deviceContext->OMSetRenderTargets(max_render_targets, currentRenderTargetViews, currentDepthStencilView);
}

void SetViewPortToDefault()
{
	SetViewPort(g_screenWidth, g_screenHeight);
}

void TextureOutputToScreenFunctionality::OutputTextureToScreen(ID3D11ShaderResourceView* texture, D3DXVECTOR4 pos, D3DXVECTOR4 scale, int forced_lod, ID3D11PixelShader *enforced_pixel_shader)
{
	SetDepthState(depth_state_disable_test_disable_write);
	SetSRV(&texture, 1, shaderType::shader_type_pixel, 0);

	D3DXMATRIX matrix;
	D3DXMatrixIdentity(&matrix);
	SetViewPortToDefault();

	matrix._14 = pos.x;
	matrix._24 = pos.y;

	matrix._11 = scale.x;
	matrix._22 = scale.y;
	matrix._33 = scale.z;
	matrix._44 = 1;

	render_constantsBuffer_cpu.WorldViewProjectionMatrix = matrix;
	render_constantsBuffer_cpu.screen_texture_half_pixel.z = -1;

	UpdateGlobalBuffers();

	ID3D11PixelShader *pixel_shader_to_use = textureOutputPixelShader;
	if (enforced_pixel_shader != nullptr)
	{
		pixel_shader_to_use = enforced_pixel_shader;
	}

	SetShaders(textureOutputVertexShader, nullptr, pixel_shader_to_use);
	SetVertexBuffer(fullScreenQuadVertexBuffer, sizeof(Mesh::Vertex));
	SetIndexBuffer(fullScreenQuadIndexBuffer);
	SetSamplerState();

	RenderIndexed(6);

	ID3D11ShaderResourceView *null_srv = nullptr;
	SetSRV(&null_srv, 1, shaderType::shader_type_pixel, 0);
}

TextureOutputToScreenFunctionality::TextureOutputToScreenFunctionality()
{
	fullScreenQuadVertexBuffer = CreateFullScreenQuadVertexBuffer();
	fullScreenQuadIndexBuffer = CreateFullScreenQuadIndexBuffer();

	textureOutputVertexShader = CreateVertexShader("texture_output_v");
	textureOutputPixelShader = CreatePixelShader("texture_output_p");
}
