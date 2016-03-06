

#include "Definitions.hlsli"

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////

float3 rot(float3 pos, float angle) 
{
	float s = sin(angle);// * g_debug_vector.x;
	float c = cos(angle);// * g_debug_vector.y;
	return float3( c*pos.x - s*pos.z, pos.y , s*pos.x + c*pos.z);
}

PixelInputType main(VertexInputType input)
{
	PixelInputType output; 

	float4 position = float4(input.position.xyz, 1);

	//bend
	if(false)
	{
		float2 center = (g_bb_min.xz + g_bb_max.xz) * 0.5f;
		float2 length = (g_bb_max.xz - g_bb_min.xz) * 0.5f;

		float x_value = (position.x - center.x) / length.x;
		float y_value = (position.z - center.y) / length.y;
		y_value = y_value * 0.5 + 0.5;

		float one_over_sqrt = 1.0f / sqrt(2.0f);
		
		if (x_value > 0)
		{
			//x_value = saturate(x_value * 2.2f);

			
			//x_value = x_value * (1.0f + g_debug_vector.y * 0.2 * (1.0f - y_value));

			//y_value = min(y_value, g_debug_vector.y);

			float2 curved_center_position = center + float2(+length.x * one_over_sqrt, length.x * one_over_sqrt);
			//float2 y_max_position = curved_center_position + float2(-length.y * one_over_sqrt, +length.y * one_over_sqrt);
			//float2 y_min_position = curved_center_position + float2(+length.y * one_over_sqrt, -length.y * one_over_sqrt);

			//float2 end_position = y_max_position * y_value + y_min_position * (1.0f - y_value);

			position.x += (position.x - center.x) * (1.0f - y_value) * g_debug_vector.y * 0.1;

			float3 y_max_position = rot(position.xyz , x_value * PI * g_debug_vector.x * 0.1);
			float3 y_min_position = rot(position.xyz, x_value * PI * g_debug_vector.x * 0.1);

			position.xyz = y_max_position * y_value + y_min_position * (1.0f - y_value);
			//position.xz = end_position * x_value + position * (1.0f - x_value);
		}
		else
		{
			float2 curved_center_position = center + float2(-length.x * one_over_sqrt, length.x * one_over_sqrt);
		}
	}

	output.position = mul(position, g_world_view_projection_matrix);
	output.color = input.color;
	output.tex_coord = float4(input.tex_coord.xy,0,0);
	output.tangent = input.tangent;
	output.world_normal = mul(float4(input.normal.xyz,0), g_world_matrix);
	output.world_position = mul(position, g_world_matrix);
	return output;
}


