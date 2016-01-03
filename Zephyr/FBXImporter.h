#ifndef _FBX_IMPORTER_H
#define _FBX_IMPORTER_H

#include <fbxsdk.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>

class Mesh;
class Material;

class FBXImporter
{
public:

	FBXImporter(std::string file_name, std::vector<Mesh*> &meshes);

	//read functions
	void read_node(FbxNode* pNode);
	void read_mesh(FbxNode *pNode, FbxMesh* pMesh);

	void get_transformation_matrix(FbxNode * pNode, Mesh * new_mesh);

	Material* read_material(FbxNode *pNode, FbxSurfaceMaterial* material);

	//debug functions
	void PrintNode(FbxNode* pNode);
	void PrintTabs();
	FbxString GetAttributeTypeName(FbxNodeAttribute::EType type);
	void PrintAttribute(FbxNodeAttribute* pAttribute);

private:

	FbxManager* lSdkManager;

	int numTabs = 0;

	std::ofstream myfile;
	FbxScene* lScene;

	std::vector<Mesh*> &vector_to_fill;
};

#endif