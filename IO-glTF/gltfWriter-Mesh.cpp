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

namespace _IOglTF_NS_ {

//-----------------------------------------------------------------------------
class IndicesDeindexer {
	struct VertexKey {
		/* A layer can contain one or more of the following layer elements :
			* Normals
				* Binormals
				* Tangents
			* Materials
				* Polygon Groups
			* UVs
			* Vertex Colors
				* Smoothing informations
				* Vertex Creases
				* Edge Creases
				* Custom User Data
				* Visibilities
				* Textures (diffuse, ambient, specular, etc.) (deprecated)
		*/
		int _vertexIndex ;
		int _normalIndex ;
		int _textureIndex ;
		int _vcolorIndex ;
			int _binormalIndex ;
			int _tangentIndex ;
			int _bitangentIndex ;

		VertexKey (int vertexIndex, int normalIndex =-1, int textureIndex =-1, int vcolorIndex =-1,
				   int binormalIndex =-1, int tangentIndex =-1, int bitangentIndex =-1)
			: _vertexIndex(vertexIndex), _normalIndex(normalIndex), _textureIndex(textureIndex), _vcolorIndex(vcolorIndex),
			  _binormalIndex(binormalIndex), _tangentIndex(tangentIndex), _bitangentIndex(bitangentIndex)
		{}

