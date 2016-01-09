
#include "Definitions.hlsli"
#include "shared_functions.hlsli"

Texture2D hi_z_depth_texture : register(t4);

#define cb_mipCount 11
#define texelWidth (1.0f / 1024.0f)
#define texelHeight (1.0f / 1024.0f)

static const float HIZ_START_LEVEL = 0.0f;
static const float HIZ_STOP_LEVEL = 0.0f;
static const float HIZ_MAX_LEVEL = float(cb_mipCount);
static const float2 HIZ_CROSS_EPSILON = float2(texelWidth, texelHeight); // maybe need to be smaller or larger? this is mip level 0 texel size
static const uint MAX_ITERATIONS = 64u;

void get_ss_hit_pos_ray_dir(in float2 tex_coord, out float3 screen_space_position, out float3 screen_space_ray_dir, out float3 world_space_position)
{
	//calculate the view space normal and hit position
	//then calculate the view space reflection vector
	//convert all those to screen space

	float4 cameraRay = float4(tex_coord * 2.0 - 1.0, 1.0, 1.0);
	cameraRay.y *= -1;
	cameraRay = mul(cameraRay, inverseProjectionMatrix);
	cameraRay = cameraRay / cameraRay.w;
	cameraRay.w = 0;
	cameraRay = normalize(cameraRay);

	float hit_hw_depth = hi_z_depth_texture.SampleLevel(PointSampler, tex_coord, 0);
	float hit_linear_depth = hw_depth_to_linear_depth(hit_hw_depth);

	if (hit_hw_depth == 1.0f)
	{
		clip(-1);
	}

	float3 view_space_position = cameraRay * hit_linear_depth;
	world_space_position = mul(float4(view_space_position, 1), inverseViewMatrix).xyz;

	float4 screen_space_position_temp = mul(float4(view_space_position, 1), projectionMatrix);
	screen_space_position_temp /= screen_space_position_temp.w;
	screen_space_position = screen_space_position_temp.xyz;;
	screen_space_position.xy = screen_space_position.xy * 0.5 + 0.5;

	float3 world_space_normal = normal_texture.SampleLevel(PointSampler, tex_coord, 0);
	world_space_normal = normalize(world_space_normal * 2.0f - 1.0f);
	float3 view_space_normal = normalize(mul(float4(world_space_normal, 0), viewMatrix));

	float3 view_space_view_dir = normalize(view_space_position);
	float3 view_space_reflected_ray_dir = normalize(reflect(view_space_view_dir, view_space_normal));

	float4 screen_space_reflected_position = mul(float4(view_space_position + view_space_reflected_ray_dir, 1), projectionMatrix);
	screen_space_reflected_position = screen_space_reflected_position / screen_space_reflected_position.w;
	screen_space_reflected_position.xy = screen_space_reflected_position.xy * 0.5 + 0.5;
	screen_space_ray_dir = screen_space_reflected_position.xyz - screen_space_position.xyz;
}

float3 intersectDepthPlane(float3 o, float3 d, float t)
{
	return o + d * t;
}

float2 getCell(float2 ray, float2 cellCount)
{
	// does this need to be floor, or does it need fractional part - i think cells are meant to be whole pixel values (integer values) but not sure
	return floor(ray * cellCount);
}

static const float2 hiZSize = 1024; // not sure if correct - this is mip level 0 size

float2 getCellCount(float level, float rootLevel)
{
	// not sure why we need rootLevel for this
	float2 div = exp2(level);
	return hiZSize / div;
}

float3 intersectCellBoundary(float3 o, float3 d, float2 cellIndex, float2 cellCount, float2 crossStep, float2 crossOffset)
{
	float2 index = cellIndex + crossStep;
		index /= cellCount;
	index += crossOffset;
	float2 delta = index - o.xy;
		delta /= d.xy;
	float t = min(delta.x, delta.y);
	return intersectDepthPlane(o, d, t);
}

float getMinimumDepthPlane(float2 ray, float level, float rootLevel)
{
	return hi_z_depth_texture.SampleLevel(PointSampler, float2(ray.x, 1.0f - ray.y), level).r;
}

bool crossedCellBoundary(float2 cellIdxOne, float2 cellIdxTwo)
{
	return cellIdxOne.x != cellIdxTwo.x || cellIdxOne.y != cellIdxTwo.y;
}



float3 do_hiz_ss_ray_trace(float3 p, float3 v)
{
	const float rootLevel = float(cb_mipCount) - 1.0f; // convert to 0-based indexing

	float level = HIZ_START_LEVEL;

	uint iterations = 0u;

	// get the cell cross direction and a small offset to enter the next cell when doing cell crossing
	float2 crossStep = float2(v.x >= 0.0f ? 1.0f : -1.0f, v.y >= 0.0f ? 1.0f : -1.0f);
	float2 crossOffset = float2(crossStep.xy * HIZ_CROSS_EPSILON.xy);
	crossStep.xy = saturate(crossStep.xy);

	// set current ray to original screen coordinate and depth
	float3 ray = p.xyz;

	// scale vector such that z is 1.0f (maximum depth)
	float3 d = v.xyz / v.z;

	// set starting point to the point where z equals 0.0f (minimum depth)
	float3 o = intersectDepthPlane(p, d, -p.z);

	// cross to next cell to avoid immediate self-intersection
	const float2 cellCount = getCellCount(level, rootLevel);
	float2 rayCell = getCell(ray.xy, cellCount);
	ray = intersectCellBoundary(o, d, rayCell.xy, hiZSize.xy, crossStep.xy, crossOffset.xy);

	while (level >= HIZ_STOP_LEVEL && iterations < MAX_ITERATIONS)
	{
		// get the minimum depth plane in which the current ray resides
		float minZ = getMinimumDepthPlane(ray.xy, level, rootLevel);

		// get the cell number of the current ray
		const float2 cellCount = getCellCount(level, rootLevel);
		const float2 oldCellIdx = getCell(ray.xy, cellCount);

		// intersect only if ray depth is below the minimum depth plane
		float3 tmpRay = intersectDepthPlane(o, d, max(ray.z, minZ));

		// get the new cell number as well
		const float2 newCellIdx = getCell(tmpRay.xy, cellCount);

		// if the new cell number is different from the old cell number, a cell was crossed
		if (crossedCellBoundary(oldCellIdx, newCellIdx))
		{
			// intersect the boundary of that cell instead, and go up a level for taking a larger step next iteration
			tmpRay = intersectCellBoundary(o, d, oldCellIdx, cellCount.xy, crossStep.xy, crossOffset.xy); //// NOTE added .xy to o and d arguments
			level = min(HIZ_MAX_LEVEL, level + 2.0f);
		}

		ray.xyz = tmpRay.xyz;

		// go down a level in the hi-z buffer
		--level;

		++iterations;
	}

	return ray;
}

//input : screen space positions with hw depth
bool check_visibility_ss(float3 pos1, float3 pos2)
{
	float3 vec = pos2 - pos1;
	float3 ss_ray_hit = do_hiz_ss_ray_trace(pos1, vec);
	float normalized_t = (ss_ray_hit - pos1) / vec;

	return normalized_t > 1.0f;
}