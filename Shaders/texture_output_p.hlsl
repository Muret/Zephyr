
#include "Definitions.hlsli"


float4 main(PixelInputType input) : SV_TARGET
{
	float4 res = 0;
	if (screen_texture_half_pixel_forced_mipmap.z != -1)
	{
		res = float4(diffuse_texture.SampleLevel(SampleType, input.tex_coord.xy, screen_texture_half_pixel_forced_mipmap.z).rgb, 1);
	}
	else
	{
		res = float4(diffuse_texture.Sample(SampleType, input.tex_coord.xy).rgb, 1);
	}

	return res;

}