
#include "Definitions.hlsli"
#include "shared_functions.hlsli"
#include "deferred_shading_functions.hlsli"

Texture2DMS<float4, 4> diffuse_texture : register(t0);
Texture2DMS<float4, 4> normal_texture : register(t1);
Texture2DMS<float4, 4> specular_texture : register(t2);

OUTPUT main(PixelInputType input, uint	i : SV_SampleIndex)
{
	OUTPUT output = (OUTPUT)0;

	float3 albedo_color = diffuse_texture.Load(input.tex_coord.xy, 0).rgb;
	
	float4 packet_normal_depth = normal_texture.Load(input.tex_coord, 0);
	float3 normal = packet_normal_depth.rgb * 2.0f - 1.0f;
	normal = normalize(normal);

	float3 world_position = VSPositionFromDepth(input.tex_coord.xy, packet_normal_depth.a);

	shade_pixel_deferred(input.tex_coord, world_position, albedo_color, normal, output);

	return output;
}