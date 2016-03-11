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

web::json::value gltfWriter::WriteProgram (FbxNode *pNode, FbxSurfaceMaterial *pMaterial, utility::string_t programName, web::json::value &attributes) {
	web::json::value programAttributes =web::json::value::array () ;
	for ( const auto &iter : attributes.as_object () )
		programAttributes [programAttributes.size ()] =web::json::value::string (iter.first) ;

	// Get the implementation to see if it's a hardware shader.
	if ( pMaterial != nullptr ) {
		//const FbxImplementation *pImplementation =GetImplementation (pMaterial, FBXSDK_IMPLEMENTATION_HLSL) ;
		//if ( !pImplementation )
		//	pImplementation =GetImplementation (pMaterial, FBXSDK_IMPLEMENTATION_CGFX) ;
		//if ( pImplementation ) {
		//	const FbxBindingTable *pRootTable =pImplementation->GetRootTable () ;
		//	FbxString fileName =pRootTable->DescAbsoluteURL.Get () ;
		//	FbxString pTechniqueName =pRootTable->DescTAG.Get () ;
		//	const FbxBindingTable *pTable =pImplementation->GetRootTable () ;
		//	size_t entryNum =pTable->GetEntryCount () ;
		//	for ( size_t i =0 ; i < entryNum ; i++ ) {
		//		const FbxBindingTableEntry &entry =pTable->GetEntry (i) ;
		//		const char *pszEntrySrcType =entry.GetEntryType (true) ;
		//	}
		//} else {
		//	int nb =pNode->GetImplementationCount () ;
		//	//pImplementation =pNode->GetImplementation (0) ;
		//}
	}
	FbxString filename =FbxPathUtils::GetFileName (utility::conversions::to_utf8string (_fileName).c_str (), false) ;

	web::json::value program =web::json::value::object ({
		{ U("attributes"), programAttributes },
		{ U("name"), web::json::value::string (programName) },
		//{ U("fragmentShader"), web::json::value::string (utility::conversions::to_string_t (filename.Buffer ()) + U("0FS")) },
		//{ U("vertexShader"), web::json::value::string (utility::conversions::to_string_t (filename.Buffer ()) + U("0VS")) }
		{ U("fragmentShader"), web::json::value::string (programName + U("FS")) },
		{ U("vertexShader"), web::json::value::string (programName + U("VS")) }
	}) ;
	web::json::value lib =web::json::value::object ({{ programName, program }}) ;

	web::json::value shaders =WriteShaders (pNode, program) ;

	return (web::json::value::object ({ { U("programs"), lib }, { U("shaders"), shaders } })) ;
}

web::json::value gltfWriter::WriteShaders (FbxNode *pNode, web::json::value &program) {
	web::json::value fragmentShader =web::json::value::object ({
		{ U("type"), web::json::value::number ((int)IOglTF::FRAGMENT_SHADER) },
		{ U("uri"), web::json::value::string (program [U("fragmentShader")].as_string () + U(".glsl")) }
	}) ;
	web::json::value vertexShader =web::json::value::object ({
		{ U ("type"), web::json::value::number ((int)IOglTF::VERTEX_SHADER) },
		{ U ("uri"), web::json::value::string (program [U("vertexShader")].as_string () + U(".glsl")) }
	}) ;
	return (web::json::value::object ({
		{ program [U("fragmentShader")].as_string (), fragmentShader },
		{ program [U ("vertexShader")].as_string (), vertexShader }
	})) ;
}

}
