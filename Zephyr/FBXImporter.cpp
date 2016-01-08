
#include <assert.h>

#include "FBXImporter.h"
#include "Mesh.h"
#include "Material.h"

FBXImporter::FBXImporter(std::string file_name, vector<Mesh*> &meshes) : vector_to_fill(meshes)
{
	std::string log_file_name = file_name;
	log_file_name.append("log.txt");
	myfile.open(log_file_name.c_str());

	// Initialize the SDK manager. This object handles memory management.
	lSdkManager = FbxManager::Create();

	FbxIOSettings *ios = FbxIOSettings::Create(lSdkManager, IOSROOT);
	lSdkManager->SetIOSettings(ios);

	// Create an importer using the SDK manager.
	FbxImporter* lImporter = FbxImporter::Create(lSdkManager, "");

	// Use the first argument as the filename for the importer.
	if (!lImporter->Initialize(file_name.c_str(), -1, lSdkManager->GetIOSettings())) {
		myfile << "Call to FbxImporter::Initialize() failed.\n";
		myfile << "Error returned: " <<  lImporter->GetStatus().GetErrorString() << "";
		myfile.close();
		exit(-1);
	}


	// Create a new scene so that it can be populated by the imported file.
	lScene = FbxScene::Create(lSdkManager, "myScene");

	// Import the contents of the file into the scene.
	lImporter->Import(lScene);

	// The file is imported, so get rid of the importer.
	lImporter->Destroy();


	FbxGeometryConverter converter(lSdkManager);
	converter.Triangulate(lScene, true);


	// Print the nodes of the scene and their attributes recursively.
	// Note that we are not printing the root node because it should
	// not contain any attributes.
	FbxNode* lRootNode = lScene->GetRootNode();
	if (lRootNode) 
	{
		for (int i = 0; i < lRootNode->GetChildCount(); i++)
		{
			read_node(lRootNode->GetChild(i));
		}
	}

	if (lRootNode)
	{
		for (int i = 0; i < lRootNode->GetChildCount(); i++)
		{
			PrintNode(lRootNode->GetChild(i));
		}
	}

	// Destroy the SDK manager and all the other objects it was handling.
	myfile.close();
	lSdkManager->Destroy();

}


/**
* Print a node, its attributes, and all its children recursively.
*/
void FBXImporter::PrintNode(FbxNode* pNode)
{
	PrintTabs();
	const char* nodeName = pNode->GetName();
	FbxDouble3 translation = pNode->LclTranslation.Get();
	FbxDouble3 rotation = pNode->LclRotation.Get();
	FbxDouble3 scaling = pNode->LclScaling.Get();

	// Print the contents of the node.
	myfile << "<node name= " << nodeName << "\n";

	numTabs++;

	// Print the node's attributes.
	for (int i = 0; i < pNode->GetNodeAttributeCount(); i++)
		PrintAttribute(pNode->GetNodeAttributeByIndex(i));


	// Recursively print the children.
	for (int j = 0; j < pNode->GetChildCount(); j++)
		PrintNode(pNode->GetChild(j));

	numTabs--;
	PrintTabs();
	myfile << "</node>\n" ;
}

void FBXImporter::PrintTabs()
{
	for (int i = 0; i < numTabs; i++)
		myfile << "\t";
}


FbxString FBXImporter::GetAttributeTypeName(FbxNodeAttribute::EType type)
{
	switch (type) 
	{
	case FbxNodeAttribute::eUnknown: return "unidentified";
	case FbxNodeAttribute::eNull: return "null";
	case FbxNodeAttribute::eMarker: return "marker";
	case FbxNodeAttribute::eSkeleton: return "skeleton";
	case FbxNodeAttribute::eMesh: return "mesh";
	case FbxNodeAttribute::eNurbs: return "nurbs";
	case FbxNodeAttribute::ePatch: return "patch";
	case FbxNodeAttribute::eCamera: return "camera";
	case FbxNodeAttribute::eCameraStereo: return "stereo";
	case FbxNodeAttribute::eCameraSwitcher: return "camera switcher";
	case FbxNodeAttribute::eLight: return "light";
	case FbxNodeAttribute::eOpticalReference: return "optical reference";
	case FbxNodeAttribute::eOpticalMarker: return "marker";
	case FbxNodeAttribute::eNurbsCurve: return "nurbs curve";
	case FbxNodeAttribute::eTrimNurbsSurface: return "trim nurbs surface";
	case FbxNodeAttribute::eBoundary: return "boundary";
	case FbxNodeAttribute::eNurbsSurface: return "nurbs surface";
	case FbxNodeAttribute::eShape: return "shape";
	case FbxNodeAttribute::eLODGroup: return "lodgroup";
	case FbxNodeAttribute::eSubDiv: return "subdiv";
	default: return "unknown";
	}
}

