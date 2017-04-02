


#include "Definitions.hlsli"

struct Gbuffer_output
{
	float4 albedo			:	SV_Target0;
	float4 normal			:	SV_Target1;
	float4 specular_gloss	:	SV_Target2;
};

Texture2D diffuse_texture : register(t0);

Gbuffer_output main(PixelInputType input) : SV_TARGET
{
	Gbuffer_output output;
	//output.albedo = float4(input.color.xyz, 1);
	output.albedo = /*g_diffuse_color.rgba * input.color **/ float4(diffuse_texture.Sample(LinearSampler, input.tex_coord.xy).rgb, 1);
	//output.albedo = /*g_diffuse_color.rgba * input.color **/ float4(diffuse_texture.Sample(LinearSampler, input.tex_coord.xy).rgb, 1);

	//output.albedo = float4(input.tex_coord.xy,0,1);

	float view_space_depth = input.ss_position.z / input.ss_position.w;

	input.world_normal.xyz = normalize(input.world_normal.xyz);
	output.normal = float4(input.world_normal.xyz * 0.5 + float3(0.5, 0.5, 0.5), view_space_depth);
	output.specular_gloss = float4(0,0,0,0);

	return output;
}