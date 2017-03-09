


#include "Definitions.hlsli"

//struct Gbuffer_output
//{
//	float4 albedo			:	SV_Target0;
//	//float4 normal			:	SV_Target1;
//	//float4 specular_gloss	:	SV_Target2;
//};


float4 main(PixelInputType input) : SV_TARGET
{
	float2 normalized_ss_pos = input.tex_coord;
	normalized_ss_pos = normalized_ss_pos * 0.5 + 0.5;
	
	int2 tile_index = int2(g_current_tile_info.x, g_current_tile_info.y);
	float2 tile_start = tile_index * g_screen_tile_size.zw;
	float2 tile_end = tile_start + g_screen_tile_size.zw;

	if (normalized_ss_pos.x < tile_start.x || normalized_ss_pos.y < tile_start.y ||
		normalized_ss_pos.x > tile_end.x || normalized_ss_pos.y > tile_end.y)
	{
		clip(-1);
	}

	return float4(1,0,1, 1);

	//Gbuffer_output output;
	////output.albedo = float4(input.color.xyz, 1);
	//output.albedo = float4(1,0,1, 1); //g_diffuse_color.rgba * input.color * float4(diffuse_texture.Sample(LinearSampler, input.tex_coord.xy).rgb, 1);
	////output.albedo = /*g_diffuse_color.rgba * input.color **/ float4(diffuse_texture.Sample(LinearSampler, input.tex_coord.xy).rgb, 1);
	//
	////input.world_normal.xyz = normalize(input.world_normal.xyz);
	////output.normal = float4(1, 1, 1, 1); // float4(input.world_normal.xyz * 0.5 + float3(0.5, 0.5, 0.5), 0);
	////output.specular_gloss = float4(input.world_position);
	//return output;
}