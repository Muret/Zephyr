
#include "Definitions.hlsli"
#include "shared_functions.hlsli"
#include "deferred_shading_functions.hlsli"

Texture2DMS<float4, 4> diffuse_texture : register(t0);
Texture2DMS<float4, 4> normal_texture : register(t1);
Texture2DMS<float4, 4> specular_texture : register(t2);
Texture2D resolved_albedo_texture : register(t3);

OUTPUT main(PixelInputType input)
{
	OUTPUT output = (OUTPUT)0;

	int2 load_position = input.tex_coord.xy * float2(1024, 1024);
	bool is_complex_pixel = resolved_albedo_texture.Sample(PointSampler, input.tex_coord.xy).a > 0.0f;
	
	if (is_complex_pixel)
	{
		if (g_debug_vector.x > 0)
		{
			output.color = float4(1, 0, 1, 1);
			return output;
		}

		float4 final_color = 0;

		[unroll]
		//for (int i = 0; i < 4; i++)
		//{
		//	float3 albedo_color = diffuse_texture.Load(load_position, i).rgb;
		//
		//	float4 packet_normal_depth = normal_texture.Load(load_position, i);
		//	float3 normal = packet_normal_depth.rgb * 2.0f - 1.0f;
		//	normal = normalize(normal);
		//
		//	float3 world_position = VSPositionFromDepth(input.tex_coord.xy, packet_normal_depth.a);
		//
		//	shade_pixel_deferred(input.tex_coord, world_position, albedo_color, normal, output);
		//	final_color += output.color;
		//}

		output.color = final_color * 0.25;
	}
	else
	{
		float3 albedo_color = diffuse_texture.Load(load_position, 0).rgb;

		float4 packet_normal_depth = normal_texture.Load(load_position, 0);
		float3 normal = packet_normal_depth.rgb * 2.0f - 1.0f;
		normal = normalize(normal);

		float3 world_position = VSPositionFromDepth(input.tex_coord.xy, packet_normal_depth.a);

		shade_pixel_deferred(input.tex_coord, world_position, albedo_color, normal, output);
	}

	return output;
}