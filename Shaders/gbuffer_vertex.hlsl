

#include "Definitions.hlsli"

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////

float3 rot(float3 pos, float angle) 
{
	float s = sin(angle);// * g_debug_vector.x;
	float c = cos(angle);// * g_debug_vector.y;
	return float3( c*pos.x - s*pos.z, pos.y , s*pos.x + c*pos.z);
}

void calculate_tiled_screen_space_position(inout float4 ss_position, out float2 actual_ss_pos)
{
	ss_position /= ss_position.w;
	
	actual_ss_pos = ss_position;

	float2 tex_coord = ss_position.xy * 0.5 + float2(0.5, 0.5);
	
	int2 tile_index = int2(g_current_tile_info.x, g_current_tile_info.y);
	float2 tile_start = tile_index * g_screen_tile_size.zw;

	float2 tile_relative_pos = tex_coord - tile_start;
	float2 normalized_tile_coord = tile_relative_pos * g_screen_tile_size.xy;

	int current_tile_index = tile_index.x + tile_index.y * g_screen_tile_size.x;
	float2 atlas_coord_start = g_screen_tile_info[current_tile_index].xy;
	float2 real_atlas_tile_size = g_screen_tile_info[current_tile_index].zw;

	float2 normalized_texture_coord_for_atlas = atlas_coord_start + real_atlas_tile_size * normalized_tile_coord;
	normalized_texture_coord_for_atlas = normalized_texture_coord_for_atlas * 2.0 - float2(1.0f, 1.0f);

	ss_position.xy = normalized_texture_coord_for_atlas;
}

PixelInputType main(VertexInputType input)
{
	PixelInputType output; 

	float4 position = float4(input.position.xyz, 1);
	output.position = mul(position, g_world_view_projection_matrix);

	//float2 actual_ss_pos = 0;  

	//calculate_tiled_screen_space_position(output.position, actual_ss_pos);

	output.color = input.color;
	output.tex_coord = float4(input.tex_coord.xy, 0, 0);
	output.tangent = input.tangent;
	output.world_normal = mul(float4(input.normal.xyz,0), g_world_matrix);
	output.ss_position = output.position;

	return output;
}


