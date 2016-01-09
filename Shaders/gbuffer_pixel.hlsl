


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
	output.albedo = diffuse_color.rgba; float4(diffuse_texture.Sample(LinearSampler, input.tex_coord.xy).rgb, 1);

	float3 world_normal = mul(float4(input.normal.xyz,0), WorldMatrix).xyz;
	output.normal = float4(world_normal * 0.5 + 0.5, 1);
	output.specular_gloss = 0;
	return output;
}