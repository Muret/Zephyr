
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

	std::vector<Mesh*> &vector_to_fill;
};