		inline bool operator == (const VertexKey &rhs) {
			return (
				   _vertexIndex == rhs._vertexIndex
				&& _normalIndex == rhs._normalIndex
				&& _textureIndex == rhs._textureIndex
				&& _vcolorIndex == rhs._vcolorIndex
				&& _binormalIndex == rhs._binormalIndex
				&& _tangentIndex == rhs._tangentIndex
				&& _bitangentIndex == rhs._bitangentIndex
			) ;
		}
		inline bool operator != (const VertexKey &rhs) { return (!(*this == rhs)) ; }

	} ;

public:
	std::map<std::pair<int, int>, int> _vertexPoly ;
	std::vector<VertexKey> _keyList ;
	std::vector<FbxDouble3> _vertex ;
	std::vector<FbxDouble3> _normals ;
	std::vector<FbxDouble2> _uvs ;
	std::vector<FbxColor> _vcolors ;
	std::map<utility::string_t, utility::string_t> _uvSets ;

public:
	IndicesDeindexer (FbxMesh *pMesh, int layerIndex =0, bool bInGeometry =true) ;

protected:
	size_t AddPolygonVertex (int polygonId, int vertex,
							 int vertexID, FbxVector4 &position,
							 int normalIndex =-1, const FbxVector4 &normal =FbxVector4 (),
							 int uvIndex =-1, const FbxVector2 &uv =FbxVector2 (),
							 int vcolorIndex =-1, const FbxColor &vcolor =FbxColor (1., 1., 1.)
							) ;
	static FbxArray<FbxVector4> GetVertexPositions (FbxMesh *pMesh, bool bInGeometry, bool bExportControlPoints =true) ;

} ;

#define GetLayerElement(layerElt, indexV, T, V, index) \
	int indexV =-1 ; \
	T V ; \
	if ( layerElt ) { \
		indexV =(layerElt->GetReferenceMode () == FbxLayerElement::eDirect ? index : layerElt->GetIndexArray () [index]) ; \
		V =layerElt->GetDirectArray ().GetAt (indexV) ; \
	}

IndicesDeindexer::IndicesDeindexer (FbxMesh *pMesh, int layerIndex /*=0*/, bool bInGeometry /*=true*/) {
	// If the mesh is a skin binded to a skeleton, the bind pose will include its transformations.
	// In that case, do not export the transforms twice.
	int deformerCount =pMesh->GetDeformerCount (FbxDeformer::eSkin) ;
	_ASSERTE (deformerCount <= 1) ; // "Unexpected number of skin greater than 1") ;
	int clusterCount =0 ;
	// It is expected for deformerCount to be equal to 1
	for ( int i =0 ; i < deformerCount ; ++i )
		clusterCount +=FbxCast<FbxSkin> (pMesh->GetDeformer (i, FbxDeformer::eSkin))->GetClusterCount () ;
	FbxArray<FbxVector4> vertices =IndicesDeindexer::GetVertexPositions (pMesh, bInGeometry, (clusterCount == 0)) ;

	// Normals
	FbxLayer *pLayer =pMesh->GetLayer (layerIndex/*, FbxLayerElement::eNormal*/) ;
	FbxLayerElementNormal *pLayerElementNormals =pLayer->GetNormals () ;
	if ( pLayerElementNormals && pLayerElementNormals->GetMappingMode () != FbxLayerElement::eByPolygonVertex )
		pLayerElementNormals->RemapIndexTo (FbxLayerElement::eByPolygonVertex) ;
	// UV
	// todo support all uvs
	FbxArray<FbxLayerElement::EType> uvChannels =pMesh->GetAllChannelUV (layerIndex) ;
	//for ( int i =0 ; i < uvChannels.GetCount () ; i++ ) {
		//FbxLayerElementUV *pLayerElementUVs =pLayer->GetUVs (uvChannels.GetAt (i)) ;
		FbxLayerElementUV *pLayerElementUVs =pLayer->GetUVs (FbxLayerElement::eTextureDiffuse) ;
		if ( pLayerElementUVs && pLayerElementUVs->GetMappingMode () != FbxLayerElement::eByPolygonVertex )
			pLayerElementUVs->RemapIndexTo (FbxLayerElement::eByPolygonVertex) ;
		if ( pLayerElementUVs ) {
			utility::string_t st (U("TEXCOORD_")) ;
			st +=utility::conversions::to_string_t ((int)_uvSets.size ()) ;
			//FbxString name =pLayerElementUVs->GetName () ; // uvSet
			//_uvSets [name == "" ? "default" : name] =utility::conversions::to_string_t (st.Buffer ()) ;
			utility::string_t key =utility::conversions::to_string_t (
				FbxLayerElement::sTextureChannelNames [FbxLayerElement::eTextureDiffuse - FbxLayerElement::sTypeTextureStartIndex]
			) ;
			_uvSets [key] =st ;
		}		
	//}

	// Vertex Color
	FbxLayerElementVertexColor *pLayerElementColors =pLayer->GetVertexColors () ;
	if ( pLayerElementColors && pLayerElementColors->GetMappingMode () != FbxLayerElement::eByPolygonVertex )
		pLayerElementColors->RemapIndexTo (FbxLayerElement::eByPolygonVertex) ;

	int nb =pMesh->GetPolygonCount () ;
	for ( int i =0, index =0 ; i < nb ; i++ ) {
		int count =pMesh->GetPolygonSize (i) ;
		_ASSERTE( count == 3 ) ; // We forced triangulation, so we expect '3' here
		for ( int v =0 ; v < count ; v++ ) {
			int vertexID =pMesh->GetPolygonVertex (i, v) ;

			// In an ordinary geometry, export the control points.
			// In a binded geometry, export transformed control points...
			// In a controller, export the control points.
			FbxVector4 position =vertices [vertexID] ; // pMesh->GetControlPoints () [vertexID] ;

			// Normals, uvs, colors, ...
			GetLayerElement(pLayerElementNormals, normalIndex, FbxVector4, normal, index) ;
			GetLayerElement(pLayerElementUVs, uvIndex, FbxVector2, uv, index) ;
			uv [1] =1.0 - uv [1] ;
			GetLayerElement(pLayerElementColors, colorIndex, FbxColor, color, index) ;

			AddPolygonVertex (
				i, v,
				vertexID, position,
				normalIndex, normal,
				uvIndex, uv,
				colorIndex, color
			) ;
			index++ ;
		}
	}
	_ASSERTE(
		   _vertex.size () != 0 && _vertexPoly.size () !=0
		&& (_normals.size () == 0 || _normals.size () == _vertex.size () )
		&& (_uvs.size () == 0 || _uvs.size () == _vertex.size () )
		&& (_vcolors.size () == 0 || _vcolors.size () == _vertex.size () )
	) ;
}

size_t IndicesDeindexer::AddPolygonVertex (
	int polygonId, int vertex,
	int vertexID, FbxVector4 &position,
	int normalIndex/*=-1*/, const FbxVector4 &normal /*=FbxVector4 ()*/,
	int uvIndex/*=-1*/, const FbxVector2 &uv /*=FbxVector2 ()*/,
	int vcolorIndex /*=-1*/, const FbxColor &vcolor /*=FbxColor (1., 1., 1.)*/
) {
	VertexKey key (
		vertexID, normalIndex, uvIndex, vcolorIndex
	) ;
	std::vector<VertexKey>::iterator it =std::find (_keyList.begin (), _keyList.end (), key) ;
	size_t index =_keyList.size () ;
	if ( it == _keyList.end () ) {
		_keyList.push_back (key) ;
		_vertex.push_back (position) ;
		if ( normalIndex != -1 )
			_normals.push_back (normal) ;
		if ( uvIndex != -1 )
			_uvs.push_back (uv) ;
		if ( vcolorIndex != -1 )
			_vcolors.push_back (vcolor) ;
	} else {
		index =std::distance (_keyList.begin (), it) ;
	}
	_vertexPoly [std::pair<int, int> (polygonId, vertex)] =(int)index ;
	return (index) ;
}

/*static*/ FbxArray<FbxVector4> IndicesDeindexer::GetVertexPositions (FbxMesh *pMesh, bool bInGeometry, bool bExportControlPoints) {
	// In an ordinary geometry, export the control points.
	// In a binded geometry, export transformed control points...
	// In a controller, export the control points.
	bExportControlPoints =true ;

	FbxArray<FbxVector4> controlPoints ;
	// Get Control points.
	// Translate a FbxVector4* into FbxArray<FbxVector4>
	FbxVector4 *pTemp =pMesh->GetControlPoints () ;
	int nbControlPoints =pMesh->GetControlPointsCount () ;
	for ( int i =0 ; i < nbControlPoints ; i++ )
		controlPoints.Add (pTemp [i]) ;

	if ( bExportControlPoints ) {
		if ( !bInGeometry ) {
			FbxAMatrix transform =pMesh->GetNode ()->EvaluateGlobalTransform () ;
			for ( int i =0; i < nbControlPoints ; i++ )
				controlPoints [i] =transform.MultT (controlPoints [i]) ;
		}
		return (controlPoints) ;
	}
	
	// Initialize positions
	FbxArray<FbxVector4> positions (nbControlPoints) ;
	// Get the transformed control points.
	int deformerCount =pMesh->GetDeformerCount (FbxDeformer::eSkin) ;
	_ASSERTE (deformerCount <= 1) ; // Unexpected number of skin greater than 1
	// It is expected for deformerCount to be equal to 1
	for ( int i =0 ; i < deformerCount ; i++ ) {
		int clusterCount =FbxCast<FbxSkin> (pMesh->GetDeformer (FbxDeformer::eSkin))->GetClusterCount () ;
		for ( int indexLink =0 ; indexLink < clusterCount ; indexLink++ ) {
			FbxCluster *pLink =FbxCast<FbxSkin> (pMesh->GetDeformer (FbxDeformer::eSkin))->GetCluster (indexLink) ;
			FbxAMatrix jointPosition =pLink->GetLink ()->EvaluateGlobalTransform () ;
			FbxAMatrix transformLink ;
			pLink->GetTransformLinkMatrix (transformLink) ;
			FbxAMatrix m =transformLink.Inverse () * jointPosition ;
			for ( int j =0 ; j < pLink->GetControlPointIndicesCount () ; j++ ) {
				int index =pLink->GetControlPointIndices () [j] ;
				FbxVector4 controlPoint =controlPoints [index] ;
				double weight =pLink->GetControlPointWeights () [j] ;
				FbxVector4 pos =m.MultT (controlPoint) ;
				pos =pos * weight ;
				positions [index] =positions [index] + pos ;
			}
		}
	}
	return (positions) ;
}

//-----------------------------------------------------------------------------

// https://github.com/KhronosGroup/glTF/blob/master/specification/mesh.schema.json
// https://github.com/KhronosGroup/glTF/blob/master/specification/meshPrimitive.schema.json
// https://github.com/KhronosGroup/glTF/blob/master/specification/meshPrimitiveAttribute.schema.json
web::json::value gltfWriter::WriteMesh (FbxNode *pNode) {
	web::json::value meshDef =web::json::value::object () ;
	web::json::value meshPrimitives =web::json::value::array () ;
	meshDef [U("name")] =web::json::value::string (nodeId (pNode)) ; // https://github.com/KhronosGroup/glTF/blob/master/specification/glTFChildOfRootProperty.schema.json
	web::json::value accessorsAndBufferViews =web::json::value::object ({
		{ U("accessors"), web::json::value::object () },
		{ U("bufferViews"), web::json::value::object () }
	}) ;
	web::json::value materials =web::json::value::object ({{ U("materials"), web::json::value::object () }}) ;
	web::json::value techniques =web::json::value::object ({{ U("techniques"), web::json::value::object () }}) ;
	web::json::value programs =web::json::value::object ({{ U("programs"), web::json::value::object () }}) ;
	web::json::value images =web::json::value::object ({{ U("images"), web::json::value::object () }}) ;
	web::json::value samplers =web::json::value::object ({{ U("samplers"), web::json::value::object () }}) ;
	web::json::value textures =web::json::value::object ({{ U("textures"), web::json::value::object () }}) ;

	FbxMesh *pMesh =pNode->GetMesh () ; //FbxCast<FbxMesh>(pNode->GetNodeAttribute ()) ;
	if ( _triangulate ) {
		FbxGeometryConverter converter (&mManager) ;
		pMesh =FbxCast<FbxMesh>(converter.Triangulate (pMesh, true)) ;
	}
	pMesh->ComputeBBox () ;

	// Ensure we covered all layer elements of layer 0:
	// - Normals (all layers... does GLTF support this?)
	// - UVs on all layers
	// - Vertex Colors on all layers.
	// - Materials and Textures are covered when the mesh is exported.
	// - Warnings for unsupported layer element types: polygon groups, undefined

	int nbLayers =pMesh->GetLayerCount () ;
	for ( int iLayer =0 ; iLayer < nbLayers ; iLayer++ ) {
		web::json::value primitive =web::json::value::object ({
			{ U("attributes"), web::json::value::object () },
			{ U("primitive"), IOglTF::TRIANGLES } // Allowed values are 0 (POINTS), 1 (LINES), 2 (LINE_LOOP), 3 (LINE_STRIP), 4 (TRIANGLES), 5 (TRIANGLE_STRIP), and 6 (TRIANGLE_FAN).
		}) ;
		
		//web::json::value elts =web::json::value::object ({
		//	{ U("normals"), web::json::value::object () },
		//	{ U("uvs"), web::json::value::object () },
		//	{ U("colors"), web::json::value::object () }
		//}) ;
		web::json::value localAccessorsAndBufferViews =web::json::value::object () ;

		IndicesDeindexer pp (pMesh, iLayer, true) ;
		_uvSets =pp._uvSets ;

		web::json::value vertex =WriteArrayWithMinMax<FbxDouble3, float> (pp._vertex, pMesh->BBoxMin, pMesh->BBoxMax, pMesh->GetNode (), U("_Positions")) ;
		MergeJsonObjects (localAccessorsAndBufferViews, vertex) ;
		primitive [U("attributes")] [U("POSITION")] =web::json::value::string (GetJsonFirstKey (vertex [U("accessors")])) ;

		if ( pp._normals.size () ) {
			utility::string_t st (U("_Normals")) ;
			if ( iLayer != 0 )
				st +=utility::conversions::to_string_t (iLayer) ;
			web::json::value ret =WriteArrayWithMinMax<FbxDouble3, float> (pp._normals, pMesh->GetNode (), st.c_str ()) ;
			MergeJsonObjects (localAccessorsAndBufferViews, ret) ;
			st =U("NORMAL") ;
			if ( iLayer != 0 )
				st +=utility::conversions::to_string_t (iLayer) ;
			primitive [U("attributes")] [st] =web::json::value::string (GetJsonFirstKey (ret [U("accessors")])) ;
		}

		if ( pp._uvs.size () ) { // todo more than 1
			std::map<utility::string_t, utility::string_t>::iterator iter =_uvSets.begin () ;
			utility::string_t st (U("_") + iter->second) ;
			web::json::value ret =WriteArrayWithMinMax<FbxDouble2, float> (pp._uvs, pMesh->GetNode (), st.c_str ()) ;
			MergeJsonObjects (localAccessorsAndBufferViews, ret) ;
			primitive [U("attributes")] [iter->second] =web::json::value::string (GetJsonFirstKey (ret [U("accessors")])) ;
		}

		int nb =(int)pp._vcolors.size () ;
		if ( nb ) {
			std::vector<FbxDouble4> vertexColors_ (nb) ;
			for ( int i =0 ; i < nb ; i++ )
				vertexColors_.push_back (FbxDouble4 (pp._vcolors [i].mRed, pp._vcolors [i].mGreen, pp._vcolors [i].mBlue, pp._vcolors [i].mAlpha)) ;
			utility::string_t st (U("_Colors")) ;
			st +=utility::conversions::to_string_t (iLayer) ;
			web::json::value ret =WriteArrayWithMinMax<FbxDouble4, float> (vertexColors_, pMesh->GetNode (), st.c_str ()) ;
			MergeJsonObjects (localAccessorsAndBufferViews, ret) ;
			st =U("COLOR_") ;
			st +=utility::conversions::to_string_t (iLayer) ;
			primitive [U ("attributes")] [st] =web::json::value::string (GetJsonFirstKey (ret [U("accessors")])) ;
		}

		// Get mesh face indices
		std::vector<unsigned short> faces ;
		utility::MapToVec (pp._vertexPoly, faces) ;
		web::json::value polygons =WriteArray<unsigned short> (faces, 1, pMesh->GetNode (), U("_Polygons")) ;
		primitive [U("indices")] =web::json::value::string (GetJsonFirstKey (polygons [U("accessors")])) ;

		MergeJsonObjects (accessorsAndBufferViews, polygons) ;
		MergeJsonObjects (accessorsAndBufferViews, localAccessorsAndBufferViews) ;

		// Get material
		if ( pMesh->GetLayer (iLayer, FbxLayerElement::eMaterial) == nullptr ) {
			ucout << U("Error: (") << utility::conversions::to_string_t (pNode->GetTypeName ())
				  << U(") ") <<  utility::conversions::to_string_t (pNode->GetName ())
				  << U(" no material on Layer: ")
				  << iLayer
				  << U(" - no export for this layer")
				  << std::endl ;
			continue ;
		}
		FbxLayerElementMaterial *pLayerElementMaterial =pMesh->GetLayer (iLayer, FbxLayerElement::eMaterial)->GetMaterials () ;
		int materialCount =pLayerElementMaterial ? pNode->GetMaterialCount () : 0 ;
		if ( materialCount > 1 ) {
			ucout << U("Warning: (") << utility::conversions::to_string_t (pNode->GetTypeName ())
				 << U(") ") <<  utility::conversions::to_string_t (pNode->GetName ())
				  << U(" got more than one material. glTF supports one material per primitive (FBX Layer).")
				  << std::endl ;
		}
		// TODO: need to be revisited when glTF will support more than one material per layer/primitive
		/* TODO */materialCount =materialCount == 0 ? 0 : 1 ;
		for ( int i =0 ; i < materialCount ; i++ ) {
			web::json::value ret =WriteMaterial (pNode, pNode->GetMaterial (i)) ;
			if ( ret.is_string () ) {
				primitive [U("material")] =ret ;
				continue ;
			}
			primitive [U("material")] =web::json::value::string (GetJsonFirstKey (ret [U("materials")])) ;

			MergeJsonObjects (materials [U("materials")], ret [U("materials")]) ;
			if ( ret.has_field (U("images")) )
				MergeJsonObjects (images [U("images")], ret [U("images")]) ;
			if ( ret.has_field (U("samplers")) )
				MergeJsonObjects (samplers [U("samplers")], ret [U("samplers")]) ;
			if ( ret.has_field (U("textures")) )
				MergeJsonObjects (textures [U("textures")], ret [U("textures")]) ;

			utility::string_t techniqueName =GetJsonFirstKey (ret [U("techniques")]) ;
			web::json::value techniqueParameters =ret [U("techniques")] [techniqueName] [U("parameters")] ;
			AdditionalTechniqueParameters (pNode, techniqueParameters, pp._normals.size () != 0) ;
			TechniqueParameters (pNode, techniqueParameters, primitive [U("attributes")], localAccessorsAndBufferViews [U("accessors")]) ;
			ret =WriteTechnique (pNode, pNode->GetMaterial (i), techniqueParameters) ;
			//MergeJsonObjects (techniques, ret) ;
			techniques [U("techniques")] [techniqueName] =ret ;

			utility::string_t programName =ret [U("passes")] [ret [U("pass")].as_string ()] [U("instanceProgram")] [U("program")].as_string () ;
			web::json::value attributes =ret [U("passes")] [ret [U("pass")].as_string ()] [U("instanceProgram")] [U("attributes")] ;
			ret =WriteProgram (pNode, pNode->GetMaterial (i), programName, attributes) ;
			MergeJsonObjects (programs, ret) ;
		}

		meshPrimitives [meshPrimitives.size ()] =primitive ;
	}
	meshDef [U("primitives")] =meshPrimitives ;

	web::json::value lib =web::json::value::object ({ { nodeId (pNode), meshDef } }) ;
	web::json::value node =WriteNode (pNode) ;
	//if ( pMesh->GetShapeCount () )
	//	WriteControllerShape (pMesh) ; // Create a controller

	web::json::value ret =web::json::value::object ({ { U("meshes"), lib }, { U("nodes"), node } }) ;
	MergeJsonObjects (ret, accessorsAndBufferViews) ;
	MergeJsonObjects (ret, images) ;
	MergeJsonObjects (ret, materials) ;
	MergeJsonObjects (ret, techniques) ;
	MergeJsonObjects (ret, programs) ;
	MergeJsonObjects (ret, samplers) ;
	MergeJsonObjects (ret, textures) ;

	return (ret) ;
}

}
