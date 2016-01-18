//
// Copyright (c) Autodesk, Inc. All rights reserved 
//
// C++ glTF FBX importer/exporter plug-in
// by Cyrille Fauvel - Autodesk Developer Network (ADN)
// January 2015
//
// Permission to use, copy, modify, and distribute this software in
// object code form for any purpose and without fee is hereby granted, 
// provided that the above copyright notice appears in all copies and 
// that both that copyright notice and the limited warranty and
// restricted rights notice below appear in all supporting 
// documentation.
//
// AUTODESK PROVIDES THIS PROGRAM "AS IS" AND WITH ALL FAULTS. 
// AUTODESK SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTY OF
// MERCHANTABILITY OR FITNESS FOR A PARTICULAR USE.  AUTODESK, INC. 
// DOES NOT WARRANT THAT THE OPERATION OF THE PROGRAM WILL BE
// UNINTERRUPTED OR ERROR FREE.
//
#include "StdAfx.h"
#include "gltfWriter.h"
#include <string.h> // for memcmp


namespace _IOglTF_NS_ {

	class gltfwriterVBO {
		struct PackedVertex {
			FbxDouble4 m_position;
			FbxDouble2 m_uv;
			FbxDouble3 m_normal;
			FbxColor   m_vcolors;
			bool operator<(const PackedVertex that) const{
				return memcmp((void*)this, (void*)&that, sizeof(PackedVertex))>0;
			};
		};
		std::vector<unsigned short> m_in_indices, m_out_indices;
		std::vector<FbxDouble4> m_in_positions, m_out_positions;
		std::vector<FbxDouble2> m_in_uvs, m_out_uvs;
		std::vector<FbxDouble3> m_in_normals, m_out_normals;
		std::vector<FbxDouble3> m_in_tangents, m_out_tangents;
		std::vector<FbxDouble3> m_in_binormals, m_out_binormals;
		std::vector<FbxColor>   m_in_vcolors, m_out_vcolors;
		std::map<utility::string_t, utility::string_t> m_uvSets;
		FbxMesh *m_pMesh;

	public:
		gltfwriterVBO(FbxMesh *pMesh) { m_pMesh = pMesh; }
		void GetLayerElements(int layerIndex, bool bInGeometry);
		FbxArray<FbxVector4> GetVertexPositions(bool bInGeometry, bool bExportControlPoints);
		bool getSimilarVertexIndex(PackedVertex & packed, std::map<PackedVertex, unsigned short> & VertexToOutIndex, unsigned short & result);
		void indexVBO();
		std::vector<unsigned short> getIndices()     { return m_out_indices; }
		std::vector<FbxDouble4>     getPositions()   { return m_out_positions; }
		std::vector<FbxDouble2>     getUvs()         { return m_out_uvs; }
		std::vector<FbxDouble3>     getNormals()     { return m_out_normals; }
		std::vector<FbxDouble3>     getTangents()    { return m_out_tangents; }
		std::vector<FbxDouble3>     getBinormals()   { return m_out_binormals; }
		std::vector<FbxColor>       getVertexColors() { return m_out_vcolors; }
		std::map<utility::string_t, utility::string_t> getUvSets() { return m_uvSets; }

	};

	/* ==================================================================================================================
	** Function    : getSimilarVertexIndex
	** Abstraction : Find the Packed Vertex from the existing map, if found return true else return false
	** =================================================================================================================*/
	bool gltfwriterVBO::getSimilarVertexIndex(
		PackedVertex & packed,
		std::map<PackedVertex, unsigned short> & VertexToOutIndex,
		unsigned short & result)
	{
		std::map<PackedVertex, unsigned short>::iterator it = VertexToOutIndex.find(packed);
		if (it == VertexToOutIndex.end()){
			return false;
		}
		else{
			result = it->second;
			return true;
		}
	}