void FBXImporter::PrintAttribute(FbxNodeAttribute* pAttribute)
{
	if (!pAttribute)
	{
		return;
	}

	FbxString typeName = GetAttributeTypeName(pAttribute->GetAttributeType());
	FbxString attrName = pAttribute->GetName();
	PrintTabs();
	// Note: to retrieve the character array of a FbxString, use its Buffer() method.
	myfile << "<attribute type= " << typeName.Buffer() << " name= " << attrName.Buffer() << " />\n";
}

void FBXImporter::read_node(FbxNode* pNode)
{
	for (int i = 0; i < pNode->GetNodeAttributeCount(); i++)
	{
		FbxNodeAttribute* pAttribute = pNode->GetNodeAttributeByIndex(i);
		FbxNodeAttribute::EType attribute_type = pAttribute->GetAttributeType();

		if (attribute_type == FbxNodeAttribute::eMesh)
		{
			FbxMesh* pMesh = (FbxMesh*)pAttribute;
			read_mesh(pNode, pMesh);
		}
	}

	int materialCount = pNode->GetSrcObjectCount<FbxSurfaceMaterial>();

	// Recursively print the children.
	for (int j = 0; j < pNode->GetChildCount(); j++)
	{
		read_node(pNode->GetChild(j));
	}

}

//void FBXImporter::read_mesh(FbxNode *pNode, FbxMesh* pMesh)
//{
//	FbxVector4* pVertices = pMesh->GetControlPoints();
//
//	Mesh *new_mesh = new Mesh();
//	std::vector<Mesh::Vertex> vertices;
//
//	FbxGeometryElementUV* uvElement = pMesh->GetElementUV(0);
//
//	FbxLayerElement::EMappingMode mode = uvElement->GetMappingMode();
//
//	const bool lUseIndex = uvElement->GetReferenceMode() != FbxGeometryElement::eDirect;
//	const int lIndexCount = (lUseIndex) ? uvElement->GetIndexArray().GetCount() : 0;
//
//	int lPolyIndexCounter = 0;
//	for (int j = 0; j < pMesh->GetPolygonCount(); j++)
//	{
//		int iNumVertices = pMesh->GetPolygonSize(j);
//		assert(iNumVertices == 3);
//
//		for (int k = 0; k < iNumVertices; k++)
//		{
//			int iControlPointIndex = pMesh->GetPolygonVertex(j, k);
//
//			Mesh::Vertex vertex;
//			vertex.position[0] = (float)pVertices[iControlPointIndex].mData[0];
//			vertex.position[1] = (float)pVertices[iControlPointIndex].mData[1];
//			vertex.position[2] = (float)pVertices[iControlPointIndex].mData[2];
//			vertex.position[3] = 1;
//
//			vertex.color[0] = 1;
//			vertex.color[1] = 1;
//			vertex.color[2] = 1;
//			vertex.color[3] = 1;
//
//			//the UV index depends on the reference mode
//			int lUVIndex = lUseIndex ? uvElement->GetIndexArray().GetAt(lPolyIndexCounter) : lPolyIndexCounter;
//
//			FbxVector2 uv = uvElement->GetDirectArray().GetAt(lUVIndex);
//			vertex.texture_coord[0] = uv[0];
//			vertex.texture_coord[1] = uv[1];
//
//			vertices.push_back(vertex);
//			lPolyIndexCounter++;
//		}
//	}
//
//	//fetch material
//	{
//		FbxGeometryElementMaterial *mesh_material = pMesh->GetElementMaterial(0);
//	}
//
//	//new_mesh->create_from_fbx(vertices);
	//vector_to_fill.push_back(new_mesh);
//}

