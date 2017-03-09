
#include <assert.h>

#define TINYOBJLOADER_IMPLEMENTATION

#include "OBJSceneImporter.h"
#include "Mesh.h"
#include "Material.h"
#include "Light.h"
#include "ResourceManager.h"
#include "Utilities.h"
#include "Camera.h"
#include "MeshGroup.h"

OBJSceneImporter::OBJSceneImporter(std::string base_path, std::string file_name)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	//timerutil t;
	//t.start();
	std::string err;
	string full_path = base_path + "/" + file_name;
	string mth_base_path = base_path + "/";
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, full_path.c_str(),
		mth_base_path.c_str(), true);
	//t.end();
	//printf("Parsing time: %lu [msecs]\n", t.msec());

	if (!err.empty())
	{
		std::cerr << err << std::endl;
	}

	scene_to_fill = new Scene(Utilities::get_file_name_from_path_wo_extension(full_path));
	resource_manager.add_scene(scene_to_fill);

	std::vector<Material*> obj_materials;

	for (int i = 0; i < materials.size(); i++)
	{
		tinyobj::material_t &material = materials[i];

		std::string texture_names[3];
		
		texture_names[Material::mtt_diffuse] = material.diffuse_texname; Utilities::get_file_name_from_path(material.diffuse_texname);
		texture_names[Material::mtt_specular] = material.specular_texname; Utilities::get_file_name_from_path(material.specular_texname);
		texture_names[Material::mtt_normal] = material.bump_texname; Utilities::get_file_name_from_path(material.bump_texname);

		D3DXVECTOR4 diffuse_color;
		diffuse_color.x = static_cast<const double>(materials[i].diffuse[0]);
		diffuse_color.y = static_cast<const double>(materials[i].diffuse[1]);
		diffuse_color.z = static_cast<const double>(materials[i].diffuse[2]);
		diffuse_color.w = 1;

		Material *new_material = new Material();
		new_material->create_from_file(texture_names, diffuse_color);

		obj_materials.push_back(new_material);
	}

	int a = 5;

	for (int i = 0; i < shapes.size(); i++)
	{
		tinyobj::shape_t &cur_shape = shapes[i];
		Mesh *new_mesh = new Mesh();
		new_mesh->set_name(cur_shape.name.c_str());

		vector<Mesh::Vertex> vertices;
		vector<int> indices;

		int material_index = cur_shape.mesh.material_ids[0];

		size_t index_offset = 0;
		for (size_t f = 0; f < shapes[i].mesh.num_face_vertices.size(); f++)
		{
			size_t fnum = shapes[i].mesh.num_face_vertices[f];

			printf("  face[%ld].fnum = %ld\n", static_cast<long>(f),
				static_cast<unsigned long>(fnum));

			// For each vertex in the face
			for (size_t v = 0; v < fnum; v++) 
			{
				tinyobj::index_t idx = shapes[i].mesh.indices[index_offset + v];
				int vertex_index = idx.vertex_index;
				int normal_index = idx.normal_index;
				int texcoord_index = idx.texcoord_index;

				Mesh::Vertex new_vertex;
				new_vertex.position.x = attrib.vertices[vertex_index * 3 + 0];
				new_vertex.position.y = attrib.vertices[vertex_index * 3 + 1];
				new_vertex.position.z = attrib.vertices[vertex_index * 3 + 2];
				new_vertex.position.w = 1;

				new_vertex.normal.x = attrib.normals[normal_index * 3 + 0];
				new_vertex.normal.y = attrib.normals[normal_index * 3 + 1];
				new_vertex.normal.z = attrib.normals[normal_index * 3 + 2];
				new_vertex.normal.w = 0;

				new_vertex.texture_coord.x = attrib.texcoords[texcoord_index * 2 + 0];
				new_vertex.texture_coord.y = attrib.texcoords[texcoord_index * 2 + 1];
				new_vertex.texture_coord.z = 0;
				new_vertex.texture_coord.w = 0;

				vertices.push_back(new_vertex);
				indices.push_back(vertices.size() - 1);
			}

			index_offset += fnum;
		}

		new_mesh->create_from_buffers(vertices, indices);
		new_mesh->validate_bounding_box();

		new_mesh->set_material(obj_materials[material_index]);

		scene_to_fill->add_mesh(new_mesh);
		meshes_.push_back(new_mesh);
	}


}

const vector<Mesh*> OBJSceneImporter::get_scene_meshes() const
{
	return meshes_;
}

const vector<MeshGroup*> OBJSceneImporter::get_scene_mesh_groups() const
{
	return vector<MeshGroup*>();
}
