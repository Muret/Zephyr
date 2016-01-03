
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
	float3 normal = normal_texture.Sample(PointSampler, input.tex_coord.xy);
	float3 screen_color = screen_texture.Sample(PointSampler, input.tex_coord.xy);
	//return abs(float4(normal * 2.0f - 1.0f, 1));

	if (check_if_reflected(normal))
	{
		return float4(screen_color,1);
	}

	float3 screen_space_position, screen_space_ray_dir;
	get_ss_hit_pos_ray_dir(input.tex_coord.xy, screen_space_position, screen_space_ray_dir);

	float3 final_position = do_hiz_ss_ray_trace(screen_space_position, screen_space_ray_dir);

	float3 reflection_color = 0;
	if (final_position.x < 1.0f && final_position.x > 0.0f &&
		final_position.y < 1.0f && final_position.y > 0.0f)
	{
		return float4(1,0,1,1);
	}

	return screen_texture.Sample(PointSampler, final_position.xy) * 0.5;

}