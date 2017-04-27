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

//-----------------------------------------------------------------------------
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

		PackedVertex packed;
		if (_in_joints.size() && _in_weights.size())
		packed ={ _in_positions [i], _in_joints[i], _in_weights[i], in_uv, in_normal, in_tangent, in_binormal, in_vcolor } ;
		else
		packed ={ _in_positions [i], {0,0,0,0},{0,0,0,0},in_uv, in_normal, in_tangent, in_binormal, in_vcolor } ;

		// Try to find a similar vertex in out_XXXX
		unsigned short index ;
		bool found =getSimilarVertexIndex (packed, VertexToOutIndex, index) ;
		if ( found ) { // A similar vertex is already in the VBO, use it instead !
			_out_indices.push_back (index) ;
		} else { // If not, it needs to be added in the output data.
			_out_positions.push_back (_in_positions [i]) ;
			if (_in_joints.size() && _in_weights.size()) {
				_out_joints.push_back (_in_joints [i]) ;
				_out_weights.push_back (_in_weights [i]) ;
			}
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
	 //bExportControlPoints =true ;

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

	FbxAMatrix globalPosition;
	FbxTime pTime;
	FbxPose *pPose;
	const int pVertexCount = _pMesh->GetControlPointsCount();
	FbxVector4* pVertexArray = NULL;
	if (pVertexCount) {
		pVertexArray = new FbxVector4[pVertexCount];
		memcpy(pVertexArray, _pMesh->GetControlPoints(), pVertexCount * sizeof(FbxVector4));
	}
	const int pSkinCount = _pMesh->GetDeformerCount(FbxDeformer::eSkin);
	int pClusterCount = 0;
	for (int pSkinIndex = 0; pSkinIndex < pSkinCount; ++pSkinIndex)
	{
		pClusterCount += ((FbxSkin *)(_pMesh->GetDeformer(pSkinIndex, FbxDeformer::eSkin)))->GetClusterCount();
	}
	if (pClusterCount)
	{
		// Deform the vertex array with the skin deformer.
		ComputeSkinDeformation(globalPosition, _pMesh, pTime, pVertexArray, pPose);
	}

	// Initialize positions
	FbxArray<FbxVector4> positions (nbControlPoints) ;
	for(auto i=0; i < pVertexCount; i++)
		positions.InsertAt(i, pVertexArray[i]);

	return (positions) ;
}

// Function : GetLayerElements
FbxArray<FbxLayerElement::EType> myGetAllChannelUV (FbxMesh *_pMesh) {
	FbxArray<FbxLayerElement::EType> ret ;
	//FbxLayer *pLayer =gltfwriterVBO::getLayer (_pMesh, FbxLayerElement::eMaterial) ;
	//if ( pLayer == nullptr )
	//	return (ret) ;
	//FbxLayerElementMaterial *pLayerElementMaterial =pLayer->GetMaterials () ;
	//int materialCount =pLayerElementMaterial ? _pMesh->GetNode ()->GetMaterialCount () : 0 ;
	////if ( materialCount > 1 ) { // See gltfWriter-Mesh.cpp #169 - WriteMesh (FbxNode *pNode)
	//materialCount =materialCount == 0 ? 0 : 1 ;
	//for ( int i =0 ; i < materialCount ; i++ ) {
	//	FbxSurfaceMaterial *pMaterial =_pMesh->GetNode ()->GetMaterial (i) ;
	//	FbxSurfaceLambert *pLambertSurface =FbxCast<FbxSurfaceLambert> (pMaterial) ;
	//	if ( pLambertSurface == nullptr )
	//		continue ;
	//	

	//	if ( !property.IsValid () )
	//		return (ret) ;
	//	if ( property.GetSrcObjectCount<FbxTexture> () != 0 ) {
	//		FbxTexture *pTexture =property.GetSrcObject<FbxTexture> (0) ;
	//	}
	//}
	return (ret) ;
}

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

