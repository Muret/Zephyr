
#include "TextureQuadTree.h"
#include "Texture.h"

TextureQuadTree::TextureQuadTree(int atlas_size, int biggest_square_tile_size) : 
	atlas_size_(atlas_size) , biggest_tile_size_(biggest_square_tile_size)
{
	int chunk_size = biggest_square_tile_size * 2;
	number_of_chunks_ = atlas_size / chunk_size;
	float size_fraction = 1.0f / (float)number_of_chunks_;

	for (int chunk_i = 0; chunk_i < number_of_chunks_; chunk_i++)
	{
		for (int chunk_j = 0; chunk_j < number_of_chunks_; chunk_j++)
		{
			TreeNode * new_chunk = construct_tree_node_recursive(D3DXVECTOR2(chunk_i * size_fraction, chunk_j * size_fraction), chunk_size);
			chunk_roots_.push_back(new_chunk);
		}
	}

	atlas_texture_ = new Texture(D3DXVECTOR3(atlas_size_, atlas_size_, 1), nullptr, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 1);

	create_depth_texture(atlas_size_);
}

TextureQuadTree::~TextureQuadTree()
{
	//TODO_MURAT000
}

void TextureQuadTree::create_depth_texture(int atlas_size)
{
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	// Initialize the description of the depth buffer.
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// Set up the description of the depth buffer.
	depthBufferDesc.Width = atlas_size;
	depthBufferDesc.Height = atlas_size;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	// Create the texture for the depth buffer using the filled out description.
	HRESULT result = g_device->CreateTexture2D(&depthBufferDesc, NULL, &depth_atlas_texture_);
	depth_atlas_texture_srv_ = CreateTextureResourceView(depth_atlas_texture_, DXGI_FORMAT_R32_FLOAT, 0, 1, D3D_SRV_DIMENSION_TEXTURE2D);

	// Set up the depth stencil view description.
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	// Create the depth stencil view.
	result = g_device->CreateDepthStencilView(depth_atlas_texture_, &depthStencilViewDesc, &depth_atlas_texture_view_);
}

TextureQuadTree::Tile TextureQuadTree::get_tile(int requested_size)
{
	float normalized_size_local = (float)requested_size / (float)atlas_size_;
	for (int i = 0; i < chunk_roots_.size(); i++)
	{
		int double_requested_size = requested_size * 2;
		TreeNode* current_node = chunk_roots_[i];
		while (current_node && current_node->current_tile_size_ != double_requested_size)
		{
			current_node = current_node->child_node_;
		}

		if(current_node)
		{
			for (int j = 0; j < 3; j++)
			{
				if (current_node->free_flags_[j])
				{
					Tile new_tile;

					new_tile.start = current_node->start_position_ + D3DXVECTOR2((j % 2) * normalized_size_local , (j >> 1) * normalized_size_local);
					current_node->free_flags_[j] = false;
					new_tile.normalized_size = D3DXVECTOR2(normalized_size_local, normalized_size_local);
					new_tile.tile_location_in_chunk = j;
					new_tile.texture_handle = atlas_texture_;
					new_tile.size = requested_size;
					new_tile.owner_node = current_node;
					return new_tile;
				}
			}
		}
	}
	
	Tile tile;
	tile.owner_node = nullptr;
	tile.size = 0;
	return tile;
}

Texture * TextureQuadTree::get_atlas_texture() const
{
	return atlas_texture_;
}

void TextureQuadTree::return_tile(const Tile & tile)
{
	if (tile.owner_node && tile.tile_location_in_chunk != -1)
	{
		tile.owner_node->free_flags_[tile.tile_location_in_chunk] = true;
	}
}

TextureQuadTree::TreeNode * TextureQuadTree::construct_tree_node_recursive(D3DXVECTOR2 start_position, int current_size)
{
	float size_fraction = current_size / atlas_size_;

	TreeNode *new_chunk = new TreeNode();
	new_chunk->current_tile_size_ = current_size;
	new_chunk->start_position_ = start_position;
	new_chunk->free_flags_[0] = true;
	new_chunk->free_flags_[1] = true;
	new_chunk->free_flags_[2] = true;

	if (current_size > 1)
	{
		new_chunk->child_node_ = construct_tree_node_recursive(start_position + D3DXVECTOR2(size_fraction, size_fraction), current_size / 2);
	}
	else
	{
		new_chunk->child_node_ = nullptr;
	}

	return new_chunk;
}
