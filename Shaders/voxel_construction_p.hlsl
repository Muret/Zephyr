
#include "Definitions.hlsli"
#include "vct_helpers.hlsli"


float4 main(PixelInputType input) : SV_TARGET
{
	//float grid_position_normalized = (input.world_position.xyz - g_scene_min) * g_inverse_scene_length;
	//uint3 grid_index = grid_position_normalized * g_grid_resolution_xyz_iteration_count_w.xyz;

	float3 world_position = input.world_position;
	float3 albedo = input.color;
	float3 normal = input.world_normal;

	return handle_grid_entry(world_position, albedo, normal);
}