#define _GetAllChannelUV_ 1
#ifndef _GetAllChannelUV_
	// UV or Vertex Color
	int nbLayers =_pMesh->GetLayerCount () ;
	_pMesh->GetAllChannelUV (iLayer)
	//FbxArray<const FbxLayerElementUV*> hh =GetUVSets () ;

	//std::map<FbxLayerElement::EType, FbxLayerElementUV *> channels ;
	//for ( int index =FbxLayerElement::sTypeTextureStartIndex ; index < FbxLayerElement::sTypeTextureEndIndex ; index++ ) {
	//	FbxLayerElement::EType channel =static_cast<FbxLayerElement::EType>(index) ;
	//	FbxLayerElementUV *pLayerElementUVs =elementUVs (channel, -1) ;
	//	if ( pLayerElementUVs ) {
	//		utility::string_t st (U("TEXCOORD_0")) ;
	//		utility::string_t key =utility::conversions::to_string_t (FbxLayerElement::sTextureChannelNames [channel - FbxLayerElement::sTypeTextureStartIndex]) ;
	//		_uvSets [key] =st ;
	//	}
	//}
#else
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
					FbxLayerElement::sTextureChannelNames [channel - FbxLayerElement::sTypeTextureStartIndex]
				) ;
				_uvSets [key] =st ;
			}
		}
	}
#endif

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
			FbxVector4 position =vertices [vertexID] ; // _pMesh->GetControlPoints () [vertexID] ;
			_in_positions.push_back (position) ;

			if (_skinJointIndexes.size() && _skinVertexWeights.size()) {
				FbxVector4 indices;
				int jsize = _skinJointIndexes[vertexID].size();
				for(int j=0; j < jsize && j < 4; j++) {
					indices[j] = _skinJointIndexes[vertexID][j];

				}
				if (jsize < 4) { // append 0's
					for(int k=jsize; k < 4; k++) {
						indices[k] = 0;

					}
				}

				_in_joints.push_back (indices) ;

				FbxVector4 w;
				int size = _skinVertexWeights[vertexID].size();
				for(int j=0; j < size && j < 4; j++) {
					w[j] = _skinVertexWeights[vertexID][j];

				}
				if (size < 4) { // append 0's
					for(int k=size; k < 4; k++) {
						w[k] = 0;

					}
				}
				_in_weights.push_back (w) ;
				//std::cout << "VBO vertexID " << vertexID << std::endl;
			}

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

/*static*/ FbxLayer *gltfwriterVBO::getLayer (FbxMesh *_pMesh, FbxLayerElement::EType pType) {
	int nbLayers =_pMesh->GetLayerCount () ;
	for ( int iLayer =0 ; iLayer < nbLayers ; iLayer++ ) {
		FbxLayer *pLayer =_pMesh->GetLayer (iLayer, FbxLayerElement::eMaterial) ;
		if ( pLayer != nullptr )
			return (pLayer) ;
	}
	return (nullptr) ;
}

// Get the matrix of the given pose
FbxAMatrix gltfwriterVBO::GetPoseMatrix(FbxPose* pPose, int pNodeIndex)
{
    FbxAMatrix lPoseMatrix;
    FbxMatrix lMatrix = pPose->GetMatrix(pNodeIndex);

    memcpy((double*)lPoseMatrix, (double*)lMatrix, sizeof(lMatrix.mData));

    return lPoseMatrix;
}

// Get the geometry offset to a node. It is never inherited by the children.
FbxAMatrix gltfwriterVBO::GetGeometry(FbxNode* pNode)
{
    const FbxVector4 lT = pNode->GetGeometricTranslation(FbxNode::eSourcePivot);
    const FbxVector4 lR = pNode->GetGeometricRotation(FbxNode::eSourcePivot);
    const FbxVector4 lS = pNode->GetGeometricScaling(FbxNode::eSourcePivot);

    return FbxAMatrix(lT, lR, lS);
}

