
#ifndef USE_TEXTURE_H_
#define USE_TEXTURE_H_

#include "d11.h"

class Texture
{

public:

	Texture();
	Texture(int w, int h, void *data, DXGI_FORMAT format, int mipmap_count = 1);

	void create(int w, int h, void *data, DXGI_FORMAT format, int mipmap_count = 1);
	void set_as_render_target( int slot, int mip_map = 0);
	void set_srv_to_shader(shaderType type, int slot, int mip_map_to_set = 0);

	ID3D11ShaderResourceView* get_srv(int mip_map = 0);
	ID3D11Texture2D* get_texture_object() const;
	ID3D11RenderTargetView* get_rt() const;
	
private:
	void validate_mipmap_rt(int mip_map);
	void validate_mipmap_srv(int mip_map);

	ID3D11Texture2D *texture_object_;
	ID3D11ShaderResourceView *texture_srv_;
	ID3D11RenderTargetView *texture_rt_;

	std::map<int, ID3D11RenderTargetView *> mipmap_rts_;
	std::map<int, ID3D11ShaderResourceView *> mipmap_srvs_;

	int width_, height_;
	DXGI_FORMAT format_;


};

#endif