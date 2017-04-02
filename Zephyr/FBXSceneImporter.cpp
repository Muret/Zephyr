
#include <assert.h>

#include "FBXSceneImporter.h"
#include "Mesh.h"
#include "Material.h"
#include "Light.h"
#include "ResourceManager.h"
#include "Utilities.h"
#include "Camera.h"
#include "MeshGroup.h"

FBXSceneImporter::FBXSceneImporter(std::string file_name)
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

	scene_to_fill = new Scene(Utilities::get_file_name_from_path_wo_extension(file_name));
	resource_manager.add_scene(scene_to_fill);

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
void FBXSceneImporter::PrintNode(FbxNode* pNode)
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

void FBXSceneImporter::PrintTabs()
{
	for (int i = 0; i < numTabs; i++)
		myfile << "\t";
}


FbxString FBXSceneImporter::GetAttributeTypeName(FbxNodeAttribute::EType type)
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

void FBXSceneImporter::PrintAttribute(FbxNodeAttribute* pAttribute)
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

void FBXSceneImporter::read_node(FbxNode* pNode)
{
	for (int i = 0; i < pNode->GetNodeAttributeCount(); i++)
	{
		FbxNodeAttribute* pAttribute = pNode->GetNodeAttributeByIndex(i);
		FbxNodeAttribute::EType attribute_type = pAttribute->GetAttributeType();

		if (attribute_type == FbxNodeAttribute::eLODGroup)
		{
			FbxLODGroup *pLodGroup = (FbxLODGroup*)pAttribute;
			read_lod_group(pNode, pLodGroup);
		}
		else if (attribute_type == FbxNodeAttribute::eMesh)
		{
			FbxMesh* pMesh = (FbxMesh*)pAttribute;
			read_mesh(pNode, pMesh);
		}
		else if (attribute_type == FbxNodeAttribute::eLight)
		{
			FbxLight* pLight = (FbxLight*)pAttribute;
			read_light(pNode, pLight);
		}
		else if (attribute_type == FbxNodeAttribute::eCamera)
		{
			FbxCamera* pCamera = (FbxCamera*)pAttribute;
			read_camera(pNode, pCamera);
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

Mesh* FBXSceneImporter::read_mesh(FbxNode *pNode, FbxMesh* pMesh)
{
	std::vector<Mesh::Vertex> vertices;
	std::vector<int> indices;
	
	//pMesh->GenerateTangentsDataForAllUVSets();

	Mesh *new_mesh = new Mesh();
	
	new_mesh->set_name(pNode->GetName());

	if (new_mesh->get_name() == "Mesh_102")
	{
		int a = 5;
	}

	int polygonCount = pMesh->GetPolygonCount();
	FbxVector4* controlPoints = pMesh->GetControlPoints();
	int controlPointCount = pMesh->GetControlPointsCount();

	int vertexID = 0;

	FbxStringList UVSetNameList;

	// Get the name of each set of UV coords
	pMesh->GetUVSetNames(UVSetNameList);

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
				FbxGeometryElementNormal* geomElementNormal = pMesh->GetElementNormal(uvElement);

				FbxLayerElement::EMappingMode mapMode = geomElementNormal->GetMappingMode();
				FbxLayerElement::EReferenceMode refMode = geomElementNormal->GetReferenceMode();

				FbxVector2 uv;
				bool uv_mapped = false;
				pMesh->GetPolygonVertexUV(polygon, polyVert, UVSetNameList.GetStringAt(0), uv, uv_mapped);

				vertex.texture_coord = D3DXVECTOR4(uv[0], uv[1], 0 , 0);

				//FbxGeometryElementUV* geomElementUV = pMesh->GetElementUV(uvElement);
				//
				//FbxLayerElement::EMappingMode mapMode = geomElementUV->GetMappingMode();
				//FbxLayerElement::EReferenceMode refMode = geomElementUV->GetReferenceMode();
				//
				//if (FbxGeometryElement::eByControlPoint == mapMode)
				//{
				//	switch (geomElementUV->GetReferenceMode())
				//	{
				//	case FbxGeometryElement::eDirect:
				//	{
				//		vertex.texture_coord.x = static_cast<float>(geomElementUV->GetDirectArray().GetAt(ctrlPointIndex).mData[0]);
				//		vertex.texture_coord.y = static_cast<float>(geomElementUV->GetDirectArray().GetAt(ctrlPointIndex).mData[1]);
				//	}
				//	break;
				//
				//	case FbxGeometryElement::eIndexToDirect:
				//	{
				//		int index = geomElementUV->GetIndexArray().GetAt(ctrlPointIndex);
				//		vertex.texture_coord.x = static_cast<float>(geomElementUV->GetDirectArray().GetAt(index).mData[0]);
				//		vertex.texture_coord.y = static_cast<float>(geomElementUV->GetDirectArray().GetAt(index).mData[1]);
				//	}
				//	break;
				//
				//	default:
				//		throw std::exception("Invalid Reference");
				//	}
				//}
				//if (FbxGeometryElement::eByPolygonVertex == mapMode)
				//{
				//	int directIndex = -1;
				//	if (FbxGeometryElement::eDirect == refMode)
				//	{
				//		directIndex = vertexID;
				//	}
				//	else if (FbxGeometryElement::eIndexToDirect == refMode)
				//	{
				//		directIndex = geomElementUV->GetIndexArray().GetAt(vertexID);
				//	}
				//
				//	// If we got an index
				//	if (directIndex != -1)
				//	{
				//		FbxVector4 texture_coord = geomElementUV->GetDirectArray().GetAt(directIndex);
				//
				//		vertex.texture_coord = D3DXVECTOR4((float)texture_coord.mData[0], (float)texture_coord.mData[1], 0, 0);
				//	}
				//}
			}

			// Grab normals
			int normElementCount = pMesh->GetElementNormalCount();

			for (int normalElement = 0; normalElement < normElementCount; normalElement++)
			{
				FbxGeometryElementNormal* geomElementNormal = pMesh->GetElementNormal(normalElement);

				FbxLayerElement::EMappingMode mapMode = geomElementNormal->GetMappingMode();
				FbxLayerElement::EReferenceMode refMode = geomElementNormal->GetReferenceMode();

				FbxVector4 fbxNormal;
				pMesh->GetPolygonVertexNormal(polygon, polyVert, fbxNormal);
				fbxNormal.Normalize();

				vertex.normal = D3DXVECTOR4(fbxNormal.mData[0], fbxNormal.mData[1], fbxNormal.mData[2], 0);

				//if (FbxGeometryElement::eByControlPoint == mapMode)
				//{ 
				//	switch (geomElementNormal->GetReferenceMode())
				//	{
				//	case FbxGeometryElement::eDirect:
				//	{
				//		vertex.normal.x = static_cast<float>(geomElementNormal->GetDirectArray().GetAt(ctrlPointIndex).mData[0]);
				//		vertex.normal.y = static_cast<float>(geomElementNormal->GetDirectArray().GetAt(ctrlPointIndex).mData[1]);
				//		vertex.normal.z = static_cast<float>(geomElementNormal->GetDirectArray().GetAt(ctrlPointIndex).mData[2]);
				//		D3DXVec4Normalize(&vertex.normal, &vertex.normal);
				//	}
				//	break;
				//
				//	case FbxGeometryElement::eIndexToDirect:
				//	{
				//		int index = geomElementNormal->GetIndexArray().GetAt(ctrlPointIndex);
				//		vertex.normal.x = static_cast<float>(geomElementNormal->GetDirectArray().GetAt(index).mData[0]);
				//		vertex.normal.y = static_cast<float>(geomElementNormal->GetDirectArray().GetAt(index).mData[1]);
				//		vertex.normal.z = static_cast<float>(geomElementNormal->GetDirectArray().GetAt(index).mData[2]);
				//		D3DXVec4Normalize(&vertex.normal, &vertex.normal);
				//	}
				//	break;
				//
				//	default:
				//		throw std::exception("Invalid Reference");
				//	}
				//}
				//if (FbxGeometryElement::eByPolygonVertex == mapMode)
				//{
				//	int directIndex = -1;
				//	if (FbxGeometryElement::eDirect == refMode)
				//	{
				//		directIndex = vertexID;
				//	}
				//	else if (FbxGeometryElement::eIndexToDirect == refMode)
				//	{
				//		directIndex = geomElementNormal->GetIndexArray().GetAt(vertexID);
				//	}
				//
				//	// If we got an index
				//	if (directIndex != -1)
				//	{
				//		FbxVector4 norm = geomElementNormal->GetDirectArray().GetAt(directIndex);
				//
				//		D3DXVECTOR4 normal_final((float)norm.mData[0], (float)norm.mData[1], (float)norm.mData[2], 0);
				//		D3DXVec4Normalize(&vertex.normal, &normal_final);
				//	}
				//}


			}

			// grab tangents
			int tangentElementCount = pMesh->GetElementTangentCount();

			for (int normalElement = 0; normalElement < tangentElementCount; normalElement++)
			{
				FbxGeometryElementTangent* geomElementTangent = pMesh->GetElementTangent(normalElement);

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
			vertices.push_back(vertex);
			indices.push_back(size);

			++vertexID;
		}

		//int cur_size = indices.size();
		//int temp = indices[cur_size - 3];
		//indices[cur_size - 3] = indices[cur_size - 1];
		//indices[cur_size - 1] = temp;
	}

	int materialCount = pNode->GetSrcObjectCount<FbxSurfaceMaterial>();

	new_mesh->create_from_buffers(vertices, indices);
	scene_to_fill->add_mesh(new_mesh);

	if (materialCount > 0)
	{
		FbxSurfaceMaterial* material = (FbxSurfaceMaterial*)pNode->GetSrcObject<FbxSurfaceMaterial>(0);
		new_mesh->set_material(read_material(pNode, material));
	}

	get_transformation_matrix(pNode, new_mesh);
	std::cout << "Read mesh : " << new_mesh->get_name() << "\n";

	return new_mesh;
}

Material* FBXSceneImporter::read_material(FbxNode *pNode, FbxSurfaceMaterial* material)
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

		//diffuse color
		D3DXVECTOR4 diffuse_color = D3DXVECTOR4(1, 1, 1, 1);
		if (material->GetClassId().Is(FbxSurfaceLambert::ClassId))
		{
			FbxSurfaceLambert *lambert_material = (FbxSurfaceLambert *)material;
			FbxDouble3 diffuse = lambert_material->Diffuse.Get();
			diffuse_color = D3DXVECTOR4(diffuse[0], diffuse[1], diffuse[2], 1);
		}

		Material *new_material= new Material;
		new_material->create_from_file(texture_names, diffuse_color);
		return new_material;
	}

	return nullptr;
}