FbxAMatrix gltfwriterVBO::GetGlobalPosition(FbxNode* pNode, const FbxTime& pTime, FbxPose* pPose, FbxAMatrix* pParentGlobalPosition)
{
    FbxAMatrix lGlobalPosition;
    bool        lPositionFound = false;

    if (pPose)
    {
	    int lNodeIndex;
	    try {
		    lNodeIndex = pPose->Find(pNode);// __CRASH__ ???
	    } catch(...) {
		    std::cout<< "Unable to find the node" << __LINE__ << " : " << __FILE__ << std::endl; 
		    lNodeIndex = -1;	
	    } 

        if (lNodeIndex > -1)
        {
            // The bind pose is always a global matrix.
            // If we have a rest pose, we need to check if it is
            // stored in global or local space.
            if (pPose->IsBindPose() || !pPose->IsLocalMatrix(lNodeIndex))
            {
                lGlobalPosition = GetPoseMatrix(pPose, lNodeIndex);
            }
            else
            {
                // We have a local matrix, we need to convert it to
                // a global space matrix.
                FbxAMatrix lParentGlobalPosition;

                if (pParentGlobalPosition)
                {
                    lParentGlobalPosition = *pParentGlobalPosition;
                }
                else
                {
                    if (pNode->GetParent())
                    {
                        lParentGlobalPosition = GetGlobalPosition(pNode->GetParent(), pTime, pPose);
                    }
                }

                FbxAMatrix lLocalPosition = GetPoseMatrix(pPose, lNodeIndex);
                lGlobalPosition = lParentGlobalPosition * lLocalPosition;
            }

            lPositionFound = true;
        }
    }

    if (!lPositionFound)
    {
        // There is no pose entry for that node, get the current global position instead.

        // Ideally this would use parent global position and local position to compute the global position.
        // Unfortunately the equation 
        //    lGlobalPosition = pParentGlobalPosition * lLocalPosition
        // does not hold when inheritance type is other than "Parent" (RSrs).
        // To compute the parent rotation and scaling is tricky in the RrSs and Rrs cases.
        lGlobalPosition = pNode->EvaluateGlobalTransform(pTime);
    }

    return lGlobalPosition;
}

// Scale all the elements of a matrix.
void gltfwriterVBO::MatrixScale(FbxAMatrix& pMatrix, double pValue)
{
    int i,j;

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
        {
            pMatrix[i][j] *= pValue;
        }
    }
}


// Add a value to all the elements in the diagonal of the matrix.
void gltfwriterVBO::MatrixAddToDiagonal(FbxAMatrix& pMatrix, double pValue)
{
    pMatrix[0][0] += pValue;
    pMatrix[1][1] += pValue;
    pMatrix[2][2] += pValue;
    pMatrix[3][3] += pValue;
}