	/* ==================================================================================================================
	** Function    : indexVBO()
	** Abstraction : 1. Pack positions, uvs, normals, vertex colors in to single entity (packedVertex)
	**               2. Search the packed vertex in the Index List
	**               3. If found, don't add it just use the existing one. If NOT found then add them into list
	** =================================================================================================================*/
	void gltfwriterVBO::indexVBO()
	{
		std::map<PackedVertex, unsigned short> VertexToOutIndex;

		// For each input vertex
		for (unsigned int i = 0; i < m_in_positions.size(); i++){

			PackedVertex packed = { m_in_positions[i], m_in_uvs[i], m_in_normals[i], m_in_vcolors[i] };

			// Try to find a similar vertex in out_XXXX
			unsigned short index;
			bool found = getSimilarVertexIndex(packed, VertexToOutIndex, index);

			if (found){ // A similar vertex is already in the VBO, use it instead !
				m_out_indices.push_back(index);
			}
			else{ // If not, it needs to be added in the output data.
				m_out_positions.push_back(m_in_positions[i]);
				m_out_uvs.push_back(m_in_uvs[i]);
				m_out_normals.push_back(m_in_normals[i]);
				m_out_tangents.push_back(m_in_tangents[i]);
				m_out_binormals.push_back(m_in_binormals[i]);
				m_out_vcolors.push_back(m_in_vcolors[i]);
				unsigned short newindex = (unsigned short)m_out_positions.size() - 1;
				m_out_indices.push_back(newindex);
				VertexToOutIndex[packed] = newindex;
			}
		}
	}

	/* ==================================================================================================================
	** Function    : GetVertexPositions
	** Abstraction : Find the Packed Vertex from the existing map, if found return true else return false
	** =================================================================================================================*/
	FbxArray<FbxVector4> gltfwriterVBO::GetVertexPositions(bool bInGeometry, bool bExportControlPoints) {
		// In an ordinary geometry, export the control points.
		// In a binded geometry, export transformed control points...
		// In a controller, export the control points.
		bExportControlPoints = true;

		FbxArray<FbxVector4> controlPoints;
		// Get Control points.
		// Translate a FbxVector4* into FbxArray<FbxVector4>
		FbxVector4 *pTemp = m_pMesh->GetControlPoints();
		int nbControlPoints = m_pMesh->GetControlPointsCount();
		for (int i = 0; i < nbControlPoints; i++)
			controlPoints.Add(pTemp[i]);

		if (bExportControlPoints) {
			if (!bInGeometry) {
				FbxAMatrix transform = m_pMesh->GetNode()->EvaluateGlobalTransform();
				for (int i = 0; i < nbControlPoints; i++)
					controlPoints[i] = transform.MultT(controlPoints[i]);
			}
			return (controlPoints);
		}

		// Initialize positions
		FbxArray<FbxVector4> positions(nbControlPoints);
		// Get the transformed control points.
		int deformerCount = m_pMesh->GetDeformerCount(FbxDeformer::eSkin);
		_ASSERTE(deformerCount <= 1); // Unexpected number of skin greater than 1
		// It is expected for deformerCount to be equal to 1
		for (int i = 0; i < deformerCount; i++) {
			int clusterCount = FbxCast<FbxSkin>(m_pMesh->GetDeformer(FbxDeformer::eSkin))->GetClusterCount();
			for (int indexLink = 0; indexLink < clusterCount; indexLink++) {
				FbxCluster *pLink = FbxCast<FbxSkin>(m_pMesh->GetDeformer(FbxDeformer::eSkin))->GetCluster(indexLink);
				FbxAMatrix jointPosition = pLink->GetLink()->EvaluateGlobalTransform();
				FbxAMatrix transformLink;
				pLink->GetTransformLinkMatrix(transformLink);
				FbxAMatrix m = transformLink.Inverse() * jointPosition;
				for (int j = 0; j < pLink->GetControlPointIndicesCount(); j++) {
					int index = pLink->GetControlPointIndices()[j];
					FbxVector4 controlPoint = controlPoints[index];
					double weight = pLink->GetControlPointWeights()[j];
					FbxVector4 pos = m.MultT(controlPoint);
					pos = pos * weight;
					positions[index] = positions[index] + pos;
				}
			}
		}
		return (positions);
	}

