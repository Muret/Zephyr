
#include "Definitions.hlsli"

float4 main(PixelInputType input) : SV_TARGET
{
	float3 albedo_color = diffuse_texture.Sample(LinearSampler, input.tex_coord.xy).rgb;
	float3 normal = normalize(input.normal);

	float3 final_color = max(dot(-light_direction.xyz, normal), 0) * albedo_color * light_color;

	return float4(final_color,1);
}