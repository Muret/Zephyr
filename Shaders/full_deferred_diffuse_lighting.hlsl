
#include "Definitions.hlsli"
#include "shared_functions.hlsli"

#define USE_TILED_SHADOW_LIGHT_RENDERING

RWStructuredBuffer<int2> tile_render_data : register(u4);

Texture2D last_frame_sun_amount_texture : register(t6);

float get_normal_from_atlas_texture(float2 t_cord, out int current_tile_index, out float cur_atlas_size, bool prev_frame)
{
	float2 tile_index = 0;
	tile_index.x = int(t_cord.x * g_screen_tile_size.x);
	tile_index.y = int(t_cord.y * g_screen_tile_size.y);

	tile_index.x = min(tile_index.x, g_screen_tile_size.x - 1);
	tile_index.y = min(tile_index.y, g_screen_tile_size.y - 1);
	
	float2 tile_start = tile_index * g_screen_tile_size.zw;

	float2 tile_relative_pos = t_cord - tile_start;
	float2 normalized_tile_coord = tile_relative_pos * g_screen_tile_size.xy;
	//
	current_tile_index = tile_index.x + tile_index.y * g_screen_tile_size.x;
	if (prev_frame)
	{
		current_tile_index += 64;
	}
	float2 atlas_coord_start = g_screen_tile_info[current_tile_index].xy;
	float2 real_atlas_tile_size = g_screen_tile_info[current_tile_index].zw;

	cur_atlas_size = real_atlas_tile_size.x;

	if (prev_frame == false)
	{
		int current_data;
		InterlockedAdd(tile_render_data[current_tile_index].x, 1, current_data);
	}

	float2 texture_coord = atlas_coord_start + real_atlas_tile_size * normalized_tile_coord;
	texture_coord.y = 1.0f - texture_coord.y;

	if (prev_frame)
	{
		return last_frame_sun_amount_texture.Sample(LinearSampler, texture_coord).x;
	}
	else
	{
		return light_shadow_depth_texture.Sample(LinearSampler, texture_coord).x;
	}
}

#define MAX_RES 0.0062

float3 get_light_res_heatmap(float size)
{
	float heat = size / MAX_RES;

	return lerp(float3(0,1,0), float3(1,0,0), heat);
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

float get_light_sun_amount(float3 world_position, float last_frame_sun_ammount, in out float cur_atlas_size)
{
	float4 light_ss_position = mul(float4(world_position, 1), g_light_view_projection_matrix);
	light_ss_position /= light_ss_position.w;

	if (light_ss_position.x < -1.0f || light_ss_position.y < -1.0f ||
		light_ss_position.x > 1.0f || light_ss_position.y > 1.0f)
	{
		return 1.0f;
	}

	int current_tile_index = 0;
	float2 light_texture_coord = light_ss_position.xy * 0.5 + 0.5;
	//light_texture_coord.y = 1.0f - light_texture_coord;
#ifdef USE_TILED_SHADOW_LIGHT_RENDERING
	float prev_shadow_depth = get_normal_from_atlas_texture(light_texture_coord, current_tile_index, cur_atlas_size, true).x;
	float shadow_depth = get_normal_from_atlas_texture(light_texture_coord, current_tile_index, cur_atlas_size, false).x;
#else
	float shadow_depth = light_shadow_depth_texture.SampleLevel(PointSampler, light_texture_coord, 0);
#endif

	float cur_sun_amount = (shadow_depth + 1e-6) > light_ss_position.z;
	float prev_sun_amount = (prev_shadow_depth + 1e-6) > light_ss_position.z;

	float light_amount_diff = abs(cur_sun_amount - prev_sun_amount);
	if (light_amount_diff > 0.5f)
	{
		int current_data;
		InterlockedAdd(tile_render_data[current_tile_index].y, 1, current_data);
	}

	return cur_sun_amount;
}

struct OUTPUT
{
	float4 color : SV_TARGET0;
	float4 light_amount : SV_TARGET1;
};

OUTPUT main(PixelInputType input)
{
	OUTPUT output;

	float last_frame_sun_amount = last_frame_sun_amount_texture.Sample(LinearSampler, input.tex_coord.xy).r;

	float3 albedo_color = diffuse_texture.Sample(LinearSampler, input.tex_coord.xy).rgb;
	//float3 normal = get_normal_from_atlas_texture(input.tex_coord.xy).rgb;
	float3 normal = normal_texture.Sample(LinearSampler, input.tex_coord).rgb * 2.0f - 1.0f;
	normal = normalize(normal);

	float hit_hw_depth = depth_texture.SampleLevel(PointSampler, input.tex_coord.xy, 0);

	if (hit_hw_depth == 1.0f)
	{
		clip(-1);
	}

	float3 world_position = get_ws_position_from_uv(input.tex_coord.xy, hit_hw_depth);

	float3 ray_to_light = g_ws_light_position.xyz - world_position;
	ray_to_light = normalize(ray_to_light);

	float cur_atlas_size = 0;
	float nDotL = max(dot(ray_to_light, normal), 0.0f);
	float light_amount = get_light_sun_amount(world_position, last_frame_sun_amount, cur_atlas_size);
	float light_amount_diff = ddx(light_amount) +ddy (light_amount);
	//return float4(light_amount_diff.xxx, 1);

	output.light_amount = float4(light_amount.xxx, 1);

	if (g_debug_vector.y > 0)
	{
		output.color = float4(get_light_res_heatmap(cur_atlas_size), 1);
		return output;
	}

	output.color = g_debug_vector.x > 0 ? float4(light_amount_diff.xxx, 1) : float4((nDotL * light_amount + 0.1) * albedo_color, 1);
	

	//return float4(normal * 0.5 + 0.5, 1);
	return output;

	//return float4(normal, 1);
	//return float4(normal * 0.5 + 0.5, 1);
	////return float4(1,0,1, 1);
	//return float4(albedo_color, 1);

}