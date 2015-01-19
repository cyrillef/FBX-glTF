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
#include "gltfReader.h"

namespace _IOglTF_NS_ {

//-----------------------------------------------------------------------------
gltfReader::gltfReader (FbxManager &pManager, int pID) : FbxReader(pManager, pID, FbxStatusGlobal::GetRef ()) {
}

gltfReader::~gltfReader () {
}

void gltfReader::GetVersion (int &pMajor, int &pMinor, int &pRevision) {
	pMajor =0 ;
	pMinor =8 ;
	pRevision=0 ;
}

bool gltfReader::FileOpen (char *pFileName) {
	_in =std::wifstream (pFileName, std::ios::in) ;
	return (IsFileOpen ()) ;
}
bool gltfReader::FileClose () {
	_in.close () ;
	return (true) ;
}

bool gltfReader::IsFileOpen () {
	return (_in.is_open ()) ;
}

bool gltfReader::GetReadOptions (bool pParseFileAsNeeded) {
	return (true) ;
}

bool gltfReader::Read (FbxDocument *pDocument) {
	FbxScene *pScene =FbxCast<FbxScene> (pDocument) ;
	FbxNode *pRoot =pScene->GetRootNode () ;

	return (true) ;
}

// Creates a gltfReader in the Sdk Manager
/*static*/ FbxReader *gltfReader::Create_gltfReader (FbxManager &pManager, FbxImporter &pImporter, int pSubID, int pPluginID) {
	FbxReader *reader =FbxNew<gltfReader> (pManager, pPluginID) ; // use FbxNew instead of new, since FBX will take charge its deletion
	reader->SetIOSettings (pImporter.GetIOSettings ()) ;
	return (reader) ;
}

// Get extension, description or version info about gltfReader
/*static*/ void *gltfReader::gltfFormatInfo (FbxReader::EInfoRequest pRequest, int pId) {
	switch ( pRequest ) {
		case FbxReader::eInfoExtension:
			return (_IOglTF_NS_::_gltfFormatInfo (FbxWriter::eInfoExtension, pId)) ;
		case FbxReader::eInfoDescriptions:
			return (_IOglTF_NS_::_gltfFormatInfo (FbxWriter::eInfoDescriptions, pId)) ;
		default:
			return (0) ;
	}
}

/*static*/ void gltfReader::FillIOSettings (FbxIOSettings &pIOS) {
	// Here you can write your own FbxIOSettings and parse them.
	// Example at: http://help.autodesk.com/view/FBX/2015/ENU/?guid=__files_GUID_75CD0DC4_05C8_4497_AC6E_EA11406EAE26_htm
}

//-----------------------------------------------------------------------------

}