// Sum two matrices element by element.
void gltfwriterVBO::MatrixAdd(FbxAMatrix& pDstMatrix, FbxAMatrix& pSrcMatrix)
{
    int i,j;

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
        {
            pDstMatrix[i][j] += pSrcMatrix[i][j];
        }
    }
}
//Compute the transform matrix that the cluster will transform the vertex.
void gltfwriterVBO::ComputeClusterDeformation(FbxAMatrix& pGlobalPosition, 
							   FbxMesh* _pMesh,
							   FbxCluster* pCluster, 
							   FbxAMatrix& pVertexTransformMatrix,
							   FbxTime &pTime, 
							   FbxPose* pPose)
{
    FbxCluster::ELinkMode lClusterMode = pCluster->GetLinkMode();

	FbxAMatrix lReferenceGlobalInitPosition;
	FbxAMatrix lReferenceGlobalCurrentPosition;
	FbxAMatrix lAssociateGlobalInitPosition;
	FbxAMatrix lAssociateGlobalCurrentPosition;
	FbxAMatrix lClusterGlobalInitPosition;
	FbxAMatrix lClusterGlobalCurrentPosition;

	FbxAMatrix lReferenceGeometry;
	FbxAMatrix lAssociateGeometry;
	FbxAMatrix lClusterGeometry;

	FbxAMatrix lClusterRelativeInitPosition;
	FbxAMatrix lClusterRelativeCurrentPositionInverse;
	
	if (lClusterMode == FbxCluster::eAdditive && pCluster->GetAssociateModel())
	{
		pCluster->GetTransformAssociateModelMatrix(lAssociateGlobalInitPosition);
		// Geometric transform of the model
		lAssociateGeometry = GetGeometry(pCluster->GetAssociateModel());
		lAssociateGlobalInitPosition *= lAssociateGeometry;
		lAssociateGlobalCurrentPosition = GetGlobalPosition(pCluster->GetAssociateModel(), pTime, pPose);

		pCluster->GetTransformMatrix(lReferenceGlobalInitPosition);
		// Multiply lReferenceGlobalInitPosition by Geometric Transformation
		lReferenceGeometry = GetGeometry(_pMesh->GetNode());
		lReferenceGlobalInitPosition *= lReferenceGeometry;
		lReferenceGlobalCurrentPosition = pGlobalPosition;

		// Get the link initial global position and the link current global position.
		pCluster->GetTransformLinkMatrix(lClusterGlobalInitPosition);
		// Multiply lClusterGlobalInitPosition by Geometric Transformation
		lClusterGeometry = GetGeometry(pCluster->GetLink());
		lClusterGlobalInitPosition *= lClusterGeometry;
		lClusterGlobalCurrentPosition = GetGlobalPosition(pCluster->GetLink(), pTime, pPose);

		// Compute the shift of the link relative to the reference.
		//ModelM-1 * AssoM * AssoGX-1 * LinkGX * LinkM-1*ModelM
		pVertexTransformMatrix = lReferenceGlobalInitPosition.Inverse() * lAssociateGlobalInitPosition * lAssociateGlobalCurrentPosition.Inverse() *
			lClusterGlobalCurrentPosition * lClusterGlobalInitPosition.Inverse() * lReferenceGlobalInitPosition;
	}
	else
	{
		pCluster->GetTransformMatrix(lReferenceGlobalInitPosition);
		lReferenceGlobalCurrentPosition = pGlobalPosition;
		// Multiply lReferenceGlobalInitPosition by Geometric Transformation
		lReferenceGeometry = GetGeometry(_pMesh->GetNode());
		lReferenceGlobalInitPosition *= lReferenceGeometry;

		// Get the link initial global position and the link current global position.
		pCluster->GetTransformLinkMatrix(lClusterGlobalInitPosition);
		lClusterGlobalCurrentPosition = GetGlobalPosition(pCluster->GetLink(), pTime, pPose);

		// Compute the initial position of the link relative to the reference.
		lClusterRelativeInitPosition = lClusterGlobalInitPosition.Inverse() * lReferenceGlobalInitPosition;

		// Compute the current position of the link relative to the reference.
		lClusterRelativeCurrentPositionInverse = lReferenceGlobalCurrentPosition.Inverse() * lClusterGlobalCurrentPosition;

		// Compute the shift of the link relative to the reference.
		pVertexTransformMatrix = lClusterRelativeCurrentPositionInverse * lClusterRelativeInitPosition;

	}
			_inverseBindMatrices.push_back(pVertexTransformMatrix);
}
// Deform the vertex array in classic linear way.
void gltfwriterVBO::ComputeLinearDeformation(FbxAMatrix& pGlobalPosition, 
                               FbxMesh* _pMesh, 
                               FbxTime& pTime, 
                               FbxVector4* pVertexArray,
			       FbxPose* pPose)
{
	// All the links must have the same link mode.
	FbxCluster::ELinkMode lClusterMode = ((FbxSkin*)_pMesh->GetDeformer(0, FbxDeformer::eSkin))->GetCluster(0)->GetLinkMode();

	int lVertexCount = _pMesh->GetControlPointsCount();
	FbxAMatrix* lClusterDeformation = new FbxAMatrix[lVertexCount];
	memset(lClusterDeformation, 0, lVertexCount * sizeof(FbxAMatrix));

	double* lClusterWeight = new double[lVertexCount];
	memset(lClusterWeight, 0, lVertexCount * sizeof(double));

	if (lClusterMode == FbxCluster::eAdditive)
	{
		for (int i = 0; i < lVertexCount; ++i)
		{
			lClusterDeformation[i].SetIdentity();
		}
	}

	// For all skins and all clusters, accumulate their deformation and weight
	// on each vertices and store them in lClusterDeformation and lClusterWeight.
	int lSkinCount = _pMesh->GetDeformerCount(FbxDeformer::eSkin);
	for ( int lSkinIndex=0; lSkinIndex<lSkinCount; ++lSkinIndex)
	{
		FbxSkin * lSkinDeformer = (FbxSkin *)_pMesh->GetDeformer(lSkinIndex, FbxDeformer::eSkin);
		
		int lClusterCount = lSkinDeformer->GetClusterCount();
		for ( int lClusterIndex=0; lClusterIndex<lClusterCount; ++lClusterIndex)
		{
			FbxCluster* lCluster = lSkinDeformer->GetCluster(lClusterIndex);
			if (!lCluster->GetLink())
				continue;

			FbxAMatrix lVertexTransformMatrix;
			ComputeClusterDeformation(pGlobalPosition, _pMesh, lCluster, lVertexTransformMatrix, pTime, pPose);


			int lVertexIndexCount = lCluster->GetControlPointIndicesCount();
			for (int k = 0; k < lVertexIndexCount; ++k) 
			{            
				auto jointIndex=-1;
				int lIndex = lCluster->GetControlPointIndices()[k];
				double lWeight = lCluster->GetControlPointWeights()[k];

				// Sometimes, the mesh can have less points than at the time of the skinning
				// because a smooth operator was active when skinning but has been deactivated during export.
				if (lIndex >= lVertexCount)
					continue;
				if (lWeight == 0.0)
				{
					continue;
				}
				std::string findString(lCluster->GetLink()->GetName());
				std::transform (findString.begin(), findString.end(), findString.begin(), [](char ch) {
						return ch == ' ' ? '_' : ch;
						});
				auto it = std::find(_jointNames.begin(), _jointNames.end(), findString);
				if (it != _jointNames.end())
					jointIndex = std::distance(_jointNames.begin(), it);
				if (jointIndex == -1)
					jointIndex = 0;
				_skinJointIndexes[lIndex].push_back(jointIndex);
				_skinVertexWeights[lIndex].push_back(lWeight);

				// Compute the influence of the link on the vertex.
				FbxAMatrix lInfluence = lVertexTransformMatrix;
				MatrixScale(lInfluence, lWeight);

				if (lClusterMode == FbxCluster::eAdditive)
				{    
					// Multiply with the product of the deformations on the vertex.
					MatrixAddToDiagonal(lInfluence, 1.0 - lWeight);
					lClusterDeformation[lIndex] = lInfluence * lClusterDeformation[lIndex];

					// Set the link to 1.0 just to know this vertex is influenced by a link.
					lClusterWeight[lIndex] = 1.0;
				}
				else // lLinkMode == FbxCluster::eNormalize || lLinkMode == FbxCluster::eTotalOne
				{
					// Add to the sum of the deformations on the vertex.
					MatrixAdd(lClusterDeformation[lIndex], lInfluence);

					// Add to the sum of weights to either normalize or complete the vertex.
					lClusterWeight[lIndex] += lWeight;
				}
			}//For each vertex			
		}//lClusterCount
	}

	//Actually deform each vertices here by information stored in lClusterDeformation and lClusterWeight
	for (int i = 0; i < lVertexCount; i++) 
	{
		FbxVector4 lSrcVertex = pVertexArray[i];
		FbxVector4& lDstVertex = pVertexArray[i];
		double lWeight = lClusterWeight[i];

		// Deform the vertex if there was at least a link with an influence on the vertex,
		if (lWeight != 0.0) 
		{
			lDstVertex = lClusterDeformation[i].MultT(lSrcVertex);
			if (lClusterMode == FbxCluster::eNormalize)
			{
				// In the normalized link mode, a vertex is always totally influenced by the links. 
				lDstVertex /= lWeight;
			}
			else if (lClusterMode == FbxCluster::eTotalOne)
			{
				// In the total 1 link mode, a vertex can be partially influenced by the links. 
				lSrcVertex *= (1.0 - lWeight);
				lDstVertex += lSrcVertex;
			}
		} 
	}

	delete [] lClusterDeformation;
	delete [] lClusterWeight;
}

