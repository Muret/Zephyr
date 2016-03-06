


#include "Definitions.hlsli"

struct Gbuffer_output
{
	float4 albedo			:	SV_Target0;
	float4 normal			:	SV_Target1;
	float4 specular_gloss	:	SV_Target2;
};


Gbuffer_output main(PixelInputType input) : SV_TARGET
{
	Gbuffer_output output;
	output.albedo = g_diffuse_color.rgba * input.color; float4(diffuse_texture.Sample(LinearSampler, input.tex_coord.xy).rgb, 1);

	input.world_normal.xyz = normalize(input.world_normal.xyz);
	output.normal = float4(input.world_normal.xyz * 0.5 + float3(0.5,0.5,0.5), 0);
	output.specular_gloss = float4(input.world_position);
	return output;
}