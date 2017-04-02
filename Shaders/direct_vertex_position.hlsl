

#include "Definitions.hlsli"

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////

PixelInputType main(VertexInputType input)
{
	PixelInputType output; 

	float4 position = float4(input.position.xyz, 1);

	output.position = position;
	output.color = 0;

	output.tex_coord = float4(input.tex_coord.xy,0,0);
	output.tangent = input.tangent;
	output.world_normal = mul(float4(input.normal.xyz,0), g_world_matrix);
	output.world_position = mul(position, g_world_matrix);
	output.ss_position = position;

	return output;
}


