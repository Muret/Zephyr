#ifndef __INCLUDE_DEFINITIONS_HLSLI
#define __INCLUDE_DEFINITIONS_HLSLI

/////////////
// GLOBALS //
/////////////
cbuffer MatrixBuffer : register (b0)
{
	matrix WorldViewProjectionMatrix;
	matrix WorldMatrix;
	matrix inverseWorldViewProjectionMatrix;
	matrix inverseProjectionMatrix;
	matrix projectionMatrix;

	float4 right_direction;
	float4 up_direction;
	float4 view_direction;
	float4 camera_position;

	float4 screen_texture_half_pixel_forced_mipmap;
	float4 near_far_padding2;
};

cbuffer LightingInfoBuffer : register (b1)
{
	float4 light_direction;
	float4 light_color;
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
	float4 normal		: NORMAL0;
	float4 tangent		: TANGENT0;
};




//time for some textures

Texture2D diffuse_texture : register(t0);
Texture2D normal_texture : register(t1);
Texture2D specular_texture : register(t2);
Texture2D screen_texture : register(t3);

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
	AddressU = Clamp;
	AddressV = Clamp;
};

#endif