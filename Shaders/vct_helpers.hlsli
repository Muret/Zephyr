
#include "Definitions.hlsli"
#include "shared_functions.hlsli"

struct NodeAllocationRequest
{
	float3 position;
	float3 albedo;
	float3 normal;
	float3 ID;
};

struct Node
{
	int construction_mutex;
	int brick_index;

	int child_node_set_index;
	int is_constructed;
};


struct NodeSet
{
	Node nodes[8];
};

cbuffer VCTConstants : register (b3)
{
	float4 g_scene_max;
	float4 g_scene_min;
	float4 g_inverse_scene_length;
	float4 g_grid_resolution_xyz_iteration_count_w;
};

RWStructuredBuffer<NodeSet> nodes_pool :  register (u4);
RWStructuredBuffer<NodeAllocationRequest> deferred_allocation_requests :  register (u5);

#ifdef COMPUTE_PASS
	RWStructuredBuffer<NodeAllocationRequest> deferred_allocation_requests_input :  register (u6);
#endif

#define GET_NODE(index) nodes_pool[index.x].nodes[index.y]

int get_child_index(int3 index)
{
	return index.x + index.y * 2 + index.z * 4;
}


float4 handle_grid_entry(float3 world_position, float3 albedo, float3  normal)
{
	float3 cur_brick_mid_point = (g_scene_max.xyz + g_scene_min.xyz) * 0.5f;
	float3 cur_half_cell = (g_scene_max.xyz - g_scene_min.xyz) * 0.5f;

	uint2 current_brick_index = uint2(0, 0);

	[loop]
	for (int i = 0; i < g_grid_resolution_xyz_iteration_count_w.w; i++)
	{
		if (GET_NODE(current_brick_index).is_constructed == 0)		//needs to be constructed
		{
			int original_value;
			InterlockedAdd(GET_NODE(current_brick_index).construction_mutex, 1, original_value);
			if (original_value == 0)	//construct its children
			{
				uint node_set_index = nodes_pool.IncrementCounter() + 1;
				GET_NODE(current_brick_index).child_node_set_index = node_set_index;
				GET_NODE(current_brick_index).is_constructed = 1;
			}
			else //somebody else is constructing this node, register self for deferred execution
			{
				uint deferred_index = deferred_allocation_requests.IncrementCounter();
				deferred_allocation_requests[deferred_index].position = world_position;
				deferred_allocation_requests[deferred_index].albedo = albedo;
				deferred_allocation_requests[deferred_index].normal = normal;

				return float4(0,0,0,0);
			}
		}

		//find next node
		int3 index;
		index.x = step(world_position.x, cur_half_cell);
		index.y = step(world_position.y, cur_half_cell);
		index.z = step(world_position.z, cur_half_cell);

		int3 in_temp = index * 2 - 1;

		cur_half_cell *= 0.5;
		cur_brick_mid_point = cur_half_cell * in_temp + cur_brick_mid_point;

		int child_index = get_child_index(index);
		current_brick_index.x = GET_NODE(current_brick_index).child_node_set_index;
		current_brick_index.y = child_index;
	}

	return float4(albedo, 1);

}