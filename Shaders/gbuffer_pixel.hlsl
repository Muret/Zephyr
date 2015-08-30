


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
	output.albedo = float4(diffuse_texture.Sample(SampleType, input.tex_coord.xy).rgb,1);
	output.normal = float4(input.normal.xyz * 0.5 + 0.5, 1);
	output.specular_gloss = 0;
	return output;
}