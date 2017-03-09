
#include "Definitions.hlsli"
#include "shared_functions.hlsli"

#define USE_TILED_SHADOW_LIGHT_RENDERING

float get_normal_from_atlas_texture(float2 t_cord)
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
	int current_tile_index = tile_index.x + tile_index.y * g_screen_tile_size.x;
	float2 atlas_coord_start = g_screen_tile_info[current_tile_index].xy;
	float2 real_atlas_tile_size = g_screen_tile_info[current_tile_index].zw;

	float2 texture_coord = atlas_coord_start + real_atlas_tile_size * normalized_tile_coord;
	texture_coord.y = 1.0f - texture_coord.y;

	return light_shadow_depth_texture.Sample(LinearSampler, texture_coord).x;
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

float get_light_sun_amount(float3 world_position)
{
	float4 light_ss_position = mul(float4(world_position, 1), g_light_view_projection_matrix);
	light_ss_position /= light_ss_position.w;

	if (light_ss_position.x < -1.0f || light_ss_position.y < -1.0f ||
		light_ss_position.x > 1.0f || light_ss_position.y > 1.0f)
	{
		return 1.0f;
	}

	float2 light_texture_coord = light_ss_position.xy * 0.5 + 0.5;
	//light_texture_coord.y = 1.0f - light_texture_coord;
#ifdef USE_TILED_SHADOW_LIGHT_RENDERING
	float shadow_depth = get_normal_from_atlas_texture(light_texture_coord).x;
#else
	float shadow_depth = light_shadow_depth_texture.SampleLevel(PointSampler, light_texture_coord, 0);
#endif
	//return (shadow_depth - 0.98) * 50;

	return (shadow_depth + 1e-6) > light_ss_position.z;
}

float4 main(PixelInputType input) : SV_TARGET
{
	float3 albedo_color = diffuse_texture.Sample(LinearSampler, input.tex_coord.xy).rgb;
	//float3 normal = get_normal_from_atlas_texture(input.tex_coord.xy).rgb;
	float3 normal = normal_texture.Sample(LinearSampler, input.tex_coord).rgb * 2.0f - 1.0f;
	normal = normalize(normal);

	float hit_hw_depth = depth_texture.SampleLevel(PointSampler, input.tex_coord.xy, 0);
	float3 world_position = get_ws_position_from_uv(input.tex_coord.xy, hit_hw_depth);

	float light_amount = get_light_sun_amount(world_position);

	float3 ray_to_light = g_ws_light_position.xyz - world_position;
	ray_to_light = normalize(ray_to_light);

	float3 final_color = (max(dot(ray_to_light, normal), 0.0f) * light_amount + 0.3) * albedo_color;

	//return float4(normal * 0.5 + 0.5, 1);
	return float4(final_color, 1);

	//return float4(normal, 1);
	//return float4(normal * 0.5 + 0.5, 1);
	////return float4(1,0,1, 1);
	//return float4(albedo_color, 1);

}