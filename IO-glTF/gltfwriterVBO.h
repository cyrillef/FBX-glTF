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

// Function    : GetLayerElement
// Abstraction : From the given layer get the single element
#define GetLayerElement(layerElt, indexV, T, V, index, lambda) \
	int indexV =-1 ; \
	T V ; \
	if ( layerElt ) { \
		indexV =(layerElt->GetReferenceMode () == FbxLayerElement::eDirect ? index : layerElt->GetIndexArray () [index]) ; \
		V =layerElt->GetDirectArray ().GetAt (indexV) ; \
		lambda (V) ; \
	}

class gltfwriterVBO {
	struct PackedVertex {
		FbxDouble3 _position ;
		FbxDouble4 _joint ;
		FbxDouble4 _weight;
		FbxDouble2 _uv ;
		FbxDouble3 _normal ;
		FbxDouble3 _tangent ;
		FbxDouble3 _binormal ;
		FbxColor _vcolor ;
		bool operator<(const PackedVertex that) const {
			return (memcmp ((void *)this, (void *)&that, sizeof (PackedVertex)) > 0) ;
		} ;
	} ;
	std::vector<unsigned short> _in_indices, _out_indices ;
	std::vector<FbxDouble3> _in_positions, _out_positions ; // babylon.js does not like homogeneous coordinates (i.e. FbxDouble4)
	std::vector<FbxDouble4> _in_joints, _out_joints ; 
	std::vector<FbxDouble4> _in_weights, _out_weights ; 
	std::vector<FbxDouble2> _in_uvs, _out_uvs ;
	std::vector<FbxDouble3> _in_normals, _out_normals ;
	std::vector<FbxDouble3> _in_tangents, _out_tangents ;
	std::vector<FbxDouble3> _in_binormals, _out_binormals ;
	std::vector<FbxColor> _in_vcolors, _out_vcolors ;
	std::map<utility::string_t, utility::string_t> _uvSets ;
	FbxMesh *_pMesh ;
	std::vector<std::string> _jointNames;
	std::vector<FbxAMatrix> _inverseBindMatrices;
	std::map<int, std::vector<double>> _skinJointIndexes;
        std::map<int, std::vector<double>> _skinVertexWeights;
	void MatrixAdd(FbxAMatrix& pDstMatrix, FbxAMatrix& pSrcMatrix);
	void MatrixAddToDiagonal(FbxAMatrix& pMatrix, double pValue);
	void MatrixScale(FbxAMatrix& pMatrix, double pValue);
	FbxAMatrix GetGeometry(FbxNode* pNode);
	FbxAMatrix GetPoseMatrix(FbxPose* pPose, int pNodeIndex);
	FbxAMatrix GetGlobalPosition(FbxNode* pNode, const FbxTime& pTime, FbxPose* pPose = NULL, FbxAMatrix* pParentGlobalPosition = NULL);

public:
	gltfwriterVBO (FbxMesh *pMesh) { _pMesh =pMesh ; }

	void GetLayerElements (bool bInGeometry) ;
	FbxArray<FbxVector4> GetVertexPositions (bool bInGeometry, bool bExportControlPoints) ;
	bool getSimilarVertexIndex (PackedVertex & packed, std::map<PackedVertex, unsigned short> &VertexToOutIndex, unsigned short &result) ;
	void indexVBO () ;

