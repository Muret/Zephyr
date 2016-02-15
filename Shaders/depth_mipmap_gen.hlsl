
#include "Definitions.hlsli"


float4 main(PixelInputType input) : SV_TARGET
{
	float2 pixel_size = g_screen_texture_half_pixel_forced_mipmap.xy;

	float d1 = diffuse_texture.SampleLevel(PointSampler, input.tex_coord.xy + pixel_size * float2(0,0)		, -1);
	float d2 = diffuse_texture.SampleLevel(PointSampler, input.tex_coord.xy + pixel_size * float2(1,1)		, -1);
	float d3 = diffuse_texture.SampleLevel(PointSampler, input.tex_coord.xy + pixel_size * float2(1,-1)		, -1);
	float d4 = diffuse_texture.SampleLevel(PointSampler, input.tex_coord.xy + pixel_size * float2(-1,1)		, -1);
	float d5 = diffuse_texture.SampleLevel(PointSampler, input.tex_coord.xy + pixel_size * float2(-1,-1)	, -1);

	float min_depth = min( min(min(d1, d2), min(d3, d4)) , d5);
	float max_depth = max( max(max(d1, d2), max(d3, d4)) , d5);

	return float4(min_depth, max_depth, 0, 0);
}