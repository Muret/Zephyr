
#include "GPUVoxelOcctree.h"
#include "ResourceManager.h"
#include "BoundingBox.h"
#include "Camera.h"
#include "Texture.h"
#include "GPUBuffer.h"
#include "Shader.h"

GPUVoxelOcctree::GPUVoxelOcctree(const D3DXVECTOR3 &resolution)
{
	resolution_ = resolution;

	x_render_texture_ = new Texture(resolution.z, resolution.y, NULL, DXGI_FORMAT_R32G32B32A32_FLOAT);
	y_render_texture_ = new Texture(resolution.x, resolution.z, NULL, DXGI_FORMAT_R32G32B32A32_FLOAT);
	z_render_texture_ = new Texture(resolution.x, resolution.y, NULL, DXGI_FORMAT_R32G32B32A32_FLOAT);

	const int max_requests = resolution.x * resolution.y * resolution.z;
	node_allocate_requests_buffer_ = new GPUBuffer(sizeof(NodeAllocationRequestEntry), max_requests, NULL, 
		GPUBuffer::BufferCreationFlags::structured_buffer | GPUBuffer::BufferCreationFlags::append_consume_buffer);


}

GPUVoxelOcctree::~GPUVoxelOcctree()
{
}

void GPUVoxelOcctree::construct(Scene * scene)
{
	BoundingBox bb = scene->get_bb();

	//render from -x
	{
		D3DXVECTOR4 cam_pos = D3DXVECTOR4(bb.get_min().x - 5, (bb.get_min().y + bb.get_max().y) * 0.5, (bb.get_min().z + bb.get_max().z) * 0.5, 1);

		Camera ortho_cam;
		ortho_cam.set_position(cam_pos);
		ortho_cam.set_directions(D3DXVECTOR4(1, 0, 0, 0), D3DXVECTOR4(0, 1, 0, 0), D3DXVECTOR4(0, 0, 1, 0));
		ortho_cam.set_ortho_params(bb.get_min().z, bb.get_max().z, bb.get_max().y, bb.get_min().y, 4, 4 + bb.get_max().x);

		x_render_texture_->set_as_render_target(0);
	}

	//render from -y
	{
		D3DXVECTOR4 cam_pos = D3DXVECTOR4((bb.get_min().x + bb.get_max().x) * 0.5, bb.get_min().y - 5, (bb.get_min().z + bb.get_max().z) * 0.5, 1);

		Camera ortho_cam;
		ortho_cam.set_position(cam_pos);
		ortho_cam.set_directions(D3DXVECTOR4(0, 1, 0, 0), D3DXVECTOR4(0, 0 , 1, 0), D3DXVECTOR4(1, 0, 0, 0));
		ortho_cam.set_ortho_params(bb.get_min().x, bb.get_max().x, bb.get_max().z, bb.get_min().z, 4, 4 + bb.get_max().y);

		y_render_texture_->set_as_render_target(0);

	}

	//render from -z
	{
		D3DXVECTOR4 cam_pos = D3DXVECTOR4((bb.get_min().x + bb.get_max().x) * 0.5, (bb.get_min().y + bb.get_max().y) * 0.5, bb.get_min().z - 5, 1);

		Camera ortho_cam;
		ortho_cam.set_position(cam_pos);
		ortho_cam.set_directions(D3DXVECTOR4(0, 0, 1, 0), D3DXVECTOR4(0, 1, 0, 0), D3DXVECTOR4(-1, 0, 0, 0));
		ortho_cam.set_ortho_params(bb.get_min().x, bb.get_max().x, bb.get_max().y, bb.get_min().y, 4, 4 + bb.get_max().z);

		z_render_texture_->set_as_render_target(0);
	}

	

}
