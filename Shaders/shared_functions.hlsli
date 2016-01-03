#ifndef __INCLUDE_SHARED_FUNCTIONS_HLSLI
#define __INCLUDE_SHARED_FUNCTIONS_HLSLI

float hw_depth_to_linear_depth(float hw_depth)
{
	float near = near_far_padding2.x;
	float far = near_far_padding2.y;
	return -1 * near * far / ((hw_depth * (far - near)) - far);

	//float m1 = projectionMatrix._m23;
	//float m2 = projectionMatrix._m22;
	//return m1 / (hw_depth - m2);
}




#endif