// Deform the vertex array in Dual Quaternion Skinning way.
void gltfwriterVBO::ComputeDualQuaternionDeformation(FbxAMatrix& pGlobalPosition, 
									 FbxMesh* _pMesh, 
									 FbxTime& pTime, 
									 FbxVector4* pVertexArray,
									 FbxPose* pPose)
{
	// All the links must have the same link mode.
	FbxCluster::ELinkMode lClusterMode = ((FbxSkin*)_pMesh->GetDeformer(0, FbxDeformer::eSkin))->GetCluster(0)->GetLinkMode();

	int lVertexCount = _pMesh->GetControlPointsCount();
	int lSkinCount = _pMesh->GetDeformerCount(FbxDeformer::eSkin);

	FbxDualQuaternion* lDQClusterDeformation = new FbxDualQuaternion[lVertexCount];
	memset(lDQClusterDeformation, 0, lVertexCount * sizeof(FbxDualQuaternion));

	double* lClusterWeight = new double[lVertexCount];
	memset(lClusterWeight, 0, lVertexCount * sizeof(double));

	// For all skins and all clusters, accumulate their deformation and weight
	// on each vertices and store them in lClusterDeformation and lClusterWeight.
	for ( int lSkinIndex=0; lSkinIndex<lSkinCount; ++lSkinIndex)
	{
		FbxSkin * lSkinDeformer = (FbxSkin *)_pMesh->GetDeformer(lSkinIndex, FbxDeformer::eSkin);
		int lClusterCount = lSkinDeformer->GetClusterCount();
		for ( int lClusterIndex=0; lClusterIndex<lClusterCount; ++lClusterIndex)
		{
			FbxCluster* lCluster = lSkinDeformer->GetCluster(lClusterIndex);
			if (!lCluster->GetLink())
				continue;

			FbxAMatrix lVertexTransformMatrix;
			ComputeClusterDeformation(pGlobalPosition, _pMesh, lCluster, lVertexTransformMatrix, pTime, pPose);

			FbxQuaternion lQ = lVertexTransformMatrix.GetQ();
			FbxVector4 lT = lVertexTransformMatrix.GetT();
			FbxDualQuaternion lDualQuaternion(lQ, lT);

			int lVertexIndexCount = lCluster->GetControlPointIndicesCount();
			for (int k = 0; k < lVertexIndexCount; ++k) 
			{ 
				int jointIndex = -1;
				int lIndex = lCluster->GetControlPointIndices()[k];
				double lWeight = lCluster->GetControlPointWeights()[k];

				// Sometimes, the mesh can have less points than at the time of the skinning
				// because a smooth operator was active when skinning but has been deactivated during export.
				if (lIndex >= lVertexCount)
					continue;
				if (lWeight == 0.0)
					continue;

				std::string findString(lCluster->GetLink()->GetName());
                                std::transform (findString.begin(), findString.end(), findString.begin(), [](char ch) {
                                                return ch == ' ' ? '_' : ch;
                                                });
                                auto it = std::find(_jointNames.begin(), _jointNames.end(), findString);
                                if (it != _jointNames.end())
                                        jointIndex = std::distance(_jointNames.begin(), it);
                                if (jointIndex == -1)
                                        jointIndex = 0;
                                _skinJointIndexes[lIndex].push_back(jointIndex);
                                _skinVertexWeights[lIndex].push_back(lWeight);
				// Compute the influence of the link on the vertex.
				FbxDualQuaternion lInfluence = lDualQuaternion * lWeight;
				if (lClusterMode == FbxCluster::eAdditive)
				{    
					// Simply influenced by the dual quaternion.
					lDQClusterDeformation[lIndex] = lInfluence;

					// Set the link to 1.0 just to know this vertex is influenced by a link.
					lClusterWeight[lIndex] = 1.0;
				}
				else // lLinkMode == FbxCluster::eNormalize || lLinkMode == FbxCluster::eTotalOne
				{
					if(lClusterIndex == 0)
					{
						lDQClusterDeformation[lIndex] = lInfluence;
					}
					else
					{
						// Add to the sum of the deformations on the vertex.
						// Make sure the deformation is accumulated in the same rotation direction. 
						// Use dot product to judge the sign.
						double lSign = lDQClusterDeformation[lIndex].GetFirstQuaternion().DotProduct(lDualQuaternion.GetFirstQuaternion());
						if( lSign >= 0.0 )
						{
							lDQClusterDeformation[lIndex] += lInfluence;
						}
						else
						{
							lDQClusterDeformation[lIndex] -= lInfluence;
						}
					}
					// Add to the sum of weights to either normalize or complete the vertex.
					lClusterWeight[lIndex] += lWeight;
				}
			}//For each vertex
		}//lClusterCount
	}

	//Actually deform each vertices here by information stored in lClusterDeformation and lClusterWeight
	for (int i = 0; i < lVertexCount; i++) 
	{
		FbxVector4 lSrcVertex = pVertexArray[i];
		FbxVector4& lDstVertex = pVertexArray[i];
		double lWeightSum = lClusterWeight[i];

		// Deform the vertex if there was at least a link with an influence on the vertex,
		if (lWeightSum != 0.0) 
		{
			lDQClusterDeformation[i].Normalize();
			lDstVertex = lDQClusterDeformation[i].Deform(lDstVertex);

			if (lClusterMode == FbxCluster::eNormalize)
			{
				// In the normalized link mode, a vertex is always totally influenced by the links. 
				lDstVertex /= lWeightSum;
			}
			else if (lClusterMode == FbxCluster::eTotalOne)
			{
				// In the total 1 link mode, a vertex can be partially influenced by the links. 
				lSrcVertex *= (1.0 - lWeightSum);
				lDstVertex += lSrcVertex;
			}
		} 
	}

	delete [] lDQClusterDeformation;
	delete [] lClusterWeight;
}
// Deform the vertex array according to the links contained in the mesh and the skinning type.
void gltfwriterVBO::ComputeSkinDeformation(FbxAMatrix& pGlobalPosition, 
									 FbxMesh* _pMesh, 
									 FbxTime& pTime, 
									 FbxVector4* pVertexArray,
									 FbxPose* pPose)
{
	FbxSkin * lSkinDeformer = (FbxSkin *)_pMesh->GetDeformer(0, FbxDeformer::eSkin);
	FbxSkin::EType lSkinningType = lSkinDeformer->GetSkinningType();

	if(lSkinningType == FbxSkin::eLinear || lSkinningType == FbxSkin::eRigid)
	{
		ComputeLinearDeformation(pGlobalPosition, _pMesh, pTime, pVertexArray, pPose);
	}
	else if(lSkinningType == FbxSkin::eDualQuaternion)
	{
		ComputeDualQuaternionDeformation(pGlobalPosition, _pMesh, pTime, pVertexArray, pPose);
	}
	else if(lSkinningType == FbxSkin::eBlend)
	{
		int lVertexCount = _pMesh->GetControlPointsCount();

		FbxVector4* lVertexArrayLinear = new FbxVector4[lVertexCount];
		memcpy(lVertexArrayLinear, _pMesh->GetControlPoints(), lVertexCount * sizeof(FbxVector4));

		FbxVector4* lVertexArrayDQ = new FbxVector4[lVertexCount];
		memcpy(lVertexArrayDQ, _pMesh->GetControlPoints(), lVertexCount * sizeof(FbxVector4));

		ComputeLinearDeformation(pGlobalPosition, _pMesh, pTime, lVertexArrayLinear, pPose);
		ComputeDualQuaternionDeformation(pGlobalPosition, _pMesh, pTime, lVertexArrayDQ, pPose);

		// To blend the skinning according to the blend weights
		// Final vertex = DQSVertex * blend weight + LinearVertex * (1- blend weight)
		// DQSVertex: vertex that is deformed by dual quaternion skinning method;
		// LinearVertex: vertex that is deformed by classic linear skinning method;
		int lBlendWeightsCount = lSkinDeformer->GetControlPointIndicesCount();
		for(int lBWIndex = 0; lBWIndex<lBlendWeightsCount; ++lBWIndex)
		{
			double lBlendWeight = lSkinDeformer->GetControlPointBlendWeights()[lBWIndex];
			pVertexArray[lBWIndex] = lVertexArrayDQ[lBWIndex] * lBlendWeight + lVertexArrayLinear[lBWIndex] * (1 - lBlendWeight);
		}
	}
}

