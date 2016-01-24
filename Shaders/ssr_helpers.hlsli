
#include "Definitions.hlsli"
#include "shared_functions.hlsli"

Texture2D hi_z_depth_texture : register(t4);

#define cb_mipCount 11
#define texelWidth (1.0f / 1024.0f)
#define texelHeight (1.0f / 1024.0f)

static const float HIZ_START_LEVEL = 0.0f;
static const float HIZ_STOP_LEVEL = 0.0f;
static const float HIZ_MAX_LEVEL = float(cb_mipCount);
static const float2 HIZ_CROSS_EPSILON = float2(texelWidth * 0.5, texelHeight * 0.5); // maybe need to be smaller or larger? this is mip level 0 texel size
static const uint MAX_ITERATIONS = 64u;

float3 path_trace_imp(float3 position, inout Randomer randomer);

float3 get_ws_normal(float2 uv)
{
	float3 ws_normal = normal_texture.Sample(PointSampler, uv);
	ws_normal *= 2.0f - 1.0f;

	return ws_normal;
}

float3 get_vs_normal(float2 uv)
{
	float3 ws_normal = normal_texture.Sample(PointSampler, uv);
	ws_normal *= 2.0f - 1.0f;

	return normalize(mul(float4(ws_normal, 0), viewMatrix));
}

float3 get_ws_position_from_uv(float2 tex_coord, float hw_depth)
{
	float4 cameraRay = float4(tex_coord * 2.0 - 1.0, 1.0, 1.0);
	cameraRay.y *= -1;
	cameraRay = mul(cameraRay, inverseProjectionMatrix);
	cameraRay = cameraRay / cameraRay.w;
	cameraRay.w = 0;
	cameraRay = normalize(cameraRay);

	float hit_linear_depth = hw_depth_to_linear_depth(hw_depth);
	float3 view_space_position = cameraRay * hit_linear_depth;
	return mul(float4(view_space_position, 1), inverseViewMatrix).xyz;
}

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

static const float2 hiZSize = 1024 / exp2(HIZ_START_LEVEL); // not sure if correct - this is mip level 0 size

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


float between(float value, float min, float max)
{
	return min <= value && value < max;
}

float3 do_hiz_ss_ray_trace(float3 p, float3 v)
{
	const float rootLevel = float(cb_mipCount) - 1.0f; // convert to 0-based indexing

	float level = HIZ_START_LEVEL;

	uint iterations = 0u;

	// get the cell cross direction and a small offset to enter the next cell when doing cell crossing
	float2 crossStep = sign(v.xy); //float2(v.x >= 0.0f ? 1.0f : -1.0f, v.y >= 0.0f ? 1.0f : -1.0f);
	float2 crossOffset = float2(crossStep.xy * HIZ_CROSS_EPSILON.xy);
	//crossStep.xy = saturate(crossStep.xy);

	// set current ray to original screen coordinate and depth
	float3 ray = p.xyz;

	// scale vector such that z is 1.0f (maximum depth)
	float3 d = v.xyz / v.z;

	// set starting point to the point where z equals 0.0f (minimum depth)
	float3 o = intersectDepthPlane(p, d, -p.z);

	float2 range_min_max = float2(0, 1);
	//if (trace_until)
	//{
	//	range_min_max.y = (trace_until_position.x - o.x) / d.x;
	//}

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
			ray.xyz = tmpRay.xyz;

			crossOffset *= 2.0f;
		}
		else
		{
			crossOffset /= 2.0f;
		}

		//float cur_t = (ray.x - o.x) / d.x;
		//if (!between(cur_t, range_min_max.x, range_min_max.y))
		//{
		//	return ray.xyz;
		//}

		// go down a level in the hi-z buffer
		--level;

		++iterations;
	}

	return ray;
}

//input : screen space positions with hw depth
bool check_visibility_ss(float3 pos1, float3 pos2)
{
	float3 vec = (pos2 - pos1);
	float3 ss_ray_hit = do_hiz_ss_ray_trace(pos1, normalize(vec));

	//vec = normalize(vec);

	float normalized_t = (ss_ray_hit.x - pos1.x) / vec.x;
	if (abs(vec.x) < abs(vec.y))
	{
		normalized_t = (ss_ray_hit.y - pos1.y) / vec.y;
	}

	return normalized_t > 0.99f;
}

