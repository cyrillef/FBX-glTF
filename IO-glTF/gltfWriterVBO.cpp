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
#include "gltfwriterVBO.h"

namespace _IOglTF_NS_ {

// Function    : getSimilarVertexIndex
// Abstraction : Find the Packed Vertex from the existing map, if found return true else return false
bool gltfwriterVBO::getSimilarVertexIndex (PackedVertex &packed, std::map<PackedVertex, unsigned short> &VertexToOutIndex, unsigned short &result) {
	std::map<PackedVertex, unsigned short>::iterator it =VertexToOutIndex.find (packed) ;
	if ( it == VertexToOutIndex.end () )
		return (false) ;
	result =it->second ;
	return (true) ;
}

// Function    : indexVBO()
// Abstraction : 1. Pack positions, uvs, normals, vertex colors in to single entity (packedVertex)
//               2. Search the packed vertex in the Index List
//               3. If found, don't add it just use the existing one. If NOT found then add them into list
void gltfwriterVBO::indexVBO () {
	std::map<PackedVertex, unsigned short> VertexToOutIndex;
	// For each input vertex
	for ( unsigned int i =0 ; i < _in_positions.size () ; i++ ) {
		FbxDouble2 in_uv =i < _in_uvs.size () ? _in_uvs [i] : FbxDouble2 () ;
		FbxDouble3 in_normal =i < _in_normals.size () ? _in_normals [i] : FbxDouble3 () ;
		FbxDouble3 in_tangent =i < _in_tangents.size () ? _in_tangents [i] : FbxDouble3 () ;
		FbxDouble3 in_binormal =i < _in_binormals.size () ? _in_binormals [i] : FbxDouble3 () ;
		FbxColor in_vcolor =i < _in_vcolors.size () ? _in_vcolors [i] : FbxColor () ;

		PackedVertex packed ={ _in_positions [i], in_uv, in_normal, in_tangent, in_binormal, in_vcolor } ;

		// Try to find a similar vertex in out_XXXX
		unsigned short index ;
		bool found =getSimilarVertexIndex (packed, VertexToOutIndex, index) ;
		if ( found ) { // A similar vertex is already in the VBO, use it instead !
			_out_indices.push_back (index) ;
		} else { // If not, it needs to be added in the output data.
			_out_positions.push_back (_in_positions [i]) ;
			if ( _in_uvs.size () )
				_out_uvs.push_back (in_uv) ;
			if ( _in_normals.size () )
				_out_normals.push_back (in_normal) ;
			if ( _in_tangents.size () )
				_out_tangents.push_back (in_tangent) ;
			if ( _in_binormals.size () )
				_out_binormals.push_back (in_binormal) ;
			if ( _in_vcolors.size () )
				_out_vcolors.push_back (in_vcolor) ;

			index =(unsigned short)_out_positions.size () - 1 ;
			VertexToOutIndex [packed] =index ;
			_out_indices.push_back (index) ;
		}
	}
}

// Function    : GetVertexPositions
// Abstraction : Find the Packed Vertex from the existing map, if found return true else return false
FbxArray<FbxVector4> gltfwriterVBO::GetVertexPositions (bool bInGeometry, bool bExportControlPoints) {
	// In an ordinary geometry, export the control points.
	// In a binded geometry, export transformed control points...
	// In a controller, export the control points.
	bExportControlPoints =true ;

	FbxArray<FbxVector4> controlPoints ;
	// Get Control points.
	// Translate a FbxVector4* into FbxArray<FbxVector4>
	FbxVector4 *pTemp =_pMesh->GetControlPoints () ;
	int nbControlPoints =_pMesh->GetControlPointsCount () ;
	for ( int i =0 ; i < nbControlPoints ; i++ )
		controlPoints.Add (pTemp [i]) ;

	if ( bExportControlPoints ) {
		if ( !bInGeometry ) {
			FbxAMatrix transform =_pMesh->GetNode ()->EvaluateGlobalTransform () ;
			for ( int i =0 ; i < nbControlPoints ; i++ )
				controlPoints [i] =transform.MultT (controlPoints [i]) ;
		}
		return (controlPoints) ;
	}

	// Initialize positions
	FbxArray<FbxVector4> positions (nbControlPoints) ;
	// Get the transformed control points.
	int deformerCount =_pMesh->GetDeformerCount (FbxDeformer::eSkin) ;
	_ASSERTE (deformerCount <= 1); // Unexpected number of skin greater than 1
	// It is expected for deformerCount to be equal to 1
	for ( int i =0 ; i < deformerCount ; i++ ) {
		int clusterCount =FbxCast<FbxSkin> (_pMesh->GetDeformer (FbxDeformer::eSkin))->GetClusterCount () ;
		for ( int indexLink =0 ; indexLink < clusterCount ; indexLink++ ) {
			FbxCluster *pLink =FbxCast<FbxSkin> (_pMesh->GetDeformer (FbxDeformer::eSkin))->GetCluster (indexLink) ;
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

// Function    : GetLayerElements
void gltfwriterVBO::GetLayerElements (bool bInGeometry /*=true*/) {

    // ambient: material.ambientColorMap | material.ambientColor | [0, 0, 0, 1]
    // diffuse: material.diffuseColorMap | material.diffuseColor | [0, 0, 0, 1]
    // emissive: material.emissiveColorMap | material.emissiveColor | [0, 0, 0, 1]
    // specular: material.specularColorMap | specularColor

	// If the mesh is a skin binded to a skeleton, the bind pose will include its transformations.
	// In that case, do not export the transforms twice.
	int deformerCount =_pMesh->GetDeformerCount (FbxDeformer::eSkin) ;
	_ASSERTE (deformerCount <= 1) ; // "Unexpected number of skin greater than 1") ;
	int clusterCount =0 ;
	// It is expected for deformerCount to be equal to 1
	for ( int i =0 ; i < deformerCount ; i++ )
		clusterCount +=FbxCast<FbxSkin> (_pMesh->GetDeformer (i, FbxDeformer::eSkin))->GetClusterCount () ;

	FbxArray<FbxVector4> vertices =GetVertexPositions (bInGeometry, (clusterCount == 0)) ; // Vertex Positions
	FbxLayerElementNormal *pLayerElementNormals =elementNormals () ; // Normals

	// UV or Vertex Color
	std::map<FbxLayerElement::EType, FbxLayerElementUV *> channels ;
	int nbLayers =_pMesh->GetLayerCount () ;
	for ( int iLayer =0 ; iLayer < nbLayers ; iLayer++ ) {
		FbxArray<FbxLayerElement::EType> uvChannels =_pMesh->GetAllChannelUV (iLayer) ;
		for ( int i =0 ; i < uvChannels.GetCount () ; i++ ) {
			FbxLayerElement::EType channel =uvChannels.GetAt (i) ;
			if ( channels.find (channel) != channels.cend () )
				continue ; // We got one already for this channel
			FbxLayerElementUV *pLayerElementUVs =elementUVs (channel, iLayer) ;
			channels [channel] =pLayerElementUVs ;
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
		}
	}

	FbxGeometryElementTangent *pLayerTangents =elementTangents () ; // Tangents
	FbxGeometryElementBinormal *pLayerBinormals =elementBinormals () ; // Binormals
	FbxLayerElementVertexColor *pLayerElementColors =elementVcolors () ; // Vertex Color

	int nb =_pMesh->GetPolygonCount () ;
	for ( int i =0, index =0 ; i < nb ; i++ ) {
		int count =_pMesh->GetPolygonSize (i) ;
		_ASSERTE( count == 3 ) ; // We forced triangulation, so we expect '3' here
		for ( int v =0 ; v < count ; v++, index++ ) {
			int vertexID =_pMesh->GetPolygonVertex (i, v) ;
			// In an ordinary geometry, export the control points.
			// In a binded geometry, export transformed control points...
			// In a controller, export the control points.
			FbxVector4 position =vertices [vertexID] ; // pMesh->GetControlPoints () [vertexID] ;
			_in_positions.push_back (position) ;

			GetLayerElement (pLayerElementNormals, normalIndex, FbxVector4, normal, index, [this] (FbxVector4 &V) {
				_in_normals.push_back (V) ;
			}) ;
			FbxLayerElementUV *pLayerElementUVs =channels [FbxLayerElement::eTextureDiffuse] ;
			GetLayerElement (pLayerElementUVs, uvIndex, FbxVector2, uv, index, [this] (FbxVector2 &V) {
				V [1] =1.0 - V [1] ;
				_in_uvs.push_back (V) ;
			}) ;
			GetLayerElement (pLayerTangents, tangentIndex, FbxVector4, tangent, index, [this] (FbxVector4 &V) {
				_in_tangents.push_back (V) ;
			}) ;
			GetLayerElement (pLayerBinormals, binormalIndex, FbxVector4, binormal, index, [this] (FbxVector4 &V) {
				_in_binormals.push_back (V) ;
			}) ;
			GetLayerElement (pLayerElementColors, colorIndex, FbxColor, color, index, [this] (FbxColor &V) {
				_in_vcolors.push_back (V) ;
			}) ;
		}
	}
}

FbxLayerElementNormal *gltfwriterVBO::elementNormals (int iLayer /*=-1*/)  {
	if ( iLayer != -1 ) {
		FbxLayer *pLayer =_pMesh->GetLayer (iLayer) ;
		FbxLayerElementNormal *pLayerElementNormals =pLayer->GetNormals () ;
		if ( pLayerElementNormals && pLayerElementNormals->GetMappingMode () != FbxLayerElement::eByPolygonVertex )
			pLayerElementNormals->RemapIndexTo (FbxLayerElement::eByPolygonVertex) ;
		return (pLayerElementNormals) ;
	} else {
		int nbLayers =_pMesh->GetLayerCount () ;
		for ( iLayer =0 ; iLayer < nbLayers ; iLayer++ ) {
			FbxLayerElementNormal *pLayerElementNormals =elementNormals (iLayer) ;
			if ( pLayerElementNormals )
				return (pLayerElementNormals) ;
		}
	}
	return (nullptr) ;
}

FbxLayerElementUV *gltfwriterVBO::elementUVs (FbxLayerElement::EType channel, int iLayer /*=-1*/) {
	if ( iLayer != -1 ) {
		FbxLayer *pLayer =_pMesh->GetLayer (iLayer) ;
		FbxLayerElementUV *pLayerElementUVs =pLayer->GetUVs (channel) ;
		if ( pLayerElementUVs && pLayerElementUVs->GetMappingMode () != FbxLayerElement::eByPolygonVertex )
			pLayerElementUVs->RemapIndexTo (FbxLayerElement::eByPolygonVertex) ;
		return (pLayerElementUVs) ;
	} else {
		int nbLayers =_pMesh->GetLayerCount () ;
		for ( iLayer =0 ; iLayer < nbLayers ; iLayer++ ) {
			FbxLayerElementUV *pLayerElementUVs =elementUVs (channel, iLayer) ;
			if ( pLayerElementUVs )
				return (pLayerElementUVs) ;
		}
	}
	return (nullptr) ;
}

FbxGeometryElementTangent *gltfwriterVBO::elementTangents (int iLayer /*=-1*/) {
	if ( iLayer != -1 ) {
		FbxLayer *pLayer =_pMesh->GetLayer (iLayer) ;
		FbxGeometryElementTangent* pLayerTangent =pLayer->GetTangents () ;
		if ( pLayerTangent && pLayerTangent->GetMappingMode () == FbxGeometryElement::eByPolygonVertex )
			pLayerTangent->RemapIndexTo (FbxLayerElement::eByPolygonVertex) ;
		return (pLayerTangent) ;
	} else {
		int nbLayers =_pMesh->GetLayerCount () ;
		for ( iLayer =0 ; iLayer < nbLayers ; iLayer++ ) {
			FbxGeometryElementTangent *pLayerTangent =elementTangents (iLayer) ;
			if ( pLayerTangent )
				return (pLayerTangent) ;
		}
	}
	return (nullptr) ;
}

FbxGeometryElementBinormal *gltfwriterVBO::elementBinormals (int iLayer /*=-1*/) {
	if ( iLayer != -1 ) {
		FbxLayer *pLayer =_pMesh->GetLayer (iLayer) ;
		FbxGeometryElementBinormal* pLayerBinormal =pLayer->GetBinormals () ;
		if ( pLayerBinormal && pLayerBinormal->GetMappingMode () == FbxGeometryElement::eByPolygonVertex )
			pLayerBinormal->RemapIndexTo (FbxLayerElement::eByPolygonVertex) ;
		return (pLayerBinormal) ;
	} else {
		int nbLayers =_pMesh->GetLayerCount () ;
		for ( iLayer =0 ; iLayer < nbLayers ; iLayer++ ) {
			FbxGeometryElementBinormal *pLayerBinormal =elementBinormals (iLayer) ;
			if ( pLayerBinormal )
				return (pLayerBinormal) ;
		}
	}
	return (nullptr) ;
}

FbxLayerElementVertexColor *gltfwriterVBO::elementVcolors (int iLayer /*=-1*/) {
	if ( iLayer != -1 ) {
		FbxLayer *pLayer =_pMesh->GetLayer (iLayer) ;
		FbxLayerElementVertexColor *pLayerElementColors =nullptr ;
		pLayerElementColors =pLayer->GetVertexColors () ;
		if ( pLayerElementColors && pLayerElementColors->GetMappingMode () != FbxLayerElement::eByPolygonVertex )
			pLayerElementColors->RemapIndexTo (FbxLayerElement::eByPolygonVertex) ;
		return (pLayerElementColors) ;
	} else {
		int nbLayers =_pMesh->GetLayerCount () ;
		for ( iLayer =0 ; iLayer < nbLayers ; iLayer++ ) {
			FbxLayerElementVertexColor *pLayerElementColors =elementVcolors (iLayer) ;
			if ( pLayerElementColors )
				return (pLayerElementColors) ;
		}
	}
	return (nullptr) ;
}

/*static*/ FbxLayer *gltfwriterVBO::getLayer (FbxMesh *pMesh, FbxLayerElement::EType pType) {
	int nbLayers =pMesh->GetLayerCount () ;
	for ( int iLayer =0 ; iLayer < nbLayers ; iLayer++ ) {
		FbxLayer *pLayer =pMesh->GetLayer (iLayer, FbxLayerElement::eMaterial) ;
		if ( pLayer != nullptr )
			return (pLayer) ;
	}
	return (nullptr) ;
}

}