void FBXImporter::read_mesh(FbxNode *pNode, FbxMesh* pMesh)
{
	std::vector<Mesh::Vertex> vertices;
	std::vector<int> indices;
	
	//pMesh->GenerateTangentsDataForAllUVSets();

	Mesh *new_mesh = new Mesh();
	
	new_mesh->set_name(pNode->GetName());

	int polygonCount = pMesh->GetPolygonCount();
	FbxVector4* controlPoints = pMesh->GetControlPoints();
	int controlPointCount = pMesh->GetControlPointsCount();

	int vertexID = 0;

	for (int polygon = polygonCount - 1; polygon > -1; polygon--)
	{
		int polyVertCount = pMesh->GetPolygonSize(polygon);
		assert(polyVertCount == 3);

		for (int polyVert = 0; polyVert < polyVertCount; polyVert++)
		{
			Mesh::Vertex vertex;

			int cpIndex = pMesh->GetPolygonVertex(polygon, polyVert);

			// Grab our CP index as well our position information
			//uniqueVert.m_nControlPointIndex = cpIndex;
			vertex.position[0] = controlPoints[cpIndex].mData[0];
			vertex.position[1] = controlPoints[cpIndex].mData[1];
			vertex.position[2] = controlPoints[cpIndex].mData[2];
			vertex.position[3] = 1;

			// Grab UVs
			int uvElementCount = pMesh->GetElementUVCount();
			int ctrlPointIndex = pMesh->GetPolygonVertex(polygon, polyVert);

			for (int uvElement = 0; uvElement < uvElementCount; uvElement++)
			{
				FbxGeometryElementUV* geomElementUV = pMesh->GetElementUV(uvElement);

				FbxLayerElement::EMappingMode mapMode = geomElementUV->GetMappingMode();
				FbxLayerElement::EReferenceMode refMode = geomElementUV->GetReferenceMode();

				int directIndex = -1;

				if (FbxGeometryElement::eByControlPoint == mapMode)
				{
					if (FbxGeometryElement::eDirect == refMode)
					{
						directIndex = cpIndex;
					}
					else if (FbxGeometryElement::eIndexToDirect == refMode)
					{
						directIndex = geomElementUV->GetIndexArray().GetAt(cpIndex);
					}
				}
				else if (FbxGeometryElement::eByPolygonVertex == mapMode)
				{
					if (FbxGeometryElement::eDirect == refMode || FbxGeometryElement::eIndexToDirect == refMode)
					{
						directIndex = pMesh->GetTextureUVIndex(polygon, polyVert);
					}

				}

				// If we got a UV index
				if (directIndex != -1)
				{
					FbxVector2 uv = geomElementUV->GetDirectArray().GetAt(directIndex);

					vertex.texture_coord = D3DXVECTOR4((float)uv.mData[0], (float)uv.mData[1], 0 , 0);
				}
				else
				{
					vertex.texture_coord = D3DXVECTOR4(0, 0, 0, 0);
				}
			}

			// Grab normals
			int normElementCount = pMesh->GetElementNormalCount();

			for (int tangentElement = 0; tangentElement < normElementCount; tangentElement++)
			{
				FbxGeometryElementNormal* geomElementNormal = pMesh->GetElementNormal(tangentElement);

				FbxLayerElement::EMappingMode mapMode = geomElementNormal->GetMappingMode();
				FbxLayerElement::EReferenceMode refMode = geomElementNormal->GetReferenceMode();

				int directIndex = -1;

				if (FbxGeometryElement::eByControlPoint == mapMode)
				{ 
					switch (geomElementNormal->GetReferenceMode())
					{
					case FbxGeometryElement::eDirect:
					{
						vertex.normal.x = static_cast<float>(geomElementNormal->GetDirectArray().GetAt(ctrlPointIndex).mData[0]);
						vertex.normal.y = static_cast<float>(geomElementNormal->GetDirectArray().GetAt(ctrlPointIndex).mData[1]);
						vertex.normal.z = static_cast<float>(geomElementNormal->GetDirectArray().GetAt(ctrlPointIndex).mData[2]);
					}
					break;

					case FbxGeometryElement::eIndexToDirect:
					{
						int index = geomElementNormal->GetIndexArray().GetAt(ctrlPointIndex);
						vertex.normal.x = static_cast<float>(geomElementNormal->GetDirectArray().GetAt(index).mData[0]);
						vertex.normal.y = static_cast<float>(geomElementNormal->GetDirectArray().GetAt(index).mData[1]);
						vertex.normal.z = static_cast<float>(geomElementNormal->GetDirectArray().GetAt(index).mData[2]);
					}
					break;

					default:
						throw std::exception("Invalid Reference");
					}
				}
				if (FbxGeometryElement::eByPolygonVertex == mapMode)
				{
					if (FbxGeometryElement::eDirect == refMode)
					{
						directIndex = vertexID;
					}
					else if (FbxGeometryElement::eIndexToDirect == refMode)
					{
						directIndex = geomElementNormal->GetIndexArray().GetAt(vertexID);
					}

					// If we got an index
					if (directIndex != -1)
					{
						FbxVector4 norm = geomElementNormal->GetDirectArray().GetAt(directIndex);

						vertex.normal = D3DXVECTOR4((float)norm.mData[0], (float)norm.mData[1], (float)norm.mData[2], 0);
					}
				}


			}

			// grab tangents
			int tangentElementCount = pMesh->GetElementTangentCount();

			for (int tangentElement = 0; tangentElement < tangentElementCount; tangentElement++)
			{
				FbxGeometryElementTangent* geomElementTangent = pMesh->GetElementTangent(tangentElement);

				FbxLayerElement::EMappingMode mapMode = geomElementTangent->GetMappingMode();
				FbxLayerElement::EReferenceMode refMode = geomElementTangent->GetReferenceMode();

				int directIndex = -1;

				if (FbxGeometryElement::eByPolygonVertex == mapMode)
				{
					if (FbxGeometryElement::eDirect == refMode)
					{
						directIndex = vertexID;
					}
					else if (FbxGeometryElement::eIndexToDirect == refMode)
					{
						directIndex = geomElementTangent->GetIndexArray().GetAt(vertexID);
					}
				}

				// If we got an index
				if (directIndex != 1)
				{
					FbxVector4 tangent = geomElementTangent->GetDirectArray().GetAt(directIndex);

					vertex.tangent = D3DXVECTOR4((float)tangent.mData[0], (float)tangent.mData[1], (float)tangent.mData[2], 0);
				}

			}

			size_t size = vertices.size();
			size_t i;

			//for (i = 0; i < size; i++)
			//{
			//	if (vertex == vertices[i])
			//	{
			//		break;
			//	}
			//}

			//if (i == size)
			{
				vertices.push_back(vertex);
			}

			indices.push_back(size);
			++vertexID;
		}

		int cur_size = indices.size();
		int temp = indices[cur_size - 3];
		indices[cur_size - 3] = indices[cur_size - 1];
		indices[cur_size - 1] = temp;
	}

	int materialCount = pNode->GetSrcObjectCount<FbxSurfaceMaterial>();

	new_mesh->create_from_fbx(vertices, indices);
	vector_to_fill.push_back(new_mesh);

	if (materialCount > 0)
	{
		FbxSurfaceMaterial* material = (FbxSurfaceMaterial*)pNode->GetSrcObject<FbxSurfaceMaterial>(0);

		new_mesh->set_material(read_material(pNode, material));
	}

	get_transformation_matrix(pNode, new_mesh);
	cout << "Read mesh : " << new_mesh->get_name() << "\n";

}


