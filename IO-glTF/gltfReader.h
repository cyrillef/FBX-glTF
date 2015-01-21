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
#pragma once

namespace _IOglTF_NS_ {

class gltfReader : public FbxReader {

private:
	std::wifstream _in ;
	//FbxManager*	mManager ;

public:
	gltfReader (FbxManager &pManager, int pID) ;
	virtual ~gltfReader () ;

	virtual void GetVersion (int &pMajor, int &pMinor, int &pRevision) ;
	virtual bool FileOpen (char  *pFileName) ;
	virtual bool FileClose () ;
	virtual bool IsFileOpen () ;
	virtual bool GetReadOptions (bool pParseFileAsNeeded =true) ;
	virtual bool Read (FbxDocument *pDocument) ;

	static FbxReader *Create_gltfReader (FbxManager &pManager, FbxImporter &pImporter, int pSubID, int pPluginID) ;
	static void *gltfFormatInfo (FbxReader::EInfoRequest pRequest, int pId) ;
	static void FillIOSettings (FbxIOSettings &pIOS) ;

protected:

} ;

}
