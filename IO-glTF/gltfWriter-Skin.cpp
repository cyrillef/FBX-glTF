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

web::json::value gltfWriter::WriteSkinArray(FbxNode *pNode, std::vector<FbxVector4> vertexArray, int skinAccessorCount, utility::string_t suffix) {

   utility::string_t skinAccName;
   web::json::value ret;
   web::json::value accessorsAndBufferViews =web::json::value::object ({
                { U("accessors"), web::json::value::object () },
                { U("bufferViews"), web::json::value::object () }
        }) ;

    skinAccName = utility::conversions::to_string_t (U("_skinAccessor_")) ;
    skinAccName += utility::conversions::to_string_t (suffix) ;
    skinAccName += utility::conversions::to_string_t ((int)skinAccessorCount);
    web::json::value skinAccessor =WriteArray <FbxVector4, float>(vertexArray, pNode, skinAccName.c_str()) ;
    ret = web::json::value::string (GetJsonFirstKey (skinAccessor [U("accessors")])) ;
    MergeJsonObjects (accessorsAndBufferViews, skinAccessor) ;
    MergeJsonObjects (_json, accessorsAndBufferViews) ;
return ret;
}


web::json::value gltfWriter::WriteSkin(FbxMesh *pMesh) {
	int lClusterCount = 0;
        web::json::value skins = web::json::value::object ();
        web::json::value jointNames;
	std::vector<FbxVector4> vertexArray;

	utility::string_t skinName;
	skinName  = utility::conversions::to_string_t (nodeId(pMesh->GetNode()));
	skinName += utility::conversions::to_string_t (U("_skin")) ;

	FbxArray<FbxVector4> controlPoints ;
        // Get Control points.
        // Translate a FbxVector4* into FbxArray<FbxVector4>
        FbxVector4 *pTemp =pMesh->GetControlPoints () ;
        int nbControlPoints =pMesh->GetControlPointsCount () ;
        for ( int i =0 ; i < nbControlPoints ; i++ )
                controlPoints.Add (pTemp [i]) ;

	// Initialize positions
        FbxArray<FbxVector4> positions (nbControlPoints) ;
        std::vector<FbxVector4> linkVertexes; 
        // Get the transformed control points.
        int deformerCount =pMesh->GetDeformerCount (FbxDeformer::eSkin) ;
        _ASSERTE (deformerCount <= 1); // Unexpected number of skin greater than 1
        // It is expected for deformerCount to be equal to 1
	for ( int i =0 ; i < deformerCount ; i++ ) {
		FbxSkin * lSkinDeformer = (FbxSkin *)pMesh->GetDeformer(0, FbxDeformer::eSkin);
		int clusterCount = lSkinDeformer->GetClusterCount();
		for ( int indexLink =0 ; indexLink < clusterCount ; indexLink++ ) {
			FbxCluster *pLink =FbxCast<FbxSkin> (pMesh->GetDeformer (0, FbxDeformer::eSkin))->GetCluster (indexLink) ;
			// We are interested only in joints
			if(!pLink->GetLink()) continue;
			// JointNames should be unique and it should be as same as skeletons array
			jointNames[jointNames.size()] = web::json::value::string (utility::conversions::to_string_t (pLink->GetLink()->GetName()));

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
			/* The array length is the same as the count property of the inverseBindMatrices accessor, 
			   and the same as the total quantity of all skeleton nodes from node-trees referenced by 
			   the skinned mesh instance node's skeletons array.
			*/
			linkVertexes.push_back(positions[indexLink]);
		}
	}
	skins[skinName][U("bindShapeMatrix")] = GetTransform(pMesh->GetNode());
	skins[skinName][U("inverseBindMatrices")] = WriteSkinArray(pMesh->GetNode(), linkVertexes, 1, "skin");
	skins[skinName][U("jointNames")] = jointNames;

   return (skins);
}

}