#ifdef __XX__
//-----------------------------------------------------------------------------
template<class T>
gltfwriterVBOT<T>::gltfwriterVBO (T *_pMesh) {
	_pMesh =_pMesh ;
}

// Function    : getSimilarVertexIndex
// Abstraction : Find the Packed Vertex from the existing map, if found return true else return false
template<class T>
bool gltfwriterVBOT<T>::getSimilarVertexIndex (PackedVertex &packed, std::map<PackedVertex, unsigned short> &VertexToOutIndex, unsigned short &result) {
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
template<class T>
void gltfwriterVBOT<T>::indexVBO () {
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
template<class T>
FbxArray<FbxVector4> gltfwriterVBOT<T>::GetVertexPositions (bool bInGeometry, bool bExportControlPoints) {
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

// Function : GetLayerElements
template<class T>
FbxArray<FbxLayerElement<T>::EType> myGetAllChannelUVT (T *_pMesh) {
	FbxArray<FbxLayerElement::EType> ret ;
	//FbxLayer *pLayer =gltfwriterVBO::getLayer (_pMesh, FbxLayerElement::eMaterial) ;
	//if ( pLayer == nullptr )
	//	return (ret) ;
	//FbxLayerElementMaterial *pLayerElementMaterial =pLayer->GetMaterials () ;
	//int materialCount =pLayerElementMaterial ? _pMesh->GetNode ()->GetMaterialCount () : 0 ;
	////if ( materialCount > 1 ) { // See gltfWriter-Mesh.cpp #169 - WriteMesh (FbxNode *pNode)
	//materialCount =materialCount == 0 ? 0 : 1 ;
	//for ( int i =0 ; i < materialCount ; i++ ) {
	//	FbxSurfaceMaterial *pMaterial =_pMesh->GetNode ()->GetMaterial (i) ;
	//	FbxSurfaceLambert *pLambertSurface =FbxCast<FbxSurfaceLambert> (pMaterial) ;
	//	if ( pLambertSurface == nullptr )
	//		continue ;
	//	

	//	if ( !property.IsValid () )
	//		return (ret) ;
	//	if ( property.GetSrcObjectCount<FbxTexture> () != 0 ) {
	//		FbxTexture *pTexture =property.GetSrcObject<FbxTexture> (0) ;
	//	}
	//}
	return (ret) ;
}

template<class T>
void gltfwriterVBOT<T>::GetLayerElements (bool bInGeometry /*=true*/) {

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

#define _GetAllChannelUV_ 1
#ifndef _GetAllChannelUV_
	// UV or Vertex Color
	int nbLayers =_pMesh->GetLayerCount () ;
	_pMesh->GetAllChannelUV (iLayer)
	//FbxArray<const FbxLayerElementUV*> hh =GetUVSets () ;

	//std::map<FbxLayerElement::EType, FbxLayerElementUV *> channels ;
	//for ( int index =FbxLayerElement::sTypeTextureStartIndex ; index < FbxLayerElement::sTypeTextureEndIndex ; index++ ) {
	//	FbxLayerElement::EType channel =static_cast<FbxLayerElement::EType>(index) ;
	//	FbxLayerElementUV *pLayerElementUVs =elementUVs (channel, -1) ;
	//	if ( pLayerElementUVs ) {
	//		utility::string_t st (U("TEXCOORD_0")) ;
	//		utility::string_t key =utility::conversions::to_string_t (FbxLayerElement::sTextureChannelNames [channel - FbxLayerElement::sTypeTextureStartIndex]) ;
	//		_uvSets [key] =st ;
	//	}
	//}
#else
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
					FbxLayerElement::sTextureChannelNames [channel - FbxLayerElement::sTypeTextureStartIndex]
				) ;
				_uvSets [key] =st ;
			}
		}
	}
