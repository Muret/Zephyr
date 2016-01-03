
#include "Definitions.hlsli"


float4 main(PixelInputType input) : SV_TARGET
{
	float4 res = 0;
	if (screen_texture_half_pixel_forced_mipmap.z != -1)
	{
		res = float4(diffuse_texture.SampleLevel(LinearSampler, input.tex_coord.xy, screen_texture_half_pixel_forced_mipmap.z).rrr, 1);
	}
	else
	{
		res = float4(diffuse_texture.SampleLevel(PointSampler, input.tex_coord.xy, 0).rgb, 1);
	}

	return res;

}