
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
	int is_leaf_node;

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
	float4 g_inverse_leaf_dimension;
};

RWStructuredBuffer<NodeSet> nodes_pool :  register (u4);
RWStructuredBuffer<NodeAllocationRequest> deferred_allocation_requests :  register (u5);
RWTexture3D<float4> leaf_nodes_albedo :  register (u7);

#ifdef COMPUTE_PASS
	RWStructuredBuffer<NodeAllocationRequest> deferred_allocation_requests_input :  register (u6);
#endif


#define GET_NODE(index) nodes_pool[index.x].nodes[index.y]
#define GET_LEAF_NODE(index) leaf_nodes_albedo[index]
#define ALLOCATE_LEAF_BRICK(index) InterlockedAdd(GET_NODE(uint2(0,1)).construction_mutex, 1, index);

int get_child_index(int3 index)
{
	return index.x + index.y * 2 + index.z * 4;
}


float4 handle_grid_entry(float3 world_position, float3 albedo, float3  normal)
{
	float3 cur_brick_mid_point = (g_scene_max.xyz + g_scene_min.xyz) * 0.5f;
	float3 cur_half_cell = (g_scene_max.xyz - g_scene_min.xyz) * 0.5f;

	uint2 current_brick_index = uint2(0, 0);
	uint3 last_child_index3 = uint3(0, 0, 0);

	uint brick_nodeset_index = 0;

	[loop]
	for (int i = 0; i < g_grid_resolution_xyz_iteration_count_w.w ; i++)
	{
		if (GET_NODE(current_brick_index).is_constructed == 0)		//needs to be constructed
		{
			int original_value;
			InterlockedAdd(GET_NODE(current_brick_index).construction_mutex, 1, original_value);
			if (original_value == 0)	//construct its children
			{
				if (i == (g_grid_resolution_xyz_iteration_count_w.w - 1))		//allocate a texture leaf node
				{
					uint leaf_brick_index;
					ALLOCATE_LEAF_BRICK(leaf_brick_index);
					GET_NODE(current_brick_index).child_node_set_index = leaf_brick_index;
					GET_NODE(current_brick_index).is_constructed = 1;
					GET_NODE(current_brick_index).is_leaf_node = 1;

					brick_nodeset_index = leaf_brick_index;
				}
				else  		//allocate an intermediate tree node
				{
					uint node_set_index = nodes_pool.IncrementCounter() + 1;
					GET_NODE(current_brick_index).child_node_set_index = node_set_index;
					GET_NODE(current_brick_index).is_constructed = 1;
				}
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
		last_child_index3.x = step(cur_brick_mid_point.x, world_position.x);
		last_child_index3.y = step(cur_brick_mid_point.y, world_position.y);
		last_child_index3.z = step(cur_brick_mid_point.z, world_position.z);

		int3 in_temp = last_child_index3 * 2 - 1;

		cur_half_cell *= 0.5;
		cur_brick_mid_point = cur_half_cell * in_temp + cur_brick_mid_point;

		int child_index = get_child_index(last_child_index3);
		current_brick_index.x = GET_NODE(current_brick_index).child_node_set_index;
		current_brick_index.y = child_index;
	}

	//set self properties to leaf node
	{
		uint base_z_coord = brick_nodeset_index * 3;
		uint3 first_coord = uint3(0, 0, 0 + base_z_coord);

		albedo = float3(1, 0, 1);

		uint3 u000 = first_coord + uint3(0, 0, 0);
		uint3 u001 = first_coord + uint3(0, 0, 1);
		uint3 u010 = first_coord + uint3(0, 1, 0);
		uint3 u011 = first_coord + uint3(0, 1, 1);
		uint3 u100 = first_coord + uint3(1, 0, 0);
		uint3 u101 = first_coord + uint3(1, 0, 1);
		uint3 u110 = first_coord + uint3(1, 1, 0);
		uint3 u111 = first_coord + uint3(1, 1, 1);

		GET_LEAF_NODE(u000) = float4(albedo, 1);
		GET_LEAF_NODE(u010) = float4(albedo, 1);
		//GET_LEAF_NODE(u010) = float4(albedo, 1);
		//GET_LEAF_NODE(u011) = float4(albedo, 1);
		//GET_LEAF_NODE(u100) = float4(albedo, 1);
		//GET_LEAF_NODE(u101) = float4(albedo, 1);
		//GET_LEAF_NODE(u110) = float4(albedo, 1);
		//GET_LEAF_NODE(u111) = float4(albedo, 1);
	}

	return float4(albedo, 1);

}