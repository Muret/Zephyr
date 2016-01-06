#ifndef __INCLUDE_SSR_
#define __INCLUDE_SSR_

#include "RenderComponent.h"
#include <vector>

class Shader;
class Texture;

// cone traced ssr
class SSR : public RenderComponent
{
public:

	SSR();

	virtual void pre_render();
	virtual void post_render();
	virtual void post_gbuffer_render();

	virtual bool uses_postfx() const;
private:

	void generate_depth_mipmap_textures();
	void print_hi_z_depth_texture();

	Shader *depth_root_copy_shader;
	Shader *hi_z_depth_gen_shader;
	Shader *ssr_post_fx_pass;

	Texture *hi_z_depth_texture;

	Texture *hi_z_depth_texture_ping_pong[2];	//used only at creation of mip chain

	int mipmap_count_;
};


#endif