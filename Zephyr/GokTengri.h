#ifndef __INCLUDE_GOKTENGRI_
#define __INCLUDE_GOKTENGRI_

#include "includes.h"
#include "Renderer.h"

class Mesh;
class GPUBuffer;
class Scene;

namespace Tengri
{
	class BaseMesh;
	class MeshSection;

	//BROAD_TODO_LIST
	// [DONE]create batches, instances and section instances
	// render everything with drawindirect, without performance :)
	// per cluster visibility checks
	//  - broad frustum checks
	//  - coarse cone visibility check
	//  - hi-z depth culling (can be done later)
	// clip 0 sized draw calls , compact live ones
	// per-triangle visibility checks
	//  - orientation and zero area culling
	//  - depth culling
	//  - small primitive culling
	//  - frustum culling
	//  - compact triangles

	class MeshInstance
	{
	public:
		MeshInstance();
		~MeshInstance();

		void set_mesh(BaseMesh* b)
		{
			mesh = b;
		}

		BaseMesh* get_base_mesh() const
		{
			return mesh;
		}

		void set_frame(const D3DXMATRIX  &m)
		{
			frame = m;
		}

		const D3DXMATRIX& get_frame() const
		{
			return frame;
		}

	private:
		BaseMesh* mesh;
		D3DXMATRIX frame;
	};


	class BaseMesh
	{
	public:
		BaseMesh(Mesh *m);
		~BaseMesh();

		void add_mesh_section(MeshSection* s);
		Mesh* get_original_mesh() const;

		int vertex_buffer_start_;
		Mesh *original_mesh;
		vector<MeshSection*> sections;

	};

	class MeshSection
	{
	public:
		MeshSection(BaseMesh *base_mesh);
		~MeshSection();

		void set_indices(const std::vector<int> &ind);
		int index_buffer_start_index_;
		int section_info_index_;

		const std::vector<int> &get_indices() const
		{
			return indices_;
		}

		BaseMesh *get_base_mesh() const
		{
			return base_mesh_;
		}

		BoundingBox get_bounding_box() const;

	private:
		std::vector<int> indices_;
		BaseMesh *base_mesh_;
	};

	class GreyWolf : public Renderer
	{
	public:
		GreyWolf();
		~GreyWolf();

		void set_scene(Scene *scene);
		void tick(float dt);

		virtual void gbuffer_render() override;

		void add_instance(MeshInstance* instance);

	private:
		BaseMesh * pre_compute_mesh_sections(Mesh *mesh);

		struct InstanceInfo
		{
			D3DXMATRIX frame;
		};

		struct MeshInfo
		{
			D3DXVECTOR4 bb_max_section_start;
			D3DXVECTOR4 bb_min_section_count;
		};

		struct SectionInfo
		{
			D3DXVECTOR4 bb_max;
			D3DXVECTOR4 bb_min;
			int number_of_indices;
			int indices_start_index;
			int vertices_start_index;
			int temp;
		};

		static const int max_number_of_section_instances = 512 * 1024;
		static const int max_number_of_mesh_instances = 16 * 1024;

		GPUBuffer *global_vertex_buffer_;
		GPUBuffer *global_index_buffer_;
		GPUBuffer *section_info_buffer_;
		GPUBuffer *mesh_info_buffer_;

		GPUBuffer *section_instance_buffer_;
		GPUBuffer *mesh_instance_buffer_;

		std::vector<MeshInfo> mesh_infos_;
		std::vector<SectionInfo> section_infos_;

		std::vector<BaseMesh*> base_meshes_;
		std::vector<MeshInstance*> mesh_instances_;
	};





};




#endif