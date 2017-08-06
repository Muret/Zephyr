
#define MAX_RES 0.0062

#define USE_TILED_LIGHT_SHADOW_RENDERING

struct OUTPUT
{
	float4 color : SV_TARGET0;
	float4 light_amount : SV_TARGET1;
};

RWStructuredBuffer<int2> tile_render_data : register(u4);
Texture2D last_frame_sun_amount_texture : register(t6);

Texture2D light_shadow_depth_texture : register(t5);

struct DeferredDebuggingParams
{
	float cur_atlas_size;
	int2 cur_shadow_tile_index;
	float2 atlas_uv;
	float shadow_depth;
	float2 light_texture_uv;
};

float get_normal_from_atlas_texture(float2 t_cord, out int current_tile_index, inout DeferredDebuggingParams debugging_params, bool prev_frame)
{
	float2 tile_index = 0;
	tile_index.x = int(t_cord.x * g_screen_tile_size.x);
	tile_index.y = int(t_cord.y * g_screen_tile_size.y);

	tile_index.x = min(tile_index.x, g_screen_tile_size.x - 1);
	tile_index.y = min(tile_index.y, g_screen_tile_size.y - 1);

	//tile_index.y = 16 - tile_index.y;

	debugging_params.cur_shadow_tile_index = tile_index;

	//t_cord.y = 1.0f - t_cord.y;

	float2 tile_start = tile_index * g_screen_tile_size.zw;

	float2 tile_relative_pos = t_cord - tile_start;
	float2 normalized_tile_coord = tile_relative_pos * g_screen_tile_size.xy;
	normalized_tile_coord.y = 1.0f - normalized_tile_coord.y;

	current_tile_index = tile_index.x + tile_index.y * g_screen_tile_size.x;
	if (prev_frame)
	{
		current_tile_index += 256;
	}
	float2 atlas_coord_start = g_screen_tile_info[current_tile_index].xy;
	float2 real_atlas_tile_size = g_screen_tile_info[current_tile_index].zw;

	debugging_params.cur_atlas_size = real_atlas_tile_size.x;
	

	if (prev_frame == false)
	{
		int current_data;
		//InterlockedAdd(tile_render_data[current_tile_index].x, 1, current_data);
	}

	//atlas_coord_start.y = 1.0f - atlas_coord_start.y;
	float2 texture_coord = atlas_coord_start + real_atlas_tile_size * normalized_tile_coord;
	//texture_coord.y = 1.0f - texture_coord.y;

	debugging_params.atlas_uv = texture_coord;

	if (prev_frame)
	{
		return last_frame_sun_amount_texture.Sample(LinearSampler, texture_coord).x;
	}
	else
	{
		return light_shadow_depth_texture.Sample(LinearSampler, texture_coord).x;
	}
}

float3 get_light_res_heatmap(float size)
{
	float heat = size / MAX_RES;

	return lerp(float3(0, 1, 0), float3(1, 0, 0), heat);
}

float3 get_ws_position_from_uv(float2 tex_coord, float hw_depth)
{
	float4 cameraRay = float4(tex_coord * 2.0 - 1.0, 1.0, 1.0);
	cameraRay.y *= -1;
	cameraRay = mul(cameraRay, g_inv_projection_matrix);
	cameraRay = cameraRay / cameraRay.w;
	cameraRay.w = 0;
	cameraRay = normalize(cameraRay);

	float hit_linear_depth = hw_depth_to_linear_depth(hw_depth);
	float3 view_space_position = cameraRay * hit_linear_depth;
	return mul(float4(view_space_position, 1), g_inv_view_matrix).xyz;
}

float3 get_ws_position_from_linear_depth(float iDepth, float4 iPosProj)
{
	float3 vPosView = mul(iPosProj, g_inv_projection_matrix).xyz;
	float3 vViewRay = float3(vPosView.xy * (g_near_far_padding2.y / vPosView.z), g_near_far_padding2.y);
	float3 vPosition = vViewRay * iDepth;

	return vPosition;
}

