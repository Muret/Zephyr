
#include "Definitions.hlsli"

PixelInputType main(VertexInputType input)
{
	PixelInputType output;

	float4 position = float4(input.position.xyz, 1);

	output.position = mul(position, g_world_view_projection_matrix);
	output.position /= output.position.w;
	
	output.color = 0;

	output.tex_coord = float4(input.tex_coord.xy, 0, 0);
	output.tangent = input.tangent;
	output.world_normal = input.normal;
	
	return output;
}