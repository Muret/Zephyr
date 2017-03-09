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

	enforced_gbuffer_shader = nullptr;
}

void Material::create_from_file(std::string texture_names[mtt_count], const D3DXVECTOR4 &diffuse_color)
{
	diffuse_color_ = diffuse_color;

	for (int i = 0; i < mtt_count; i++)
	{
		texture_names_[i] = texture_names[i];

		if (texture_names_[i] != "")
		{
			string direct_name = Utilities::get_file_name_from_path_wo_extension(texture_names_[i]);
			Texture *t = resource_manager.get_texture(direct_name);
			if (t == nullptr)
			{
				cout << "Could not find texture : " << direct_name << " for material : " << name_ << endl;
			}
			else
			{
				int a = 5;
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
		SetSRV(&srv, 1, shaderType::pixel, 0);
	}

	if (textures_[mtt_normal] != nullptr)
	{
		ID3D11ShaderResourceView *srv = textures_[mtt_normal]->get_srv();
		SetSRV(&srv, 1, shaderType::pixel, 1);
	}

	if (textures_[mtt_specular] != nullptr)
	{
		ID3D11ShaderResourceView *srv = textures_[mtt_specular]->get_srv();
		SetSRV(&srv, 1, shaderType::pixel, 2);
	}

	SetSamplerState();

}

D3DXVECTOR4 Material::get_diffuse_color() const
{
	return diffuse_color_;
}

void Material::write_to_file(ofstream & file)
{
	int name_length = name_.length();
	file.write((char*)&name_length, sizeof(int));
	file.write((char*)name_.c_str(), name_length * sizeof(char));

	for (int i = 0; i < mtt_count; i++)
	{
		int texture_name_length = texture_names_[i].length();
		file.write((char*)&texture_name_length, sizeof(int));
		file.write((char*)texture_names_[i].c_str(), texture_name_length * sizeof(char));
	}

	float diffuse_x, diffuse_y, diffuse_z, diffuse_w;
	diffuse_x = diffuse_color_.x;
	diffuse_y = diffuse_color_.y;
	diffuse_z = diffuse_color_.z;
	diffuse_w = diffuse_color_.w;
	file.write((char*)&diffuse_x, sizeof(float));
	file.write((char*)&diffuse_y, sizeof(float));
	file.write((char*)&diffuse_z, sizeof(float));
	file.write((char*)&diffuse_w, sizeof(float));

}

void Material::read_from_file(ifstream & file)
{
	string texture_names[mtt_count];
	D3DXVECTOR4 diffuse;

	int name_length;
	file.read((char*)&name_length, sizeof(int));
	name_.reserve(name_length + 2);
	memset((char*)name_.c_str(), 0, name_length + 1);
	file.read((char*)name_.c_str(), name_length * sizeof(char));

	for (int i = 0; i < mtt_count; i++)
	{
		int texture_name_length;
		file.read((char*)&texture_name_length, sizeof(int));

		if (texture_name_length > 0)
		{
			int a = 5;
		}

		texture_names[i].resize(texture_name_length + 2);
		memset((char*)texture_names[i].c_str(), 0 , texture_name_length + 2);
		file.read((char*)texture_names[i].c_str(), texture_name_length * sizeof(char));
	}

	float diffuse_x, diffuse_y, diffuse_z, diffuse_w;
	diffuse_x = diffuse_color_.x;
	diffuse_y = diffuse_color_.y;
	diffuse_z = diffuse_color_.z;
	diffuse_w = diffuse_color_.w;
	file.read((char*)&diffuse_x, sizeof(float));
	file.read((char*)&diffuse_y, sizeof(float));
	file.read((char*)&diffuse_z, sizeof(float));
	file.read((char*)&diffuse_w, sizeof(float));

	diffuse.x = diffuse_x;
	diffuse.y = diffuse_y;
	diffuse.z = diffuse_z;
	diffuse.w = diffuse_w;

	create_from_file(texture_names, diffuse);
}

void Material::set_enforced_gbuffer_shader(Shader * shader)
{
	enforced_gbuffer_shader = shader;
}

Shader * Material::get_enforced_gbuffer_shader() const
{
	return enforced_gbuffer_shader;
}