	std::vector<unsigned short> getIndices () { return (_out_indices) ; }
	std::vector<FbxDouble3> getPositions () { return (_out_positions) ; }
	std::vector<FbxDouble4> getJoints () { return (_out_joints) ; }
	std::vector<FbxDouble4> getWeights() { return (_out_weights) ; }
	std::vector<FbxDouble2> getUvs () { return (_out_uvs) ; }
	std::vector<FbxDouble3> getNormals () { return( _out_normals) ; }
	std::vector<FbxDouble3> getTangents () { return (_out_tangents) ; }
	std::vector<FbxDouble3> getBinormals () { return (_out_binormals) ; }
	std::vector<FbxColor> getVertexColors () { return (_out_vcolors) ; }
	std::map<utility::string_t, utility::string_t> getUvSets () { return (_uvSets) ; }
	void setJointNames(std::vector<std::string> jointNames) { _jointNames = jointNames; }
	std::vector<std::string> getJointNames() { return _jointNames; }
	std::vector<FbxAMatrix> getInverseBindMatrices() { return _inverseBindMatrices;}
	

protected:
	FbxLayerElementNormal *elementNormals (int iLayer =-1) ;
	FbxLayerElementUV *elementUVs (FbxLayerElement::EType channel, int iLayer =-1) ;
	FbxGeometryElementTangent *elementTangents (int iLayer =-1)  ;
	FbxGeometryElementBinormal *elementBinormals (int iLayer =-1) ;
	FbxLayerElementVertexColor *elementVcolors (int iLayer =-1) ;
public:
	static FbxLayer *getLayer (FbxMesh *pMesh, FbxLayerElement::EType pType) ;
	void ComputeClusterDeformation(FbxAMatrix& pGlobalPosition, FbxMesh* pMesh, FbxCluster* pCluster, FbxAMatrix& pVertexTransformMatrix, FbxTime &pTime, FbxPose* pPose);
	void ComputeLinearDeformation(FbxAMatrix& pGlobalPosition, FbxMesh* pMesh, FbxTime& pTime, FbxVector4* pVertexArray, FbxPose* pPose);
	void ComputeDualQuaternionDeformation(FbxAMatrix& pGlobalPosition, FbxMesh* pMesh, FbxTime& pTime, FbxVector4* pVertexArray, FbxPose* pPose);
	void ComputeSkinDeformation(FbxAMatrix& pGlobalPosition, FbxMesh* pMesh, FbxTime& pTime, FbxVector4* pVertexArray, FbxPose* pPose);
};
#ifdef __XX__
template<class T>
class gltfwriterVBOT {
	struct PackedVertex {
		FbxDouble3 _position ;
		FbxDouble2 _uv ;
		FbxDouble3 _normal ;
		FbxDouble3 _tangent ;
		FbxDouble3 _binormal ;
		FbxColor _vcolor ;
		bool operator<(const PackedVertex that) const {
			return (memcmp ((void *)this, (void *)&that, sizeof (PackedVertex)) > 0) ;
		} ;
	} ;
	std::vector<unsigned short> _in_indices, _out_indices ;
	std::vector<FbxDouble3> _in_positions, _out_positions ; // babylon.js does not like homogeneous coordinates (i.e. FbxDouble4)
	std::vector<FbxDouble2> _in_uvs, _out_uvs ;
	std::vector<FbxDouble3> _in_normals, _out_normals ;
	std::vector<FbxDouble3> _in_tangents, _out_tangents ;
	std::vector<FbxDouble3> _in_binormals, _out_binormals ;
	std::vector<FbxColor> _in_vcolors, _out_vcolors ;
	std::map<utility::string_t, utility::string_t> _uvSets ;
	T *_pMesh ;

public:
	gltfwriterVBO (T *pMesh) ;

	void GetLayerElements (bool bInGeometry) ;
	FbxArray<FbxVector4> GetVertexPositions (bool bInGeometry, bool bExportControlPoints) ;
	bool getSimilarVertexIndex (PackedVertex & packed, std::map<PackedVertex, unsigned short> &VertexToOutIndex, unsigned short &result) ;
	void indexVBO () ;

	std::vector<unsigned short> getIndices () { return (_out_indices) ; }
	std::vector<FbxDouble3> getPositions () { return (_out_positions) ; }
	std::vector<FbxDouble2> getUvs () { return (_out_uvs) ; }
	std::vector<FbxDouble3> getNormals () { return( _out_normals) ; }
	std::vector<FbxDouble3> getTangents () { return (_out_tangents) ; }
	std::vector<FbxDouble3> getBinormals () { return (_out_binormals) ; }
	std::vector<FbxColor> getVertexColors () { return (_out_vcolors) ; }
	std::map<utility::string_t, utility::string_t> getUvSets () { return (_uvSets) ; }

protected:
	FbxLayerElementNormal *elementNormals (int iLayer =-1) ;
	FbxLayerElementUV *elementUVs (FbxLayerElement::EType channel, int iLayer =-1) ;
	FbxGeometryElementTangent *elementTangents (int iLayer =-1)  ;
	FbxGeometryElementBinormal *elementBinormals (int iLayer =-1) ;
	FbxLayerElementVertexColor *elementVcolors (int iLayer =-1) ;
public:
	static FbxLayer *getLayer (T *pMesh, FbxLayerElement::EType pType) ;

} ;
#endif

}
