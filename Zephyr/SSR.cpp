
#include "SSR.h"
#include "Shader.h"
#include "Texture.h"

#include <assert.h>

SSR::SSR()
{
	depth_root_copy_shader = new Shader("direct_vertex_position" , "texture_output_p");
	hi_z_depth_gen_shader = new Shader("direct_vertex_position", "depth_mipmap_gen");
	ssr_post_fx_pass = new Shader("direct_vertex_position", "ssr_full_screen_pass");

	assert(g_screenWidth == g_screenHeight);
	mipmap_count_ = log2(g_screenWidth);

	hi_z_depth_texture_ping_pong[0] = new Texture( D3DXVECTOR3(g_screenWidth, g_screenHeight,1), nullptr, DXGI_FORMAT_R32G32B32A32_FLOAT, mipmap_count_ + 1);
	hi_z_depth_texture_ping_pong[1] = new Texture( D3DXVECTOR3(g_screenWidth, g_screenHeight,1), nullptr, DXGI_FORMAT_R32G32B32A32_FLOAT, mipmap_count_ + 1);
	
	hi_z_depth_texture = hi_z_depth_texture_ping_pong[0];
}

void SSR::pre_render()
{

}

void SSR::post_render()
{
	//print_hi_z_depth_texture();

	SetDepthStencilView(nullptr);
	ssr_post_fx_pass->set_shaders();
	hi_z_depth_texture->set_srv_to_shader(shaderType::pixel, 4);

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

	int render_count = mipmap_count_;

	Texture *currentOutput = hi_z_depth_texture_ping_pong[0];
	Texture *currentInput = hi_z_depth_texture_ping_pong[1];

	invalidate_srv(shaderType::pixel);

	int res = g_screenWidth;
	for (int i = 0; i < render_count; i++)
	{
		if (i == 0)
		{
			ID3D11ShaderResourceView *view = GetDepthTextureSRV();
			SetSRV(&view, 1, shaderType::pixel, 0);
			currentOutput->set_as_render_target(0);
			depth_root_copy_shader->set_shaders();
		}
		else
		{
			ID3D11ShaderResourceView *null_srv = nullptr;
			SetSRV(&null_srv, 1, shaderType::pixel, 0);

			currentOutput->set_as_render_target(0, i);
			currentInput->set_srv_to_shader(shaderType::pixel, 0, i - 1);

			hi_z_depth_gen_shader->set_shaders();
		}
		SetViewPort(0 , 0, res, res);
		RenderFullScreenQuad();
		res = res >> 1;

		Texture *temp = currentOutput;
		currentOutput = currentInput;
		currentInput = temp;
	}

	currentOutput = hi_z_depth_texture_ping_pong[0];
	currentInput = hi_z_depth_texture_ping_pong[1];

	res = g_screenWidth >> 1;
	for (int i = 1; i < render_count; i += 2)
	{
		currentOutput->set_as_render_target(0, i);
		currentInput->set_srv_to_shader(shaderType::pixel, 0, i);

		depth_root_copy_shader->set_shaders();

		SetViewPort(0, 0, res, res);
		RenderFullScreenQuad();
		res = res >> 2;
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
