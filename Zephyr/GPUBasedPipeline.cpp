
#include "GPUBasedPipeline.h"
#include "Mesh.h"
#include "ResourceManager.h"
#include "Texture.h"
#include "GPUBuffer.h"

GPUBasedPipeline::MeshInstance::MeshInstance()
{
}

GPUBasedPipeline::MeshInstance::~MeshInstance()
{
}

GPUBasedPipeline::MeshSection::MeshSection(BaseMesh *base_mesh) : base_mesh_(base_mesh)
{
}

GPUBasedPipeline::MeshSection::~MeshSection()
{
}

inline void GPUBasedPipeline::MeshSection::set_indices(const std::vector<int>& ind)
{
	indices_ = ind;
}

BoundingBox GPUBasedPipeline::MeshSection::get_bounding_box() const
{
	BoundingBox bb;

	const std::vector<Mesh::Vertex> &vertices = base_mesh_->get_original_mesh()->get_vertices();
	for (int i = 0; i < indices_.size(); i++)
	{
		bb.enlarge_bb_with_point(vertices[indices_[i]].position);
	}

	return bb;
}

GPUBasedPipeline::GPUBasedRenderer::GPUBasedRenderer()
{

}

GPUBasedPipeline::GPUBasedRenderer::~GPUBasedRenderer()
{
}

void GPUBasedPipeline::GPUBasedRenderer::set_scene(Scene * scene)
{
	Mesh *first_mesh = cur_frame_rendered_meshes_[0].mesh;
	BaseMesh *base_mesh = pre_compute_mesh_sections(first_mesh);

	base_meshes_.push_back(base_mesh);

	srand(0);
	const int number_of_spheres = 1024;
	float max_coord = 50;
	
	for (int i = 0; i < number_of_spheres; i++)
	{
		float r1 = ((double)rand() / (RAND_MAX)) * 2.0f - 1.0f;
		float r2 = ((double)rand() / (RAND_MAX)) * 2.0f - 1.0f;
		float r3 = ((double)rand() / (RAND_MAX)) * 2.0f - 1.0f;

		D3DXMATRIX new_frame;
		D3DXMatrixIdentity(&new_frame);
		new_frame.m[3][0] = r1 * max_coord;
		new_frame.m[3][1] = r2 * max_coord;
		new_frame.m[3][2] = r3 * max_coord;

		MeshInstance *new_instance = new MeshInstance();
		new_instance->set_mesh(base_mesh);
		new_instance->set_frame(new_frame);

		add_instance(new_instance);
	}

	//fill vertex buffer and index buffers
	{
		std::vector<Mesh::Vertex> global_vertices;
		std::vector<int> global_indices;
		for (int i = 0; i < base_meshes_.size(); i++)
		{
			BaseMesh *cur_base_mesh = base_meshes_[i];
			const std::vector<Mesh::Vertex> &vertices = cur_base_mesh->get_original_mesh()->get_vertices();
			cur_base_mesh->vertex_buffer_start_ = global_vertices.size();
			global_vertices.insert(global_vertices.end(), vertices.begin(), vertices.end());

			int start_of_mesh_sections = section_infos_.size();
			int mesh_section_count = 0;

			for (int section_id = 0; section_id < cur_base_mesh->sections.size(); section_id++)
			{
				MeshSection *cur_section = cur_base_mesh->sections[section_id];
				cur_section->index_buffer_start_index_ = global_indices.size();
				const std::vector<int> &cur_indices = cur_section->get_indices();
				global_indices.insert(global_indices.end(), cur_indices.begin(), cur_indices.end());

				SectionInfo new_section_info;
				BoundingBox section_bb = cur_section->get_bounding_box();
				new_section_info.bb_max = section_bb.get_max();
				new_section_info.bb_min = section_bb.get_min();;
				new_section_info.number_of_indices = cur_indices.size();
				new_section_info.indices_start_index = cur_section->index_buffer_start_index_;
				new_section_info.vertices_start_index = cur_base_mesh->vertex_buffer_start_;
				
				cur_section->section_info_index_ = section_infos_.size();
				section_infos_.push_back(new_section_info);
				mesh_section_count++;
			}

			MeshInfo new_mesh_info;
			const BoundingBox &mesh_bb = cur_base_mesh->get_original_mesh()->get_bb();
			new_mesh_info.bb_max_section_start = mesh_bb.get_max();
			new_mesh_info.bb_min_section_count = mesh_bb.get_min();
			new_mesh_info.bb_max_section_start[3] = start_of_mesh_sections;
			new_mesh_info.bb_min_section_count[3] = mesh_section_count;
			mesh_infos_.push_back(new_mesh_info);

		}

		global_vertex_buffer_ = new GPUBuffer(sizeof(Mesh::Vertex), global_vertices.size(), &global_vertices[0], (UINT)CreationFlags::vertex_buffer);
		global_index_buffer_ = new GPUBuffer(sizeof(int), global_indices.size(), &global_indices[0], (UINT)CreationFlags::index_buffer);
		section_info_buffer_ = new GPUBuffer(sizeof(SectionInfo), section_infos_.size(), &section_infos_[0], 0);
		mesh_info_buffer_ = new GPUBuffer(sizeof(MeshInfo), mesh_infos_.size(), &mesh_infos_[0], 0);

		InstanceInfo *mesh_instance_info = new InstanceInfo[max_number_of_mesh_instances];
		memset(mesh_instance_info, 0, sizeof(InstanceInfo) * max_number_of_mesh_instances);

		for (int i = 0; i < mesh_instances_.size(); i++)
		{
			mesh_instance_info[i].frame = mesh_instances_[i]->get_frame();
			int base_mesh_index = -1;

			BaseMesh *cur_instance_base_mesh = mesh_instances_[i]->get_base_mesh();
			for (int j = 0; j < base_meshes_.size(); j++)
			{
				if (cur_instance_base_mesh == base_meshes_[j])
				{
					base_mesh_index = j;
				}
			}

			if (base_mesh_index != -1)
			{
				mesh_instance_info[i].frame(3,3) = base_mesh_index;
			}
			else
			{
				ZEPHYR_ASSERT(false);
			}

		}

		mesh_instance_buffer_ = new GPUBuffer(sizeof(InstanceInfo), max_number_of_mesh_instances, mesh_instance_info, 0);
		section_instance_buffer_ = new GPUBuffer(sizeof(InstanceInfo), max_number_of_section_instances, nullptr, (UINT)CreationFlags::structured_buffer);
	}


}

