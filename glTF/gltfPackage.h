//
// Copyright (c) Autodesk, Inc. All rights reserved 
//
// C++ glTF FBX converter
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

#include <fbxsdk.h>
//#include "webgl-idl.h"

//-----------------------------------------------------------------------------
class fbxSdkMgr {
	static FbxAutoPtr<fbxSdkMgr> _singleton ;
protected:
	FbxSharedDestroyPtr<FbxManager> _sdkManager ;

protected:
	fbxSdkMgr () ;
public:
	virtual ~fbxSdkMgr () ;

public:
	static FbxAutoPtr<fbxSdkMgr> &Instance () ;
	FbxSharedDestroyPtr<FbxManager> &fbxMgr () ;

} ;

//-----------------------------------------------------------------------------
class gltfPackage {
protected:
	struct IOSettings {
		utility::string_t _name ;
	} _ioSettings ;

protected:
	FbxAutoDestroyPtr<FbxScene> _scene ;

public:
	gltfPackage () ;
	virtual ~gltfPackage () ;

	void ioSettings (
		const utility::char_t *name =nullptr,
		bool angleInDegree =false,
		bool reverseTransparency =false,
		bool defaultLighting =false,
		bool copyMedia =false,
		bool embedMedia =false
	) ;

	bool load (const utility::string_t &fn) ;
	bool import (const utility::string_t &fn) ;

	bool save (const utility::string_t &outdir) ;

	static utility::string_t filename (const utility::string_t &path) ;
	static utility::string_t pathname (const utility::string_t &filename) ;
	
protected:
	bool LoadScene (const utility::string_t &fn) ;
	bool WriteScene (const utility::string_t &outdir) ;

} ;
