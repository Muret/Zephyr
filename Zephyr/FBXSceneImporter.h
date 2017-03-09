#ifndef _FBX_IMPORTER_H
#define _FBX_IMPORTER_H

#include "includes.h"
#include <fbxsdk.h>

class Mesh;
class Material;
class Scene;
class MeshGroup;

class FBXSceneImporter
{
public:

	FBXSceneImporter(std::string file_name);

	//read functions
	void read_node(FbxNode* pNode);
	Mesh* read_mesh(FbxNode *pNode, FbxMesh* pMesh);
	void read_light(FbxNode *pNode, FbxLight* pMesh);
	void read_camera(FbxNode *pNode, FbxCamera* pCamera);
	void read_lod_group(FbxNode *pNode, FbxLODGroup *pLodGroup);

	void get_transformation_matrix(FbxNode * pNode, Mesh * new_mesh);

	Material* read_material(FbxNode *pNode, FbxSurfaceMaterial* material);

	//debug functions
	void PrintNode(FbxNode* pNode);
	void PrintTabs();
	FbxString GetAttributeTypeName(FbxNodeAttribute::EType type);
	void PrintAttribute(FbxNodeAttribute* pAttribute);

	const vector<Mesh*> get_scene_meshes() const;
	const vector<MeshGroup*> get_scene_mesh_groups() const;

private:

	FbxManager* lSdkManager;

	int numTabs = 0;

	std::ofstream myfile;
	FbxScene* lScene;

	Scene *scene_to_fill;
};

#endif