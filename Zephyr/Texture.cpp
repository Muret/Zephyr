
#include "Texture.h"

Texture::Texture()
{
	texture_object_	= nullptr;
	texture_srv_ = nullptr;
	texture_uav_ = nullptr;
	data_accesor_staging_texture_ = nullptr;

	dimension_ = D3DXVECTOR3 (-1 , -1 , -1);
}

Texture::Texture(const D3DXVECTOR3 &dimension, void *data, DXGI_FORMAT format, UINT creation_flags, int msaa_count, int mipmap_count /*= 1*/)
{
	create(dimension, data, format, creation_flags, msaa_count, mipmap_count);
}

void Texture::create(const D3DXVECTOR3 &dimension, void *data, DXGI_FORMAT format, UINT creation_flags, int msaa_count, int mipmap_count /*= 1*/)
{
	dimension_type_ = dimension.z == 1 ? Dim_2D : Dim_3D;

	if (dimension_type_ == Dim_2D)
	{
		texture_object_ = CreateTexture2D(dimension.x, dimension.y, data, format, creation_flags, msaa_count, mipmap_count);
	}
	else
	{
		texture_object_ = CreateTexture3D(dimension.x, dimension.y, dimension.z, data, format, creation_flags, mipmap_count);
	}

	D3D_SRV_DIMENSION device_srv_dim_type = dimension_type_ == Dim_3D ? D3D_SRV_DIMENSION_TEXTURE3D : D3D_SRV_DIMENSION_TEXTURE2D;
	D3D11_RTV_DIMENSION device_rtv_dim_type = dimension_type_ == Dim_3D ? D3D11_RTV_DIMENSION_TEXTURE3D : D3D11_RTV_DIMENSION_TEXTURE2D;

	if (msaa_count > 1)
	{
		device_srv_dim_type = D3D_SRV_DIMENSION_TEXTURE2DMS; 
	}

	if (!(creation_flags & CreationFlags::staging))
	{
		texture_srv_ = CreateTextureResourceView(texture_object_, format, 0, mipmap_count, device_srv_dim_type);
		texture_rt_ = CreateRenderTargetView(texture_object_, device_rtv_dim_type);
	}
	else
	{
		texture_srv_ = nullptr;
		texture_rt_ = nullptr;
	}

	if (creation_flags & (UINT)CreationFlags::structured_buffer)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc;
		ZeroMemory(&uav_desc, sizeof(uav_desc));
		uav_desc.Buffer.FirstElement = 0;

		// This is a Structured Buffer
		uav_desc.Format = format;      // Format must be must be DXGI_FORMAT_UNKNOWN, when creating a View of a Structured Buffer

		if (dimension_type_ == Dim_2D)
		{
			uav_desc.Texture2D.MipSlice = 0;
			uav_desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		}
		else
		{
			uav_desc.Texture3D.MipSlice = 0;
			uav_desc.Texture3D.FirstWSlice = 0;
			uav_desc.Texture3D.WSize = -1;
			uav_desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
		}

		bool res = g_device->CreateUnorderedAccessView(texture_object_, &uav_desc, &texture_uav_);
		ZEPHYR_ASSERT(SUCCEEDED(res));
	}

	format_ = format;
	dimension_ = dimension;
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

ID3D11Resource* Texture::get_texture_object() const
{
	return texture_object_;

}

ID3D11RenderTargetView* Texture::get_rt() const
{
	return texture_rt_;
}

void Texture::set_as_uav(int slot, shaderType type, unsigned int initial_count /*= -1*/)
{
	ZEPHYR_ASSERT(uav_ != nullptr);

	if (type == shaderType::pixel)
	{
		SetUAVToPixelShader(texture_uav_, slot, initial_count);
	}
	else if (type == shaderType::compute)
	{
		g_deviceContext->CSSetUnorderedAccessViews(slot, 1, &texture_uav_, &initial_count);
	}
	else
	{
		ZEPHYR_ASSERT(false);
	}
}

void Texture::get_data(void * data, int length)
{
	if (data_accesor_staging_texture_ == nullptr)
	{
		data_accesor_staging_texture_ = new Texture(dimension_, nullptr, format_, (UINT)CreationFlags::staging, 1);
	}

	CopySubResource(texture_object_, data_accesor_staging_texture_->get_texture_object(), dimension_, 0, 0);

	D3D11_MAPPED_SUBRESOURCE info;
	bool result = g_deviceContext->Map(data_accesor_staging_texture_->get_texture_object(), 0, D3D11_MAP_READ, 0, &info);
	ZEPHYR_ASSERT(result);

	memcpy(data, info.pData, length);

	g_deviceContext->Unmap(data_accesor_staging_texture_->get_texture_object(), 0);

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
		D3D11_RTV_DIMENSION device_rtv_dim_type = dimension_type_ == Dim_3D ? D3D11_RTV_DIMENSION_TEXTURE3D : D3D11_RTV_DIMENSION_TEXTURE2D;
		mipmap_rts_[mip_map] = CreateRenderTargetView(texture_object_, device_rtv_dim_type, mip_map, format_);
	}
}

void Texture::validate_mipmap_srv(int mip_map)
{
	if (mipmap_srvs_.find(mip_map) == mipmap_srvs_.end())
	{
		mipmap_srvs_[mip_map] = CreateTextureResourceView(texture_object_, format_ , mip_map, 1, D3D_SRV_DIMENSION_TEXTURE2D);
	}
}



