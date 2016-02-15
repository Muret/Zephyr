
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

	//return float4(1, 0, 1, 1);

	float3 final_color = screen_color * 0.1;

	//if (check_if_reflected(normal))
	//{
	//	return float4(screen_color,1);
	//}

	float3 screen_space_position, screen_space_ray_dir, ws_position;
	get_ss_hit_pos_ray_dir(input.tex_coord.xy, screen_space_position, screen_space_ray_dir, ws_position);

	final_color = do_simple_path_tracing(screen_space_position);
	return float4(final_color, 1);

	float3 vector_to_light = normalize(g_ws_light_position - ws_position.xyz);
	float3 ws_normal = normalize(normal * 2.0f - float3(1.0f,1.0f,1.0f));

	float iteration_count;
	float nDotL = saturate(dot(vector_to_light, ws_normal));
	if (check_visibility_ss(screen_space_position, g_ss_light_position.xyz))
	{
		final_color += g_light_color * nDotL * float4(screen_color, 1);
		//return float4(1, 0, 0,1);
	}

	//return float4(normalize(normal), 1);

	//return float4(nDotL.xxx,1);
	return float4(final_color, 1);

	float3 final_position = 0;// do_hiz_ss_ray_trace(screen_space_position, screen_space_ray_dir, iteration_count);

	float3 reflection_color = 0;
	if (final_position.x > 1.0f && final_position.x < 0.0f &&
		final_position.y > 1.0f && final_position.y < 0.0f)
	{
		return float4(1,0,1,1);
	}

	//final_color += screen_texture.Sample(PointSampler, float2(final_position.x, 1.0f - final_position.y)).rgb * 0.6;

	return float4(final_color,1);

}