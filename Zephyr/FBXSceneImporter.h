#ifndef _FBX_IMPORTER_H
#define _FBX_IMPORTER_H

#include "includes.h"
#include <fbxsdk.h>

class Mesh;
class Material;
class Scene;

class FBXSceneImporter
{
public:

	FBXSceneImporter(std::string file_name);

	//read functions
	void read_node(FbxNode* pNode);
	void read_mesh(FbxNode *pNode, FbxMesh* pMesh);
	void read_light(FbxNode *pNode, FbxLight* pMesh);

	void get_transformation_matrix(FbxNode * pNode, Mesh * new_mesh);

	Material* read_material(FbxNode *pNode, FbxSurfaceMaterial* material);

	//debug functions
	void PrintNode(FbxNode* pNode);
	void PrintTabs();
	FbxString GetAttributeTypeName(FbxNodeAttribute::EType type);
	void PrintAttribute(FbxNodeAttribute* pAttribute);

	const vector<Mesh*> get_scene_meshes() const;

private:

	FbxManager* lSdkManager;

	int numTabs = 0;

	std::ofstream myfile;
	FbxScene* lScene;

	Scene *scene_to_fill;
};

#endif