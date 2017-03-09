#ifndef __INCLUDE_TEXTUREQUADTREE_
#define __INCLUDE_TEXTUREQUADTREE_

#include "includes.h"

class Texture;

class TextureQuadTree
{
public:
	struct TreeNode;

	struct Tile
	{
		int size;
		D3DXVECTOR2 start;
		D3DXVECTOR2 normalized_size;
		int tile_location_in_chunk;
		Texture* texture_handle;
		TreeNode* owner_node;
	};

	TextureQuadTree(int atlas_size, int biggest_square_tile_size);
	~TextureQuadTree();

	void create_depth_texture(int atlas_size);

	Tile get_tile(int requested_size);
	Texture* get_atlas_texture() const;
	void return_tile(const Tile &tile);

	ID3D11Texture2D* depth_atlas_texture_;
	ID3D11DepthStencilView* depth_atlas_texture_view_;
	ID3D11ShaderResourceView* depth_atlas_texture_srv_;

private:

	struct TreeNode
	{
		int current_tile_size_;
		D3DXVECTOR2 start_position_;
		bool free_flags_[3];
		TreeNode* child_node_;
	};

	const int atlas_size_;
	const int biggest_tile_size_;
	
	int number_of_chunks_;

	vector<TreeNode*> chunk_roots_;

	TreeNode* construct_tree_node_recursive(D3DXVECTOR2 start_position, int current_size);

	Texture* atlas_texture_;



};


#endif