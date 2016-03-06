
#ifndef USE_TEXTURE_H_
#define USE_TEXTURE_H_

#include "d11.h"

class Texture
{

public:

	enum DimensionType
	{
		Dim_2D = 0,
		Dim_3D = 1,
	};

	Texture();
	Texture(const D3DXVECTOR3 &dimension , void *data, DXGI_FORMAT format, UINT creation_flags, int mipmap_count = 1);

	void create(const D3DXVECTOR3 &dimension, void *data, DXGI_FORMAT format, UINT creation_flags, int mipmap_count = 1);
	void set_as_render_target( int slot, int mip_map = 0);
	void set_srv_to_shader(shaderType type, int slot, int mip_map_to_set = 0);

	ID3D11ShaderResourceView* get_srv(int mip_map = 0);
	ID3D11Resource* get_texture_object() const;
	ID3D11RenderTargetView* get_rt() const;

	void set_as_uav(int slot, shaderType type,unsigned int initial_count = -1);
	
	void get_data(void *data, int length);

private:
	void validate_mipmap_rt(int mip_map);
	void validate_mipmap_srv(int mip_map);

	ID3D11Resource *texture_object_;
	ID3D11ShaderResourceView *texture_srv_;
	ID3D11RenderTargetView *texture_rt_;
	ID3D11UnorderedAccessView *texture_uav_;

	Texture *data_accesor_staging_texture_;

	std::map<int, ID3D11RenderTargetView *> mipmap_rts_;
	std::map<int, ID3D11ShaderResourceView *> mipmap_srvs_;

	D3DXVECTOR3 dimension_;
	DXGI_FORMAT format_;

	DimensionType dimension_type_;
};

#endif