
#include "SSR.h"
#include "Shader.h"
#include "Texture.h"

#include <assert.h>

SSR::SSR()
{
	depth_root_copy_shader = new Shader("default_vertex" , "texture_output_p");
	hi_z_depth_gen_shader = new Shader("default_vertex", "depth_mipmap_gen");
	ssr_post_fx_pass = new Shader("default_vertex", "ssr_full_screen_pass");

	assert(g_screenWidth == g_screenHeight);
	mipmap_count_ = log2(g_screenWidth);

	hi_z_depth_texture = new Texture(g_screenWidth, g_screenHeight, nullptr, DXGI_FORMAT_R32G32B32A32_FLOAT, mipmap_count_ + 1);
}

void SSR::pre_render()
{

}

void SSR::post_render()
{
	//print_hi_z_depth_texture();

	return;

	SetDepthStencilView(nullptr);
	ssr_post_fx_pass->set_shaders();
	hi_z_depth_texture->set_srv_to_shader(shader_type_pixel, 4);

	SetViewPortToDefault();
	RenderFullScreenQuad();
}

void SSR::post_gbuffer_render()
{
	generate_depth_mipmap_textures();
}

void SSR::generate_depth_mipmap_textures()
{
	SetRenderViews(nullptr, nullptr, 0);

	D3DXMATRIX matrix;
	D3DXMatrixIdentity(&matrix);

	render_constantsBuffer_cpu.WorldViewProjectionMatrix = matrix;
	render_constantsBuffer_cpu.screen_texture_half_pixel.z = -1;

	UpdateGlobalBuffers();

	int render_count = mipmap_count_;
	int res = g_screenWidth;
	for (int i = 0; i < render_count; i++)
	{
		if (i == 0)
		{
			ID3D11ShaderResourceView *view = GetDepthTextureSRV();
			SetSRV(&view, 1, shaderType::shader_type_pixel, 0);
			hi_z_depth_texture->set_as_render_target(0);
			depth_root_copy_shader->set_shaders();
		}
		else
		{
			hi_z_depth_texture->set_as_render_target(0,i);
			hi_z_depth_texture->set_srv_to_shader(shader_type_pixel, 0, i - 1);
			hi_z_depth_gen_shader->set_shaders();
		}

		SetViewPort(res, res);
		RenderFullScreenQuad();
		res = res >> 1;
	}
}

bool SSR::uses_postfx() const
{
	return true;
}

void SSR::print_hi_z_depth_texture()
{
	int render_count = mipmap_count_;
	D3DXVECTOR4 pos = D3DXVECTOR4(-0.9, 0.9, 0, 0);
	D3DXVECTOR4 scale = D3DXVECTOR4(0.1, 0.1, 0, 0);
	for (int i = 0; i < render_count; i++)
	{
		OutputTextureToScreen(hi_z_depth_texture->get_srv(i), pos, scale);

		pos.x += 0.25;
		if (pos.x > 0.0)
		{
			pos.x = -0.9f;
			pos.y -= 0.25f;
		}
	}
}
