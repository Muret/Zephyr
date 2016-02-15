#ifndef USE_MATERIAL_H_
#define USE_MATERIAL_H_

#include <string>

class Texture;
class Shader;

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

	void create_from_file(std::string texture_names[mtt_count], const D3DXVECTOR4 &diffuse_color);
	void set_textures();
	D3DXVECTOR4 get_diffuse_color() const;

	void set_enforced_gbuffer_shader(Shader *shader);
	Shader* get_enforced_gbuffer_shader() const;
private:

	Shader *enforced_gbuffer_shader;

	std::string texture_names_[mtt_count];
	Texture* textures_[mtt_count];

	D3DXVECTOR4 diffuse_color_;

	std::string name_;

};


#endif