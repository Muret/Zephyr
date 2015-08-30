#ifndef __INCLUDE_SHARED_FUNCTIONS_HLSLI
#define __INCLUDE_SHARED_FUNCTIONS_HLSLI

float hw_depth_to_linear_depth(float hw_depth)
{
	return projectionMatrix._m32 / (hw_depth - projectionMatrix._m22);
}




#endif