float3 get_random_ws_dir_wrt_normal(float3 normal, inout Randomer randomer)
{
	float u1 = randomer.GetCurrentFloat();
	float u2 = randomer.GetCurrentFloat();

	float r = sqrt(u1);
	float theta = 2 * PI * u2;

	float x = r * cos(theta);
	float y = r * sin(theta);

	float3 ray = float3(x, y, sqrt(max(0.0f, 1 - u1)));

	if (dot(ray, normal) < 0.0f)
	{
		ray *= -1;
	}

	return ray;
}

bool ss_position_inside_cube(float3 ss)
{
	return ss.x >= 0.0f && ss.x <= 1.0f &&
		ss.y >= 0.0f && ss.y <= 1.0f &&
		ss.z >= 0.0f && ss.z <= 1.0f;
}

float3 do_simple_path_tracing(float3 ss_position)
{
	Randomer randomer;
	randomer.SetSeed(ss_position.x * 5.164 + ss_position.y * 5.784 - ss_position.z * 7.12254);

	static const unsigned int number_of_samples = 16;
	static const float divident = 1.0f / (float)number_of_samples;

	float3 total_color = 0;
	for (int i = 0; i < number_of_samples; i++)
	{
		total_color += path_trace_imp(ss_position, randomer);
	}

	return total_color * divident;
}

//tracing of one path with importance sampling
//position : screen space xy with hw depth
float3 path_trace_imp(float3 position, inout Randomer randomer)
{
	float3 cumulative_color = 0.0f;
	float3 cumulative_color_factor = float3(1.0f, 1.0f, 1.0f);
	const unsigned int max_iterations = 6;
	unsigned int iteration_count = 0;
	
	static const float mc_termination_pos = 0.3f;
	static const float mc_termination_factor = 1.0f / mc_termination_pos;

	float3 cur_ss_position = position;
	float2 cur_uv = float2(cur_ss_position.x, 1.0f - cur_ss_position.y);
	float3 cur_albedo_color = diffuse_texture.Sample(PointSampler, cur_uv);
	float3 cur_ws_normal = get_ws_normal(cur_uv);
	float3 cur_ws_ray_dir = get_random_ws_dir_wrt_normal(cur_ws_normal, randomer);
	float3 cur_ws_position = get_ws_position_from_uv(cur_uv, cur_ss_position.z);

	cumulative_color_factor *= cur_albedo_color;

	[loop]
	while (iteration_count < max_iterations)
	{
		//importance sampling to light source
		if (check_visibility_ss(cur_ss_position, ss_light_position))
		{
			float NdotL = saturate(dot(normalize(ws_light_position - cur_ws_position), normalize(cur_ws_normal)));
			cumulative_color += NdotL * cumulative_color_factor * light_color.rgb;
		}

		float mc_random_value = randomer.GetCurrentFloat();
		if (mc_random_value < mc_termination_pos)
		{
			cumulative_color_factor = 0;
		}

		float4 ss_ray_target_point = mul( float4(cur_ws_ray_dir + cur_ws_position, 1), viewProjection);
		ss_ray_target_point /= ss_ray_target_point.w;
		float3 ss_new_ray_dir = ss_ray_target_point.xyz - cur_ss_position.xyz;

		float3 ss_hit_position = do_hiz_ss_ray_trace(cur_ss_position, ss_new_ray_dir);
		if (!ss_position_inside_cube(ss_hit_position))
		{
			cumulative_color_factor = 0;
		}

		cumulative_color_factor *= saturate(dot(normalize(cur_ws_normal), normalize(cur_ws_ray_dir))) * mc_termination_factor * cur_albedo_color;

		cur_ss_position = ss_hit_position;
		cur_uv = float2(cur_ss_position.x, 1.0f - cur_ss_position.y);
		cur_albedo_color = diffuse_texture.Sample(LinearSampler, cur_uv);
		cur_ws_normal = get_ws_normal(cur_uv);
		cur_ws_ray_dir = get_random_ws_dir_wrt_normal(cur_ws_normal, randomer);
		cur_ws_position = get_ws_position_from_uv(cur_uv, cur_ss_position.z);

		iteration_count++;
	}


	return cumulative_color;
}