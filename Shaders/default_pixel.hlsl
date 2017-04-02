
#include "Definitions.hlsli"

Texture2D diffuse_texture : register(t0);

float4 main(PixelInputType input) : SV_TARGET
{
	float3 albedo_color = diffuse_texture.Sample(PointSampler, input.tex_coord.xy).rgb;
	float3 normal = normalize(input.world_normal);

	//TODO_MURAT cleanup
	float nDotL = max(dot(-normalize(g_ws_light_position - input.world_position), normalize(normal)), 0);
	float3 final_color = nDotL * albedo_color;

	//return float4(nDotL.xxx, 1);
	return float4(normal.xyz * 0.4 + 0.6, 1);
	//return float4(final_color.rgb,1);
}