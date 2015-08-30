
#include "Texture.h"
#include "Material.h"
#include "d11.h"
#include "TextureLoader.h"

Material::Material()
{
	for (int i = 0; i < mtt_count; i++)
	{
		texture_names_[i] = "";
		textures_[i] = nullptr;
	}
}

void Material::create_from_file(std::string texture_names[mtt_count])
{
	for (int i = 0; i < mtt_count; i++)
	{
		texture_names_[i] = texture_names[i];

		if (texture_names_[i] != "")
		{
			std::string file_path = texture_names_[i];
			TextureLoader loader(file_path);
			textures_[i] = loader.create_texture_from_file();
		}
	}
}

void Material::set_textures()
{
	if (textures_[mtt_diffuse]  != nullptr)
	{
		ID3D11ShaderResourceView *srv = textures_[mtt_diffuse]->get_srv();
		SetSRV(&srv, 1, shader_type_pixel, 0);
	}

	if (textures_[mtt_normal] != nullptr)
	{
		ID3D11ShaderResourceView *srv = textures_[mtt_normal]->get_srv();
		SetSRV(&srv, 1, shader_type_pixel, 1);
	}

	if (textures_[mtt_specular] != nullptr)
	{
		ID3D11ShaderResourceView *srv = textures_[mtt_specular]->get_srv();
		SetSRV(&srv, 1, shader_type_pixel, 2);
	}

	SetSamplerState();

}

