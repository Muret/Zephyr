
#define COMPUTE_PASS
#include "vct_helpers.hlsli"

[numthreads(256, 1, 1)]
void main(uint3 dtid : SV_DispatchThreadID)
{
	uint request_id = dtid.x;
	uint total_count, stride;
	deferred_allocation_requests_input.GetDimensions(total_count, stride);
	if (total_count <= request_id)
	{
		return;
	}

	float3 albedo = deferred_allocation_requests_input[request_id].albedo;
	float3 normal = deferred_allocation_requests_input[request_id].normal;
	float3 position = deferred_allocation_requests_input[request_id].position;

	handle_grid_entry(position, albedo, normal);

}