	/* ==================================================================================================================
	** Function    : GetLayerElement
	** Abstraction : From the given layer get the single element
	** =================================================================================================================*/
#define GetLayerElement(layerElt, indexV, T, V, index) \
	int indexV =-1 ; \
	T V ; \
	if ( layerElt ) { \
		indexV =(layerElt->GetReferenceMode () == FbxLayerElement::eDirect ? index : layerElt->GetIndexArray () [index]) ; \
		V =layerElt->GetDirectArray ().GetAt (indexV) ; \
		}

	/* ==================================================================================================================
	** Function    : GetLayerElements
	** Abstraction : From the given layerIndex get all the elements like Normals, uvs, vertex colors, Tangents, Binormals,..
	** =================================================================================================================*/
	void gltfwriterVBO::GetLayerElements(
		int layerIndex /*=0*/,
		bool bInGeometry /*=true*/)
	{
		// If the mesh is a skin binded to a skeleton, the bind pose will include its transformations.
		// In that case, do not export the transforms twice.
		int deformerCount = m_pMesh->GetDeformerCount(FbxDeformer::eSkin);
		_ASSERTE(deformerCount <= 1); // "Unexpected number of skin greater than 1") ;
		int clusterCount = 0;
		// It is expected for deformerCount to be equal to 1
		for (int i = 0; i < deformerCount; ++i)
			clusterCount += FbxCast<FbxSkin>(m_pMesh->GetDeformer(i, FbxDeformer::eSkin))->GetClusterCount();
		FbxArray<FbxVector4> vertices = GetVertexPositions(bInGeometry, (clusterCount == 0));


		// Normals
		FbxLayer *pLayer = m_pMesh->GetLayer(layerIndex/*, FbxLayerElement::eNormal*/);
		FbxLayerElementNormal *pLayerElementNormals = pLayer->GetNormals();
		if (pLayerElementNormals && pLayerElementNormals->GetMappingMode() != FbxLayerElement::eByPolygonVertex)
			pLayerElementNormals->RemapIndexTo(FbxLayerElement::eByPolygonVertex);

		// UV
		// todo support all uvs
		FbxArray<FbxLayerElement::EType> uvChannels = m_pMesh->GetAllChannelUV(layerIndex);
		//for ( int i =0 ; i < uvChannels.GetCount () ; i++ ) {
		//FbxLayerElementUV *pLayerElementUVs =pLayer->GetUVs (uvChannels.GetAt (i)) ;
		FbxLayerElementUV *pLayerElementUVs = pLayer->GetUVs(FbxLayerElement::eTextureDiffuse);
		if (pLayerElementUVs && pLayerElementUVs->GetMappingMode() != FbxLayerElement::eByPolygonVertex)
			pLayerElementUVs->RemapIndexTo(FbxLayerElement::eByPolygonVertex);
		if (pLayerElementUVs) {
			utility::string_t st(U("TEXCOORD_"));
			st += utility::conversions::to_string_t((int)m_uvSets.size());
			//FbxString name =pLayerElementUVs->GetName () ; // uvSet
			//_uvSets [name == "" ? "default" : name] =utility::conversions::to_string_t (st.Buffer ()) ;
			utility::string_t key = utility::conversions::to_string_t(
				FbxLayerElement::sTextureChannelNames[FbxLayerElement::eTextureDiffuse - FbxLayerElement::sTypeTextureStartIndex]
				);
			m_uvSets[key] = st;
		}
		//}

		// Vertex Color
		FbxLayerElementVertexColor *pLayerElementColors = pLayer->GetVertexColors();
		if (pLayerElementColors && pLayerElementColors->GetMappingMode() != FbxLayerElement::eByPolygonVertex)
			pLayerElementColors->RemapIndexTo(FbxLayerElement::eByPolygonVertex);

		// Tangents
		FbxGeometryElementTangent* pLayerTangent = pLayer->GetTangents();
		if (pLayerTangent && pLayerTangent->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
			pLayerTangent->RemapIndexTo(FbxLayerElement::eByPolygonVertex);

		// Binormals
		FbxGeometryElementBinormal* pLayerBinormal = pLayer->GetBinormals();
		if (pLayerBinormal && pLayerBinormal->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
			pLayerBinormal->RemapIndexTo(FbxLayerElement::eByPolygonVertex);

		int nb = m_pMesh->GetPolygonCount();
		for (int i = 0, index = 0; i < nb; i++) {
			int count = m_pMesh->GetPolygonSize(i);
			//vijay
			//_ASSERTE( count == 3 ) ; // We forced triangulation, so we expect '3' here
			for (int v = 0; v < count; v++) {
				int vertexID = m_pMesh->GetPolygonVertex(i, v);

				// In an ordinary geometry, export the control points.
				// In a binded geometry, export transformed control points...
				// In a controller, export the control points.
				FbxVector4 position = vertices[vertexID]; // pMesh->GetControlPoints () [vertexID] ;


				// Normals, uvs, colors, ...
				GetLayerElement(pLayerElementNormals, normalIndex, FbxVector4, normal, index);
				GetLayerElement(pLayerElementUVs, uvIndex, FbxVector2, uv, index);
				uv[1] = 1.0 - uv[1];
				GetLayerElement(pLayerElementColors, colorIndex, FbxColor, color, index);
				GetLayerElement(pLayerTangent, tangentIndex, FbxVector4, tangent, index);
				GetLayerElement(pLayerBinormal, binormalIndex, FbxVector4, binormal, index);

				m_in_positions.push_back(position);
				m_in_normals.push_back(normal);
				m_in_uvs.push_back(uv);
				m_in_vcolors.push_back(color);
				m_in_tangents.push_back(tangent);
				m_in_binormals.push_back(binormal);

				index++;
			}
		}
	}


	//-----------------------------------------------------------------------------

	// https://github.com/KhronosGroup/glTF/blob/master/specification/schema/mesh.schema.json
    // https://github.com/KhronosGroup/glTF/blob/master/specification/schema/mesh.primitive.schema.json
	// https://github.com/KhronosGroup/glTF/blob/master/specification/schema/mesh.primitive.attribute.schema.json
	web::json::value gltfWriter::WriteMesh(FbxNode *pNode) {
		web::json::value meshDef = web::json::value::object();
		web::json::value meshPrimitives = web::json::value::array();
		meshDef[U("name")] = web::json::value::string(nodeId(pNode)); // https://github.com/KhronosGroup/glTF/blob/master/specification/schema/glTFChildOfRootProperty.schema.json
		web::json::value accessorsAndBufferViews = web::json::value::object({
			{ U("accessors"), web::json::value::object() },
			{ U("bufferViews"), web::json::value::object() }
		});
		web::json::value materials = web::json::value::object({ { U("materials"), web::json::value::object() } });
		web::json::value techniques = web::json::value::object({ { U("techniques"), web::json::value::object() } });
		web::json::value programs = web::json::value::object({ { U("programs"), web::json::value::object() } });
		web::json::value images = web::json::value::object({ { U("images"), web::json::value::object() } });
		web::json::value samplers = web::json::value::object({ { U("samplers"), web::json::value::object() } });
		web::json::value textures = web::json::value::object({ { U("textures"), web::json::value::object() } });

		FbxMesh *pMesh = pNode->GetMesh(); //FbxCast<FbxMesh>(pNode->GetNodeAttribute ()) ;
		if (_triangulate) {
			FbxGeometryConverter converter(&mManager);
			pMesh = FbxCast<FbxMesh>(converter.Triangulate(pMesh, true));
		}
		pMesh->ComputeBBox();

		// Ensure we covered all layer elements of layer 0:
		// - Normals (all layers... does GLTF support this?)
		// - UVs on all layers
		// - Vertex Colors on all layers.
		// - Materials and Textures are covered when the mesh is exported.
		// - Warnings for unsupported layer element types: polygon groups, undefined



		int nbLayers = pMesh->GetLayerCount();
		for (int iLayer = 0; iLayer < nbLayers; iLayer++) {
			web::json::value primitive = web::json::value::object({
				{ U("attributes"), web::json::value::object() },
				{ U("mode"), IOglTF::TRIANGLES } // Allowed values are 0 (POINTS), 1 (LINES), 2 (LINE_LOOP), 3 (LINE_STRIP), 4 (TRIANGLES), 5 (TRIANGLE_STRIP), and 6 (TRIANGLE_FAN).
			});

			//web::json::value elts =web::json::value::object ({
			//	{ U("normals"), web::json::value::object () },
			//	{ U("uvs"), web::json::value::object () },
			//	{ U("colors"), web::json::value::object () }
			//}) ;
			web::json::value localAccessorsAndBufferViews = web::json::value::object();

			//IndicesDeindexer pp(pMesh, iLayer, true);
			gltfwriterVBO vbo(pMesh);
			vbo.GetLayerElements(iLayer, true);
			vbo.indexVBO();

			std::vector<unsigned short> out_indices = vbo.getIndices();
			std::vector<FbxDouble4> out_positions = vbo.getPositions();
			std::vector<FbxDouble2> out_uvs = vbo.getUvs();
			std::vector<FbxDouble3> out_normals = vbo.getNormals();
			std::vector<FbxDouble3> out_tangents = vbo.getTangents();
			std::vector<FbxDouble3> out_binormals = vbo.getBinormals();
			std::vector<FbxColor>   out_vcolors = vbo.getVertexColors();

			_uvSets = vbo.getUvSets();

			web::json::value vertex = WriteArrayWithMinMax<FbxDouble4, float>(out_positions, pMesh->GetNode(), U("_Positions"));
			MergeJsonObjects(localAccessorsAndBufferViews, vertex);
			primitive[U("attributes")][U("POSITION")] = web::json::value::string(GetJsonFirstKey(vertex[U("accessors")]));

			if (out_normals.size()) {
				utility::string_t st(U("_Normals"));
				if (iLayer != 0)
					st += utility::conversions::to_string_t(iLayer);
				web::json::value ret = WriteArrayWithMinMax<FbxDouble3, float>(out_normals, pMesh->GetNode(), st.c_str());
				MergeJsonObjects(localAccessorsAndBufferViews, ret);
				st = U("NORMAL");
				if (iLayer != 0)
					st += utility::conversions::to_string_t(iLayer);
				primitive[U("attributes")][st] = web::json::value::string(GetJsonFirstKey(ret[U("accessors")]));
			}

			if (out_uvs.size()) { // todo more than 1
				std::map<utility::string_t, utility::string_t>::iterator iter = _uvSets.begin();
				utility::string_t st(U("_") + iter->second);
				web::json::value ret = WriteArrayWithMinMax<FbxDouble2, float>(out_uvs, pMesh->GetNode(), st.c_str());
				MergeJsonObjects(localAccessorsAndBufferViews, ret);
				primitive[U("attributes")][iter->second] = web::json::value::string(GetJsonFirstKey(ret[U("accessors")]));
			}

			int nb = (int)out_vcolors.size();
			if (nb) {
				std::vector<FbxDouble4> vertexColors_(nb);
				for (int i = 0; i < nb; i++)
					vertexColors_.push_back(FbxDouble4(out_vcolors[i].mRed, out_vcolors[i].mGreen, out_vcolors[i].mBlue, out_vcolors[i].mAlpha));
				utility::string_t st(U("_Colors"));
				st += utility::conversions::to_string_t(iLayer);
				web::json::value ret = WriteArrayWithMinMax<FbxDouble4, float>(vertexColors_, pMesh->GetNode(), st.c_str());
				MergeJsonObjects(localAccessorsAndBufferViews, ret);
				st = U("COLOR_");
				st += utility::conversions::to_string_t(iLayer);
				primitive[U("attributes")][st] = web::json::value::string(GetJsonFirstKey(ret[U("accessors")]));
			}

			// Get mesh face indices
			//std::vector<unsigned short> faces;
			//utility::MapToVec(out_indices, faces);
			web::json::value polygons = WriteArray<unsigned short>(out_indices, 1, pMesh->GetNode(), U("_Polygons"));
			primitive[U("indices")] = web::json::value::string(GetJsonFirstKey(polygons[U("accessors")]));

			MergeJsonObjects(accessorsAndBufferViews, polygons);
			MergeJsonObjects(accessorsAndBufferViews, localAccessorsAndBufferViews);

			// Get material
			if (pMesh->GetLayer(iLayer, FbxLayerElement::eMaterial) == nullptr) {
				ucout << U("Error: (") << utility::conversions::to_string_t(pNode->GetTypeName())
					<< U(") ") << utility::conversions::to_string_t(pNode->GetName())
					<< U(" no material on Layer: ")
					<< iLayer
					<< U(" - no export for this layer")
					<< std::endl;
				continue;
			}
			FbxLayerElementMaterial *pLayerElementMaterial = pMesh->GetLayer(iLayer, FbxLayerElement::eMaterial)->GetMaterials();
			int materialCount = pLayerElementMaterial ? pNode->GetMaterialCount() : 0;
			if (materialCount > 1) {
				ucout << U("Warning: (") << utility::conversions::to_string_t(pNode->GetTypeName())
					<< U(") ") << utility::conversions::to_string_t(pNode->GetName())
					<< U(" got more than one material. glTF supports one material per primitive (FBX Layer).")
					<< std::endl;
			}
			// TODO: need to be revisited when glTF will support more than one material per layer/primitive
			/* TODO */materialCount = materialCount == 0 ? 0 : 1;
			for (int i = 0; i < materialCount; i++) {
				web::json::value ret = WriteMaterial(pNode, pNode->GetMaterial(i));
				if (ret.is_string()) {
					primitive[U("material")] = ret;
					continue;
				}
				primitive[U("material")] = web::json::value::string(GetJsonFirstKey(ret[U("materials")]));

				MergeJsonObjects(materials[U("materials")], ret[U("materials")]);
				if (ret.has_field(U("images")))
					MergeJsonObjects(images[U("images")], ret[U("images")]);
				if (ret.has_field(U("samplers")))
					MergeJsonObjects(samplers[U("samplers")], ret[U("samplers")]);
				if (ret.has_field(U("textures")))
					MergeJsonObjects(textures[U("textures")], ret[U("textures")]);

				utility::string_t techniqueName = GetJsonFirstKey(ret[U("techniques")]);
				web::json::value techniqueParameters = ret[U("techniques")][techniqueName][U("parameters")];
				AdditionalTechniqueParameters(pNode, techniqueParameters, out_normals.size() != 0);
				TechniqueParameters(pNode, techniqueParameters, primitive[U("attributes")], localAccessorsAndBufferViews[U("accessors")]);
				ret = WriteTechnique(pNode, pNode->GetMaterial(i), techniqueParameters);
				//MergeJsonObjects (techniques, ret) ;
				techniques[U("techniques")][techniqueName] = ret;

				utility::string_t programName = ret[U("passes")][ret[U("pass")].as_string()][U("instanceProgram")][U("program")].as_string();
				web::json::value attributes = ret[U("passes")][ret[U("pass")].as_string()][U("instanceProgram")][U("attributes")];
				ret = WriteProgram(pNode, pNode->GetMaterial(i), programName, attributes);
				MergeJsonObjects(programs, ret);
			}

			meshPrimitives[meshPrimitives.size()] = primitive;
		}
		meshDef[U("primitives")] = meshPrimitives;

		web::json::value lib = web::json::value::object({ { nodeId(pNode), meshDef } });
		web::json::value node = WriteNode(pNode);
		//if ( pMesh->GetShapeCount () )
		//	WriteControllerShape (pMesh) ; // Create a controller

		web::json::value ret = web::json::value::object({ { U("meshes"), lib }, { U("nodes"), node } });
		MergeJsonObjects(ret, accessorsAndBufferViews);
		MergeJsonObjects(ret, images);
		MergeJsonObjects(ret, materials);
		MergeJsonObjects(ret, techniques);
		MergeJsonObjects(ret, programs);
		MergeJsonObjects(ret, samplers);
		MergeJsonObjects(ret, textures);

		return (ret);
	}

}
