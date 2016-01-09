#include "includes.h"
#include <iostream>

#include "Texture.h"
#include "Material.h"
#include "d11.h"
#include "ResourceManager.h"
#include "Utilities.h"

Material::Material()
{
	for (int i = 0; i < mtt_count; i++)
	{
		texture_names_[i] = "";
		textures_[i] = nullptr;
	}
}

void Material::create_from_file(std::string texture_names[mtt_count], const D3DXVECTOR4 &diffuse_color)
{
	diffuse_color_ = diffuse_color;

	for (int i = 0; i < mtt_count; i++)
	{
		texture_names_[i] = texture_names[i];

		if (texture_names_[i] != "")
		{
			string direct_name = Utilities::get_file_name_from_path(texture_names_[i]);
			Texture *t = resource_manager.get_texture(direct_name);
			if (t == nullptr)
			{
				cout << "Could not find texture : " << direct_name << " for material : " << name_ << endl;
			}

			textures_[i] = t;
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

D3DXVECTOR4 Material::get_diffuse_color() const
{
	return diffuse_color_;
}

