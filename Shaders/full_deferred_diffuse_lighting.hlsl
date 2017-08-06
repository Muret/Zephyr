
#include "Definitions.hlsli"
#include "shared_functions.hlsli"
#include "deferred_shading_functions.hlsli"

Texture2D diffuse_texture : register(t0);
Texture2D normal_texture : register(t1);
Texture2D specular_texture : register(t2);

OUTPUT main(PixelInputType input)
{
	OUTPUT output;

	float3 albedo_color = diffuse_texture.Sample(PointSampler, input.tex_coord.xy).rgb;
	
	float4 packet_normal_depth = normal_texture.Sample(PointSampler, input.tex_coord);
	float3 normal = packet_normal_depth.rgb * 2.0f - 1.0f;
	normal = normalize(normal);

	float3 world_position = VSPositionFromDepth(input.tex_coord.xy, packet_normal_depth.a);

	shade_pixel_deferred(input.tex_coord, world_position, albedo_color, normal, output);

	//output.color = float4(normal * 0.5 + 0.5, 1);
	//output.color = float4(1,0,1, 1);

	return output;
}