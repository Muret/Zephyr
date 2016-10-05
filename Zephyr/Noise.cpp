
#include "Noise.h"

NoiseInstance::NoiseInstance()
{
}

NoiseInstance::~NoiseInstance()
{
}

float NoiseInstance::generate_3d(const D3DXVECTOR3 & pos)
{
	return noise_instance_.GetValue(pos.x, pos.y, pos.z);
}

void NoiseInstance::build_2d_heightmap(const D3DXVECTOR2 & min, const D3DXVECTOR2 & max, const D3DXVECTOR2 &resolution, const NoiseGeneratorParams& params)
{
	min_ = min;
	max_ = max;
	resolution_ = resolution;

	module::RidgedMulti mountainTerrain;
	mountainTerrain.SetSeed(params.seed_);

	module::Billow baseFlatTerrain;
	baseFlatTerrain.SetFrequency(2.0);

	module::ScaleBias flatTerrainScaleBias;
	flatTerrainScaleBias.SetSourceModule(0, baseFlatTerrain);
	flatTerrainScaleBias.SetScale(0.125);
	flatTerrainScaleBias.SetBias(-0.75);

	module::Perlin terrainBlend;
	terrainBlend.SetFrequency(0.5);
	terrainBlend.SetPersistence(0.25);
	terrainBlend.SetSeed(params.seed_);

	module::Select finalTerrain;
	finalTerrain.SetSourceModule(0, flatTerrainScaleBias);
	finalTerrain.SetSourceModule(1, mountainTerrain);
	finalTerrain.SetControlModule(terrainBlend);
	finalTerrain.SetEdgeFalloff(0.125);

	noise_instance_.SetSeed(params.seed_);
	noise_instance_.SetOctaveCount(max(params.number_of_octaves_, 1));
	noise_instance_.SetFrequency(max(params.noise_frequency, 1.0f));

	utils::NoiseMapBuilderPlane heightMapBuilder;
	heightMapBuilder.SetSourceModule(finalTerrain);
	heightMapBuilder.SetDestNoiseMap(height_map_);
	heightMapBuilder.SetDestSize(resolution.x, resolution.y);
	heightMapBuilder.SetBounds(min.x, max.x, min.y, max.y);
	//heightMapBuilder.SetBounds(-90.0, 90.0, -180.0, 180.0);
	heightMapBuilder.Build();
}

float NoiseInstance::get_heightmap_value(const D3DXVECTOR2 & pos)
{
	ZEPHYR_ASSERT(pos.x < max_.x && pos.y < max_.y);
	ZEPHYR_ASSERT(pos.x > min_.x && pos.y > min_.y);

	D3DXVECTOR2 length = max_ - min_;
	int pixel_x = (pos.x - min_.x) / length.x * resolution_.x;
	int pixel_y = (pos.y - min_.y) / length.y * resolution_.y;
	
	return height_map_.GetValue(pixel_x, pixel_y);
}
