#ifndef INCLUDE_NOISE_
#define INCLUDE_NOISE_

#include "includes.h"
#include <noise/noise.h>
#include "libnoise/noiseutils.h"

class NoiseInstance
{
public:

	struct NoiseGeneratorParams
	{
		int number_of_octaves_;
		int seed_;
		float noise_frequency;
	};

	NoiseInstance();
	~NoiseInstance();

	float generate_3d(const D3DXVECTOR3 &pos);
	void build_2d_heightmap(const D3DXVECTOR2 &min, const D3DXVECTOR2 &max, const D3DXVECTOR2 &resolution_, const NoiseGeneratorParams& params);
	float get_heightmap_value(const D3DXVECTOR2 &pos);

private:

	//noise::module::Perlin noise_instance_;
	//noise::utils::NoiseMap height_map_;
	D3DXVECTOR2 min_;
	D3DXVECTOR2 max_;
	D3DXVECTOR2 resolution_;
};

#endif