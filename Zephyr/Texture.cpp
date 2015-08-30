
#include "Texture.h"

Texture::Texture()
{
	texture_object_	= nullptr;
	texture_srv_ = nullptr;

	width_ = -1;
	height_ = -1;
}

Texture::Texture(int w, int h, void *data, DXGI_FORMAT format, int mipmap_count /*= 1*/)
{
	create(w, h, data, format, mipmap_count);
}

void Texture::create(int w, int h, void *data, DXGI_FORMAT format, int mipmap_count /*= 1*/)
{
	texture_object_ = CreateTexture(w, h, data, format, mipmap_count);
	texture_srv_ = CreateTextureResourceView(texture_object_, format, 0, 1);
	texture_rt_ = CreateRenderTargetView(texture_object_);

	width_ = w;
	height_ = h;
	format_ = format;
}

ID3D11ShaderResourceView* Texture::get_srv(int mip_map /*= 0*/)
{
	if (mip_map == 0)
	{
		return texture_srv_;
	}
	else
	{
		validate_mipmap_srv(mip_map);
		return mipmap_srvs_[mip_map];
	}
}

void Texture::set_as_render_target(int slot, int mip_map /*= 0*/)
{
	if (mip_map == 0)
	{
		SetRenderTargetView(texture_rt_, slot);
	}
	else
	{
		validate_mipmap_rt(mip_map);
		SetRenderTargetView(mipmap_rts_[mip_map], slot);
	}
}

ID3D11Texture2D* Texture::get_texture_object() const
{
	return texture_object_;

}

ID3D11RenderTargetView* Texture::get_rt() const
{
	return texture_rt_;
}

void Texture::set_srv_to_shader(shaderType type, int slot, int mip_map_to_set /*= 0*/)
{
	if (mip_map_to_set == 0)
	{
		SetSRV(&texture_srv_, 1, type, slot);
	}
	else
	{
		validate_mipmap_srv(mip_map_to_set);
		ID3D11ShaderResourceView *srv = mipmap_srvs_[mip_map_to_set];
		SetSRV(&srv, 1, type, slot);
	}
}

void Texture::validate_mipmap_rt(int mip_map)
{
	if (mipmap_rts_.find(mip_map) == mipmap_rts_.end())
	{
		mipmap_rts_[mip_map] = CreateRenderTargetView(texture_object_, mip_map, format_);
	}
}

void Texture::validate_mipmap_srv(int mip_map)
{
	if (mipmap_srvs_.find(mip_map) == mipmap_srvs_.end())
	{
		mipmap_srvs_[mip_map] = CreateTextureResourceView(texture_object_, format_ , mip_map, 1);
	}
}



