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

// https://github.com/KhronosGroup/glTF/blob/master/specification/asset.schema.json
bool gltfWriter::WriteAsset (FbxDocumentInfo *pSceneInfo) {
	web::json::value asset =web::json::value::object () ;

	// unit - <meter> and <name>. In FBX we always work in centimeters, but we already converted to meter here
	//double scale =_scene->GetGlobalSettings ().GetSystemUnit ().GetScaleFactor () / 100. ;

	// Up axis
	//FbxScene *pScene =sceneInfo->GetScene () ;
	//FbxAxisSystem axisSystem =pScene->GetGlobalSettings ().GetAxisSystem () ;
	//int upAxisSign =0 ;
	//axisSystem.GetUpVector (upAxisSign) ;
	//if ( upAxisSign < 0 )
	//	GetStatus ().SetCode (FbxStatus::eFailure, "Invalid direction for up-axis: exporter should convert scene!") ;
	//if ( axisSystem.GetCoorSystem () != FbxAxisSystem::eRightHanded )
	//	GetStatus ().SetCode (FbxStatus::eFailure, "Axis system is Left Handed: exporter should convert scene!") ;

	// FBX uses author and comments, not authoring_tool(i.e. generator), and copyright.
	asset [U ("copyright")] =web::json::value::string (utility::conversions::to_string_t (pSceneInfo->mAuthor.Buffer ())) ;
	asset [U ("generator")] =web::json::value::string (FBX_GLTF_EXPORTER) ;
	if ( _writeDefaults )
		asset [U ("premultipliedAlpha")] =web::json::value::boolean (false) ;
	if ( _writeDefaults )
		asset [U ("profile")] =web::json::value::string (WebGL_1_0_2) ; // default WebGL 1.0.2
	asset [U ("version")] =web::json::value::number (GLTF_VERSION) ; // default 0.6
	//todo really a number double is no 0.8 but 0.8000000004

	_json [U ("asset")] =asset ;
	return (true) ;
}

}
