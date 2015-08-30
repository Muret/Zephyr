#ifndef USE_MATERIAL_H_
#define USE_MATERIAL_H_

#include <string>

class Texture;

class Material
{

public:

	enum materialTextureType
	{
		mtt_diffuse = 0,
		mtt_normal,
		mtt_specular,
		mtt_count,
	};


	Material();

	void create_from_file(std::string texture_names[mtt_count]);
	void set_textures();

private:

	std::string texture_names_[mtt_count];
	Texture* textures_[mtt_count];

	std::string name_;

};


#endif