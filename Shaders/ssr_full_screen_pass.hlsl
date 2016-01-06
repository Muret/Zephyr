
#include "Definitions.hlsli"
#include "shared_functions.hlsli"
#include "ssr_helpers.hlsli"

bool check_if_reflected(float3 normal)
{
	float t = abs(normal.x - 0);
	t += abs(normal.y - 0.5);
	t += abs(normal.z - 0.5);

	return t < 1e-2;
}

float4 main(PixelInputType input) : SV_TARGET
{
	//return pow(float4(hi_z_depth_texture.SampleLevel(PointSampler, input.tex_coord.xy, 2).rrr, 1), 2);

	float3 normal = normal_texture.Sample(PointSampler, input.tex_coord.xy);
	float3 screen_color = screen_texture.Sample(PointSampler, input.tex_coord.xy);

	float3 final_color = screen_color * 0.4;

	if (check_if_reflected(normal))
	{
		return float4(screen_color,1);
	}

	float3 screen_space_position, screen_space_ray_dir;
	get_ss_hit_pos_ray_dir(input.tex_coord.xy, screen_space_position, screen_space_ray_dir);

	float3 final_position = do_hiz_ss_ray_trace(screen_space_position, screen_space_ray_dir);

	float3 reflection_color = 0;
	if (final_position.x > 1.0f && final_position.x < 0.0f &&
		final_position.y > 1.0f && final_position.y < 0.0f)
	{
		return float4(1,0,1,1);
	}

	final_color += screen_texture.Sample(PointSampler, float2(final_position.x, 1.0f - final_position.y)).rgb * 0.6;

	return float4(final_color,1);

}