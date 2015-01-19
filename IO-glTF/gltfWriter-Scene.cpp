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

// https://github.com/KhronosGroup/glTF/blob/master/specification/scene.schema.json
bool gltfWriter::WriteScene (FbxScene *pScene, int poseIndex /*=-1*/) {
	FbxNode *pRoot =pScene->GetRootNode () ;
	FbxPose *pPose =poseIndex >= 0 ? pScene->GetPose (poseIndex) : nullptr ;
	utility::string_t szName =utility::conversions::to_string_t (pScene->GetName ()) ;
	_json [U("scene")] =web::json::value::string (szName) ;
	_json [U("scenes")] =web::json::value::object ({ { szName, web::json::value::object ({ { U("nodes"), web::json::value::array () } }) } }) ;

	//FbxDouble3 translation =pRoot->LclTranslation.Get () ;
	//FbxDouble3 rotation =pRoot->LclRotation.Get () ;
	//FbxDouble3 scaling =pRoot->LclScaling.Get () ;

	WriteSceneNodeRecursive (pRoot, pPose, true) ;
	return (true) ;
}

}
