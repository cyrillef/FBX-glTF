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
#include "gltfwriterVBO.h"
#include <string.h> // for memcmp

namespace _IOglTF_NS_ {

#ifdef _XX_
FbxArray<FbxVector4> GetVertexPositions (FbxLine *pLine, bool bInGeometry =true, bool bExportControlPoints =true) {
	// In an ordinary geometry, export the control points.
	// In a binded geometry, export transformed control points...
	// In a controller, export the control points.
	bExportControlPoints =true ;

	FbxArray<FbxVector4> controlPoints ;
	// Get Control points.
	// Translate a FbxVector4* into FbxArray<FbxVector4>
	FbxVector4 *pTemp =pLine->GetControlPoints () ;
	int nbControlPoints =pLine->GetControlPointsCount () ;
	for ( int i =0 ; i < nbControlPoints ; i++ )
		controlPoints.Add (pTemp [i]) ;

	if ( bExportControlPoints ) {
		if ( !bInGeometry ) {
			FbxAMatrix transform =pLine->GetNode ()->EvaluateGlobalTransform () ;
			for ( int i =0 ; i < nbControlPoints ; i++ )
				controlPoints [i] =transform.MultT (controlPoints [i]) ;
		}
		return (controlPoints) ;
	}

	// Initialize positions
	FbxArray<FbxVector4> positions (nbControlPoints) ;
	// Get the transformed control points.
	int deformerCount =pLine->GetDeformerCount (FbxDeformer::eSkin) ;
	_ASSERTE (deformerCount <= 1); // Unexpected number of skin greater than 1
	// It is expected for deformerCount to be equal to 1
	for ( int i =0 ; i < deformerCount ; i++ ) {
		int clusterCount =FbxCast<FbxSkin> (pLine->GetDeformer (FbxDeformer::eSkin))->GetClusterCount () ;
		for ( int indexLink =0 ; indexLink < clusterCount ; indexLink++ ) {
			FbxCluster *pLink =FbxCast<FbxSkin> (pLine->GetDeformer (FbxDeformer::eSkin))->GetCluster (indexLink) ;
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
web::json::value gltfWriter::WriteLine (FbxNode *pNode) {
#ifdef GG
	web::json::value lineDef =web::json::value::object () ;
	lineDef [U("name")] =web::json::value::string (nodeId (pNode, true)) ;

	if ( isKnownId (pNode->GetNodeAttribute ()->GetUniqueID ()) ) {
		// The line/material/... were already exported, create only the transform node
		web::json::value node =WriteNode (pNode) ;
		web::json::value ret =web::json::value::object ({ { U("nodes"), node } }) ;
		return (ret) ;
	}

	web::json::value linePrimitives =web::json::value::array () ;
	web::json::value accessorsAndBufferViews =web::json::value::object ({
		{ U("accessors"), web::json::value::object () },
		{ U("bufferViews"), web::json::value::object () }
	}) ;
	web::json::value materials =web::json::value::object ({ { U("materials"), web::json::value::object () } }) ;
	web::json::value techniques =web::json::value::object ({ { U("techniques"), web::json::value::object () } }) ;
	web::json::value programs =web::json::value::object ({ { U("programs"), web::json::value::object () } }) ;
	web::json::value images =web::json::value::object ({ { U("images"), web::json::value::object () } }) ;
	web::json::value samplers =web::json::value::object ({ { U("samplers"), web::json::value::object () } }) ;
	web::json::value textures =web::json::value::object ({ { U("textures"), web::json::value::object () } }) ;

	FbxLine *pLine =pNode->GetLine () ; //FbxCast<FbxLine>(pNode->GetNodeAttribute ()) ;
	pLine->ComputeBBox () ;

	// FBX Layers works like Photoshop layers
	// - Vertices, Polygons and Edges are not on layers, but every other elements can
	// - Usually Normals are on Layer 0 (FBX default) but can eventually be on other layers
	// - UV, vertex colors, etc... can have multiple layer representation or be on different layers,
	//   the code below will try to retrieve each element recursevily to merge all elements from 
	//   different layer.

	// Ensure we covered all layer elements of layer 0
	// - Normals (all layers... does GLTF support this?)
	// - UVs on all layers
	// - Vertex Colors on all layers.
	// - Materials and Textures are covered when the line is exported.
	// - Warnings for unsupported layer element types: polygon groups, undefined

	int nbLayers =pLine->GetLayerCount () ;
	web::json::value primitive=web::json::value::object ({
		{ U("attributes"), web::json::value::object () },
		{ U("mode"), IOglTF::LINES } // Allowed values are 0 (POINTS), 1 (LINES), 2 (LINE_LOOP), 3 (LINE_STRIP), 4 (TRIANGLES), 5 (TRIANGLE_STRIP), and 6 (TRIANGLE_FAN).
	}) ;

	web::json::value localAccessorsAndBufferViews =web::json::value::object () ;

	int deformerCount =pLine->GetDeformerCount (FbxDeformer::eSkin) ;
	_ASSERTE (deformerCount <= 1) ; // "Unexpected number of skin greater than 1") ;
	int clusterCount =0 ;
	// It is expected for deformerCount to be equal to 1
	for ( int i =0 ; i < deformerCount ; i++ )
		clusterCount +=FbxCast<FbxSkin> (pLine->GetDeformer (i, FbxDeformer::eSkin))->GetClusterCount () ;
	FbxArray<FbxVector4> positions =GetVertexPositions (pLine, true, (clusterCount == 0)) ;

//	std::vector<FbxDouble3> out_positions =vbo.getPositions () ;

	web::json::value vertex =WriteArrayWithMinMax<FbxDouble3, float> (out_positions, pLine->GetNode (), U("_Positions")) ;
	MergeJsonObjects (localAccessorsAndBufferViews, vertex);
	primitive [U("attributes")] [U("POSITION")] =web::json::value::string (GetJsonFirstKey (vertex [U("accessors")])) ;

	// Get line face indices
	web::json::value polygons =WriteArray<unsigned short> (out_indices, 1, pLine->GetNode (), U("_Polygons")) ;
	primitive [U("indices")] =web::json::value::string (GetJsonFirstKey (polygons [U("accessors")])) ;

	MergeJsonObjects (accessorsAndBufferViews, polygons) ;
	MergeJsonObjects (accessorsAndBufferViews, localAccessorsAndBufferViews) ;

	// Get material
	FbxLayer *pLayer =gltfwriterVBO::getLayer (pLine, FbxLayerElement::eMaterial) ;
	if ( pLayer == nullptr ) {
		//ucout << U("Info: (") << utility::conversions::to_string_t (pNode->GetTypeName ())
		//	<< U(") ") << utility::conversions::to_string_t (pNode->GetName ())
		//	<< U(" no material on Layer: ")
		//	<< iLayer
		//	<< std::endl ;
		// Create default material
		web::json::value ret =WriteDefaultMaterial (pNode) ;
		if ( ret.is_string () ) {
			primitive [U("material")] =ret ;
		} else {
			primitive [U("material")]=web::json::value::string (GetJsonFirstKey (ret [U("materials")])) ;

			MergeJsonObjects (materials [U("materials")], ret [U("materials")]) ;

			utility::string_t techniqueName =GetJsonFirstKey (ret [U("techniques")]) ;
			web::json::value techniqueParameters =ret [U("techniques")] [techniqueName] [U("parameters")] ;
			AdditionalTechniqueParameters (pNode, techniqueParameters, out_normals.size () != 0) ;
			TechniqueParameters (pNode, techniqueParameters, primitive [U("attributes")], localAccessorsAndBufferViews [U("accessors")], false) ;
			ret =WriteTechnique (pNode, nullptr, techniqueParameters) ;
			//MergeJsonObjects (techniques, ret) ;
			techniques [U("techniques")] [techniqueName] =ret ;

			utility::string_t programName =ret [U("program")].as_string () ;
			web::json::value attributes =ret [U("attributes")] ;
			ret =WriteProgram (pNode, nullptr, programName, attributes) ;
			MergeJsonObjects (programs, ret) ;
		}
	} else {
		FbxLayerElementMaterial *pLayerElementMaterial =pLayer->GetMaterials () ;
		int materialCount =pLayerElementMaterial ? pNode->GetMaterialCount () : 0 ;
		if ( materialCount > 1 ) {
			_ASSERTE( materialCount > 1 ) ;
			ucout << U("Warning: (") << utility::conversions::to_string_t (pNode->GetTypeName ())
				<< U(") ") << utility::conversions::to_string_t (pNode->GetName ())
				<< U(" got more than one material. glTF supports one material per primitive (FBX Layer).")
				<< std::endl ;
		}
		// TODO: need to be revisited when glTF will support more than one material per layer/primitive
		materialCount =materialCount == 0 ? 0 : 1 ;
		for ( int i =0 ; i < materialCount ; i++ ) {
			web::json::value ret =WriteMaterial (pNode, pNode->GetMaterial (i)) ;
			if ( ret.is_string () ) {
				primitive [U("material")] =ret ;
				continue ;
			}
			primitive [U("material")]=web::json::value::string (GetJsonFirstKey (ret [U("materials")])) ;

			MergeJsonObjects (materials [U("materials")], ret [U("materials")]) ;
			if ( ret.has_field (U("images")) )
				MergeJsonObjects (images [U("images")], ret [U("images")]) ;
			if ( ret.has_field (U("samplers")) )
				MergeJsonObjects (samplers [U("samplers")], ret [U("samplers")]) ;
			if ( ret.has_field (U("textures")) )
				MergeJsonObjects (textures [U("textures")], ret [U("textures")]) ;

			utility::string_t techniqueName =GetJsonFirstKey (ret [U("techniques")]) ;
			web::json::value techniqueParameters =ret [U("techniques")] [techniqueName] [U("parameters")] ;
			AdditionalTechniqueParameters (pNode, techniqueParameters, out_normals.size () != 0) ;
			TechniqueParameters (pNode, techniqueParameters, primitive [U("attributes")], localAccessorsAndBufferViews [U("accessors")]) ;
			ret =WriteTechnique (pNode, pNode->GetMaterial (i), techniqueParameters) ;
			//MergeJsonObjects (techniques, ret) ;
			techniques [U("techniques")] [techniqueName] =ret ;

			utility::string_t programName =ret [U("program")].as_string () ;
			web::json::value attributes =ret [U("attributes")] ;
			ret =WriteProgram (pNode, pNode->GetMaterial (i), programName, attributes) ;
			MergeJsonObjects (programs, ret) ;
		}
	}
	linePrimitives [linePrimitives.size ()] =primitive ;
	lineDef [U("primitives")] =linePrimitives ;

	web::json::value lib =web::json::value::object ({ { nodeId (pNode, true, true), lineDef } }) ;

	web::json::value node =WriteNode (pNode) ;
	//if ( pLine->GetShapeCount () )
	//	WriteControllerShape (pLine) ; // Create a controller

	web::json::value ret =web::json::value::object ({ { U("meshes"), lib }, { U("nodes"), node } }) ;
	MergeJsonObjects (ret, accessorsAndBufferViews) ;
	MergeJsonObjects (ret, images) ;
	MergeJsonObjects (ret, materials) ;
	MergeJsonObjects (ret, techniques) ;
	MergeJsonObjects (ret, programs) ;
	MergeJsonObjects (ret, samplers) ;
	MergeJsonObjects (ret, textures) ;
	return (ret) ;
#else
	return (web::json::value::null ()) ;
#endif
}
#endif

}