void FBXSceneImporter::get_transformation_matrix(FbxNode * pNode, Mesh * new_mesh)
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

void FBXSceneImporter::read_light(FbxNode *pNode, FbxLight* pLight)
{
	FbxDouble3 color = pLight->Color.Get();
	FbxAMatrix tr_matrix = pNode->EvaluateGlobalTransform();

	Light::LightType cur_type = (pLight->LightType.Get() == FbxLight::eDirectional) ? Light::type_directional : Light::type_pointlight;
	D3DXVECTOR4 position = D3DXVECTOR4(tr_matrix[3][0], tr_matrix[3][1], tr_matrix[3][2], 1);

	D3DXVECTOR4 direction = D3DXVECTOR4(0, -1, 0, 0);
	if (cur_type == Light::type_directional)
	{
		direction = -D3DXVECTOR4(tr_matrix[1][0], tr_matrix[1][1], tr_matrix[1][2], 0);
	}

	D3DXVECTOR4 light_color = D3DXVECTOR4(color[0], color[1], color[2], 1);

	Light *new_light = new Light();
	new_light->create_from_file(pLight->GetName(), light_color, position, direction, cur_type);
	scene_to_fill->add_light(new_light);
}

void FBXSceneImporter::read_camera(FbxNode *pNode, FbxCamera* pCamera)
{
	FbxAMatrix frame = pNode->EvaluateGlobalTransform();
	float near_v = pCamera->GetNearPlane();
	float fov_v = pCamera->ComputeFieldOfView(pCamera->FocalLength);
	float far_v = pCamera->GetFarPlane();
	string name = pCamera->GetName();

	D3DMATRIX cam_frame;
	cam_frame._11 = frame[0][0];
	cam_frame._12 = frame[0][1];
	cam_frame._13 = frame[0][2];
	cam_frame._14 = frame[0][3];

	cam_frame._21 = frame[1][0];
	cam_frame._22 = frame[1][1];
	cam_frame._23 = frame[1][2];
	cam_frame._24 = frame[1][3];

	cam_frame._31 = frame[2][0];
	cam_frame._32 = frame[2][1];
	cam_frame._33 = frame[2][2];
	cam_frame._34 = frame[2][3];

	cam_frame._41 = frame[3][0];
	cam_frame._42 = frame[3][1];
	cam_frame._43 = frame[3][2];
	cam_frame._44 = frame[3][3];

	Camera *new_camera = new Camera();
	new_camera->set_frame(cam_frame);
	new_camera->set_near(near_v);
	new_camera->set_far(far_v);
	new_camera->set_fov(fov_v);
	new_camera->set_name(name);

	scene_to_fill->add_camera(new_camera);
}