float get_light_sun_amount(float3 world_position, float last_frame_sun_ammount, inout DeferredDebuggingParams debugging_params)
{
	float4 light_ss_position = mul(float4(world_position, 1), g_light_view_projection_matrix);
	//light_ss_position /= light_ss_position.w;
	debugging_params = (DeferredDebuggingParams)0;

	if (light_ss_position.x < -1.0f || light_ss_position.y < -1.0f ||
		light_ss_position.x > 1.0f || light_ss_position.y > 1.0f)
	{
		return 1.0f;
	}

	DeferredDebuggingParams debug_params_prev = (DeferredDebuggingParams)0;

	int current_tile_index = 0;
	float2 light_texture_coord = light_ss_position.xy * 0.5 + 0.5;
#ifdef USE_TILED_LIGHT_SHADOW_RENDERING
	float prev_shadow_depth = get_normal_from_atlas_texture(light_texture_coord, current_tile_index, debug_params_prev, true).x;
	float shadow_depth = get_normal_from_atlas_texture(light_texture_coord, current_tile_index, debugging_params, false).x;

	float cur_sun_amount = (shadow_depth + 1e-6) > light_ss_position.z;
	float prev_sun_amount = (prev_shadow_depth + 1e-6) > light_ss_position.z;

	float light_amount_diff = abs(cur_sun_amount - prev_sun_amount);
	if (light_amount_diff > 0.5f)
	{
		int current_data;
		InterlockedAdd(tile_render_data[current_tile_index].y, 1, current_data);
	}

	debugging_params.shadow_depth = shadow_depth;

#else
	//light_texture_coord.y = 1.0f - light_texture_coord.y;
	float shadow_depth = light_shadow_depth_texture.SampleLevel(PointSampler, light_texture_coord, 0);
	float cur_sun_amount = (shadow_depth + 1e-6) > light_ss_position.z;
#endif

	return cur_sun_amount;
}

float3 VSPositionFromDepth(float2 vTexCoord, float z)
{
	// Get x/w and y/w from the viewport position
	float x = vTexCoord.x * 2 - 1;
	float y = (1 - vTexCoord.y) * 2 - 1;
	float4 vProjectedPos = float4(x, y, z, 1.0f);
	// Transform by the inverse projection matrix
	float4 vPositionVS = mul(vProjectedPos, g_inv_view_projection_matrix);
	// Divide by w to get the view-space position
	return vPositionVS.xyz / vPositionVS.w;
}

void shade_pixel_deferred(float2 tex_coord, float3 world_position, float3 albedo_color, float3 normal, in out OUTPUT output)
{
	float last_frame_sun_amount = last_frame_sun_amount_texture.Sample(LinearSampler, tex_coord.xy).r;

	float3 ray_to_light = -g_ws_light_position.xyz;// -world_position;
	ray_to_light = normalize(ray_to_light);

	DeferredDebuggingParams debug_params = (DeferredDebuggingParams)0;
	float nDotL = max(dot(ray_to_light, normal), 0.0f);
	float light_amount = get_light_sun_amount(world_position, last_frame_sun_amount, debug_params);
	float light_amount_diff = ddx(light_amount) + ddy(light_amount);

	output.light_amount = float4(light_amount.xxx, 1);

	if (g_debug_vector.z > 0)
	{
		output.color = float4(get_light_res_heatmap(debug_params.cur_atlas_size), 1);
		return;
	}

	output.color = float4((nDotL * light_amount + 0.3) * albedo_color, 1);

#ifdef USE_TILED_LIGHT_SHADOW_RENDERING
	//if (debug_params.cur_shadow_tile_index.x == (int)g_debug_vector.x && debug_params.cur_shadow_tile_index.y == (int)g_debug_vector.y)
	//{
	//	output.color = float4(1, 0, 1, 1);
	//}
	output.color = g_debug_vector.w > 0 ? float4(light_amount_diff.xxx, 1) : output.color;

	//if (debug_params.cur_shadow_tile_index.x == 9 && debug_params.cur_shadow_tile_index.y == 5)
	//{
	//	output.color = float4(pow(debug_params.shadow_depth.xxx, 8),1);
	//	
	//}
	//else
	//{
	//	output.color = float4((float2)debug_params.cur_shadow_tile_index / 16.0f, 0 , 1);
	//}
	//
	if (g_debug_vector.x > 0)
	{
		output.color = float4((float2)debug_params.cur_shadow_tile_index / 16.0f, 0, 1);
	}
	
	if (g_debug_vector.y > 0)
	{
		output.color = float4(debug_params.atlas_uv, 0, 1);
	}

	
#endif

}