Material* FBXImporter::read_material(FbxNode *pNode, FbxSurfaceMaterial* material)
{
	std::string texture_names[Material::mtt_count];

	if (material != NULL)
	{
		std::string material_name = material->GetName();

		{
			FbxProperty prop = material->FindProperty(FbxSurfaceMaterial::sDiffuse);

			int textureCount = prop.GetSrcObjectCount<FbxTexture>();
			if (textureCount > 0)
			{
				FbxTexture* texture = FbxCast<FbxTexture>(prop.GetSrcObject<FbxTexture>(0));
				// Then, you can get all the properties of the texture, include its name
				const char* textureName = texture->GetName();
				FbxFileTexture *file_texture = (FbxFileTexture *)texture;

				std::string fileName = file_texture->GetFileName();
				texture_names[Material::mtt_diffuse] = fileName; file_texture->GetRelativeFileName();
			}
		}

		{
			FbxProperty prop = material->FindProperty(FbxSurfaceMaterial::sBump);

			int textureCount = prop.GetSrcObjectCount<FbxTexture>();
			if (textureCount > 0)
			{
				FbxTexture* texture = FbxCast<FbxTexture>(prop.GetSrcObject<FbxTexture>(0));
				// Then, you can get all the properties of the texture, include its name
				const char* textureName = texture->GetName();
				FbxFileTexture *file_texture = (FbxFileTexture *)texture;

				std::string fileName = file_texture->GetFileName();
				texture_names[Material::mtt_normal] = fileName; file_texture->GetRelativeFileName();
			}
		}

		{
			FbxProperty prop = material->FindProperty(FbxSurfaceMaterial::sBump);

			int textureCount = prop.GetSrcObjectCount<FbxTexture>();
			if (textureCount > 0)
			{
				FbxTexture* texture = FbxCast<FbxTexture>(prop.GetSrcObject<FbxTexture>(0));
				// Then, you can get all the properties of the texture, include its name
				const char* textureName = texture->GetName();
				FbxFileTexture *file_texture = (FbxFileTexture *)texture;

				std::string fileName = file_texture->GetFileName();
				texture_names[Material::mtt_specular] = fileName; file_texture->GetRelativeFileName();
			}
		}

		Material *new_material= new Material;
		new_material->create_from_file(texture_names);
		return new_material;
	}

	return nullptr;
}

void FBXImporter::get_transformation_matrix(FbxNode * pNode, Mesh * new_mesh)
{
	FbxAMatrix tr_matrix = pNode->EvaluateGlobalTransform();
	D3DXMATRIX matrix;
	{
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				matrix.m[i][j] = tr_matrix[i][j];
			}
		}
	}
	new_mesh->set_frame(matrix);
}
