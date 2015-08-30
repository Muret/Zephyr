
#include "Definitions.hlsli"
#include "shared_functions.hlsli"

Texture2D hi_z_depth_texture : register(t4);


#define MAX_ITERATIONS 25

float4 main(PixelInputType input) : SV_TARGET
{
	float3 screen_color = screen_texture.Sample(SampleType, input.tex_coord.xy);

	//calculate the view space normal and hit position
	//then calculate the view space reflection vector
	//convert all those to screen space

	float4 screen_space_position;
	float4 screen_space_ray_dir;
	{
		float4 cameraRay = float4(input.tex_coord.xy * 2.0 - 1.0, 1.0, 1.0);
		cameraRay = mul(inverseProjectionMatrix, cameraRay);
		cameraRay = cameraRay / cameraRay.w;

		float hit_hw_depth = hi_z_depth_texture.SampleLevel(SampleType, input.tex_coord.xy, 0);
		float hit_linear_depth = hw_depth_to_linear_depth (hit_hw_depth);

		float3 view_space_position = cameraRay * hit_linear_depth;

		float3 view_space_normal = normalize(normal_texture.SampleLevel(SampleType, input.tex_coord.xy, 0));
		float3 view_space_view_dir = float3(cameraRay.xy, hit_linear_depth);
		float3 view_space_reflected_ray_dir = normalize(reflect(view_space_view_dir, view_space_normal));

		screen_space_position = mul(projectionMatrix, float4(view_space_position, 1));
		screen_space_position = screen_space_position / screen_space_position.w;

		screen_space_ray_dir = mul(projectionMatrix, float4(view_space_reflected_ray_dir, 0));
		screen_space_ray_dir = normalize(screen_space_ray_dir);
	}

	float3 current_delta_uv = float3(1.0f, 1.0f, 1.0f) / screen_texture_half_pixel_forced_mipmap.xxx;
	int current_lod_level = 0;

	int iteration_count = 0;
	while (current_lod_level >= 0)
	{
		float3 next_screen_position = screen_space_position + screen_space_ray_dir * current_delta_uv;
		
		float real_pixel_depth = hi_z_depth_texture.Sample(SampleType, next_screen_position.xy);

		if (next_screen_position.z > real_pixel_depth)
		{
			current_lod_level--;
		}
		else
		{
			current_lod_level++;
			current_delta_uv *= 2.0f;
		}

		iteration_count++;

		if (iteration_count == MAX_ITERATIONS)
		{
			return float4(screen_color,1);
		}
	}

	float3 final_position = screen_space_position + screen_space_ray_dir * current_delta_uv;

	float3 reflection_color = 0;
	if (final_position.x < 1.0f && final_position.x > 0.0f &&
		final_position.y < 1.0f && final_position.y > 0.0f)
	{
		screen_color += screen_texture.Sample(SampleType, final_position.xy);
	}

	return float4(screen_color,1);

}