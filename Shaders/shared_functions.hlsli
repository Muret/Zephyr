#ifndef __INCLUDE_SHARED_FUNCTIONS_HLSLI
#define __INCLUDE_SHARED_FUNCTIONS_HLSLI

float hw_depth_to_linear_depth(float hw_depth)
{
	float near = g_near_far_padding2.x;
	float far = g_near_far_padding2.y;
	return -1 * near * far / ((hw_depth * (far - near)) - far);

	//float m1 = projectionMatrix._m23;
	//float m2 = projectionMatrix._m22;
	//return m1 / (hw_depth - m2);
}



struct Randomer 
{
	// Source
	// http://www.gamedev.net/topic/592001-random-number-generation-based-on-time-in-hlsl/
	// Supposebly from the NVidia Direct3D10 SDK
	// Slightly modified for my purposes
	#define RANDOM_IA 16807
	#define RANDOM_IM 2147483647
	#define RANDOM_AM (1.0f/float(RANDOM_IM))
	#define RANDOM_IQ 127773u
	#define RANDOM_IR 2836
	#define RANDOM_MASK 123459876

	int seed; // Used to generate values.

	// Returns the current random float.
	float GetCurrentFloat() 
	{
		Cycle();
		return RANDOM_AM * seed;
	}

	// Returns the current random int.
	int GetCurrentInt() 
	{
		Cycle();
		return seed;
	}

	// Generates the next number in the sequence.
	void Cycle() 
	{
		seed ^= RANDOM_MASK;
		int k = seed / RANDOM_IQ;
		seed = RANDOM_IA * (seed - k * RANDOM_IQ) - RANDOM_IR * k;

		if (seed < 0)
			seed += RANDOM_IM;

		seed ^= RANDOM_MASK;
	}

	// Cycles the generator based on the input count. Useful for generating a thread unique seed.
	// PERFORMANCE - O(N)
	void Cycle(const uint _count) 
	{
		for (uint i = 0; i < _count; ++i)
			Cycle();
	}

	// Returns a random float within the input range.
	float GetRandomFloat(const float low, const float high) 
	{
		float v = GetCurrentFloat();
		return low * (1.0f - v) + high * v;
	}

	// Sets the seed
	void SetSeed(const uint value) 
	{
		seed = int(value);
		Cycle();
	}
};

#endif