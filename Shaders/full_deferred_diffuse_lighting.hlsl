
#include "Definitions.hlsli"


float4 main(PixelInputType input) : SV_TARGET
{
	float3 albedo_color = diffuse_texture.Sample(LinearSampler, input.tex_coord.xy).rgb;
	float3 normal = normalize((normal_texture.Sample(LinearSampler, input.tex_coord.xy).rgb * 2.0f - 1.0f).rgb);

	//TODO_MURAT cleanup
	float3 final_color = max(dot(-ws_light_position.xyz, normal), 0) * albedo_color * light_color;

	//return float4(normal * 0.5 + 0.5, 1);
	return float4(albedo_color, 1);

}