void GPUBasedPipeline::GPUBasedRenderer::tick(float dt)
{
}

inline void GPUBasedPipeline::GPUBasedRenderer::gbuffer_render()
{
	invalidate_srv(shaderType::pixel);

	gbuffer_albedo_texture->set_as_render_target(0);
	gbuffer_normal_texture->set_as_render_target(1);
	gbuffer_specular_texture->set_as_render_target(2);
	clearScreen(D3DXVECTOR4(0, 0, 0, 0));
	ClearRenderView(D3DXVECTOR4(0, 0, 0, 0), 1);
	ClearRenderView(D3DXVECTOR4(0, 0, 0, 0), 2);

	SetDepthState(depth_state_enable_test_enable_write);

	SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	

	//compute shader to cull per section
	{

	}

	//compute shader to cull per tri
	{

	}

	//compute shader for index buffer compaction
	{

	}

	//set shader
	//draw indirect

	SetRenderTargetView(nullptr, 1);
	SetRenderTargetView(nullptr, 2);

}

inline void GPUBasedPipeline::GPUBasedRenderer::add_instance(MeshInstance * instance)
{
	mesh_instances_.push_back(instance);
}

int pre_compute_mesh_sections_cur_sort_index = 0;
bool pre_compute_mesh_sections_sorter(D3DXVECTOR4 i, D3DXVECTOR4 j)
{
	return i[pre_compute_mesh_sections_cur_sort_index] > j[pre_compute_mesh_sections_cur_sort_index];
};


inline GPUBasedPipeline::BaseMesh* GPUBasedPipeline::GPUBasedRenderer::pre_compute_mesh_sections(Mesh * mesh)
{
	std::function<void (const std::vector<D3DXVECTOR4> indices, Mesh *mesh, BaseMesh *base_mesh)> subdivide_space = [&subdivide_space](const std::vector<D3DXVECTOR4> indices, Mesh *mesh, BaseMesh *base_mesh)
	{
		if (indices.size() < 256)
		{
			MeshSection *section = new MeshSection(base_mesh);
			base_mesh->add_mesh_section(section);

			std::vector<int> section_indices;
			for (int i = 0; i < indices.size(); i++)
			{
				section_indices.push_back(mesh->get_indices()[i * 3 + 0]);
				section_indices.push_back(mesh->get_indices()[i * 3 + 1]);
				section_indices.push_back(mesh->get_indices()[i * 3 + 2]);
			}
			section->set_indices(section_indices);
			return;
		}

		BoundingBox bb;
		for (int i = 0; i < indices.size(); i++)
		{
			bb.enlarge_bb_with_point(indices[i]);
		}

		D3DXVECTOR4 bb_length = bb.get_max() - bb.get_min();
		if (bb_length.x > bb_length.y && bb_length.x > bb_length.z)
		{
			pre_compute_mesh_sections_cur_sort_index = 0;
		}
		else if (bb_length.y > bb_length.x && bb_length.y > bb_length.z)
		{
			pre_compute_mesh_sections_cur_sort_index = 1;
		}
		else
		{
			pre_compute_mesh_sections_cur_sort_index = 2;
		}

		std::vector<D3DXVECTOR4> sorted_list, first_part, second_part;
		sorted_list.insert(sorted_list.begin(), indices.begin(), indices.end());
		std::sort(sorted_list.begin(), sorted_list.end(), pre_compute_mesh_sections_sorter);

		first_part.insert(first_part.begin(), sorted_list.begin(), sorted_list.begin() + sorted_list.size() * 0.5f);
		second_part.insert(second_part.begin(), sorted_list.begin() + sorted_list.size() * 0.5f, sorted_list.end());

		subdivide_space(first_part, mesh, base_mesh);
		subdivide_space(second_part, mesh, base_mesh);
	};

	BaseMesh *base_mesh = new BaseMesh(mesh);

	std::vector<D3DXVECTOR4> sorted_tris;
	const std::vector<int>& indices = mesh->get_indices();
	for (int i = 0; i < indices.size() / 3; i++)
	{
		D3DXVECTOR4 pos_index = mesh->get_vertices()[indices[i * 3 + 0]].position + mesh->get_vertices()[indices[i * 3 + 1]].position + mesh->get_vertices()[indices[i * 3 + 2]].position;
		pos_index = pos_index / 3.0f;
		pos_index.w = i;
		sorted_tris.push_back(pos_index);
	}
	
	subdivide_space(sorted_tris, mesh, base_mesh);

	return base_mesh;
}


GPUBasedPipeline::BaseMesh::BaseMesh(Mesh *m)
{
	original_mesh = m;
}

GPUBasedPipeline::BaseMesh::~BaseMesh()
{
}

void GPUBasedPipeline::BaseMesh::add_mesh_section(MeshSection * s)
{
	sections.push_back(s);
}

inline Mesh * GPUBasedPipeline::BaseMesh::get_original_mesh() const
{
	return original_mesh;
}
