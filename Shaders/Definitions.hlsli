#ifndef __INCLUDE_DEFINITIONS_HLSLI
#define __INCLUDE_DEFINITIONS_HLSLI

#define PI 3.14

/////////////
// GLOBALS //
/////////////
cbuffer FrameConstantsBuffer : register (b0)
{
	matrix g_view_matrix;
	matrix g_projection_matrix;
	matrix g_view_projection_matrix;

	matrix g_inv_view_matrix;
	matrix g_inv_projection_matrix;
	matrix g_inv_view_projection_matrix;

	float4 g_right_direction;
	float4 g_up_direction;
	float4 g_view_direction;
	float4 g_camera_position;

	float4 g_screen_texture_half_pixel_forced_mipmap;
	float4 g_near_far_padding2;
	float4 g_debug_vector;

	float4 g_screen_tile_size;
	float4 g_screen_tile_info[128];	
};

cbuffer MeshConstantsBuffer : register (b1)
{
	matrix g_world_matrix;
	matrix g_world_view_matrix;
	matrix g_world_view_projection_matrix;

	matrix g_inv_world_matrix;
	matrix g_inv_world_view_matrix;
	matrix g_inv_world_view_projection_matrix;

	float4 g_diffuse_color;
	float4 g_bb_min;
	float4 g_bb_max;
	float4 g_current_tile_info;
};

cbuffer LightingConstantsBuffer : register (b2)
{
	float4 g_ws_light_position;
	float4 g_ss_light_position;
	float4 g_light_color;
	matrix g_light_view_projection_matrix;
	matrix g_light_view_projection_matrix_inv;
};

//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
	float4 position		: POSITION;
	float4 color		: COLOR;
	float4 tex_coord	: TEXCOORD;
	float4 normal		: NORMAL0;
	float4 tangent		: TANGENT0;
};


struct PixelInputType
{
	float4 position		: SV_POSITION;

	float4 color		: COLOR;
	float4 tex_coord	: TEX_COORD0;
	float4 world_normal	: NORMAL0;
	float4 tangent		: TANGENT0;
	float4 world_position : TANGENT1;
	float4 ss_position  : TANGENT2;
};




//time for some textures

//samplers
SamplerState PointSampler : register(s0)
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Clamp;
	AddressV = Clamp;
};

SamplerState LinearSampler : register(s1)
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Repeat;
	AddressV = Repeat;
};

#endif