void FBXSceneImporter::read_lod_group(FbxNode * pNode, FbxLODGroup * pLodGroup)
{
	const char* nodeName = pLodGroup->GetName();
	MeshGroup *m_group = new MeshGroup(nodeName);

	int mesh_count = 0;

	for (int i = 0; i < pNode->GetChildCount(); i++)
	{
		FbxNode* lChildNode = pNode->GetChild(i);
		for (int i = 0; i < pNode->GetNodeAttributeCount(); i++)
		{
			FbxNodeAttribute* pAttribute = lChildNode->GetNodeAttributeByIndex(i);
			FbxNodeAttribute::EType attribute_type = pAttribute->GetAttributeType();

			if (attribute_type == FbxNodeAttribute::eMesh)
			{
				FbxMesh* pMesh = (FbxMesh*)pAttribute;
				Mesh * mesh = read_mesh(lChildNode, pMesh);

				float distance = 0;
				if (mesh_count > 0)
				{
					FbxDistance lThreshVal;
					pLodGroup->GetThreshold(mesh_count - 1, lThreshVal);
					distance = lThreshVal.value();
				}

				m_group->add_lod_mesh_with_start_distance(mesh, distance);
				mesh_count++;
			}
		}
	}

	scene_to_fill->add_mesh_group(m_group);
}

const vector<Mesh*> FBXSceneImporter::get_scene_meshes() const
{
	return scene_to_fill->get_meshes();
}

const vector<MeshGroup*> FBXSceneImporter::get_scene_mesh_groups() const
{
	return scene_to_fill->get_mesh_groups();
}
