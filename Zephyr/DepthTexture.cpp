
#include "DepthTexture.h"

DepthTexture::DepthTexture(const D3DXVECTOR2 & dimension, void * data, DXGI_FORMAT format, int msaa_count, int mipmap_count)
{
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	// Initialize the description of the depth buffer.
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// Set up the description of the depth buffer.
	depthBufferDesc.Width = dimension.x;
	depthBufferDesc.Height = dimension.y;
	depthBufferDesc.MipLevels = mipmap_count;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = format;
	depthBufferDesc.SampleDesc.Count = msaa_count;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	srv_ = nullptr;
	// Create the texture for the depth buffer using the filled out description.
	HRESULT result = g_device->CreateTexture2D(&depthBufferDesc, NULL, &texture_object_);
	//srv_ = CreateTextureResourceView(texture_object_, format, 0, 1, D3D_SRV_DIMENSION_TEXTURE2D);

	// Set up the depth stencil view description.
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	depthStencilViewDesc.Format = format;
	depthStencilViewDesc.ViewDimension = msaa_count > 1 ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	// Create the depth stencil view.
	result = g_device->CreateDepthStencilView(texture_object_, &depthStencilViewDesc, &depth_stencil_view_);
}

void DepthTexture::set_as_depth_stencil_view()
{
	SetDepthStencilView(depth_stencil_view_);
}

void DepthTexture::set_as_srv(shaderType type, int slot)
{
	SetSRV(&srv_, 1, type, slot);
}