#endif

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
			FbxVector4 position =vertices [vertexID] ; // _pMesh->GetControlPoints () [vertexID] ;
			_in_positions.push_back (position) ;
std::cout<<"before GetLayerElement" << std::endl;

			GetLayerElement (pLayerElementNormals, normalIndex, FbxVector4, normal, index, [this] (FbxVector4 &V) {
				_in_normals.push_back (V) ;
			}) ;
std::cout<<"after GetLayerElement" << std::endl;
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

template<class T>
FbxLayerElementNormal *gltfwriterVBOT<T>::elementNormals (int iLayer /*=-1*/)  {
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

template<class T>
FbxLayerElementUV *gltfwriterVBOT<T>::elementUVs (FbxLayerElement::EType channel, int iLayer /*=-1*/) {
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

template<class T>
FbxGeometryElementTangent *gltfwriterVBOT<T>::elementTangents (int iLayer /*=-1*/) {
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

template<class T>
FbxGeometryElementBinormal *gltfwriterVBOT<T>::elementBinormals (int iLayer /*=-1*/) {
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

template<class T>
FbxLayerElementVertexColor *gltfwriterVBOT<T>::elementVcolors (int iLayer /*=-1*/) {
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

template<T>
/*static*/ FbxLayer *gltfwriterVBOT<T>::getLayer (T *_pMesh, FbxLayerElement::EType pType) {
	int nbLayers =_pMesh->GetLayerCount () ;
	for ( int iLayer =0 ; iLayer < nbLayers ; iLayer++ ) {
		FbxLayer *pLayer =_pMesh->GetLayer (iLayer, FbxLayerElement::eMaterial) ;
		if ( pLayer != nullptr )
			return (pLayer) ;
	}
	return (nullptr) ;
}
#endif

}
