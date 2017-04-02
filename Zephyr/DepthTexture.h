
#ifndef USE_DEPTH_TEXTURE_
#define USE_DEPTH_TEXTURE_

#include "d11.h"

class DepthTexture
{
public:
	DepthTexture(const D3DXVECTOR2 &dimension, void *data, DXGI_FORMAT format, int msaa_count, int mipmap_count = 1);

	ID3D11Texture2D* get_texture_object() const
	{
		return texture_object_;
	}

	ID3D11DepthStencilView* get_depth_stencil_view() const
	{
		return depth_stencil_view_;
	}

	ID3D11ShaderResourceView* get_srv() const
	{
		return srv_;
	}

	void set_as_depth_stencil_view();
	void set_as_srv(shaderType type, int slot);

private:
	ID3D11Texture2D* texture_object_;
	ID3D11DepthStencilView* depth_stencil_view_;
	ID3D11ShaderResourceView *srv_;

};


#endif