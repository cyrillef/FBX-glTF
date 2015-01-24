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
#include "StdAfx.h"
#include "glTF.h"
#include "gltfPackage.h"

//-----------------------------------------------------------------------------
/*static*/ FbxAutoPtr<fbxSdkMgr> fbxSdkMgr::_singleton ;

fbxSdkMgr::fbxSdkMgr () : _sdkManager(FbxManager::Create ()) {
	_ASSERTE( _sdkManager ) ;
	FbxIOSettings *pIOSettings =FbxIOSettings::Create (_sdkManager, IOSROOT) ;
	//pIOSettings->SetBoolProp (IMP_FBX_CONSTRAINT, true) ;
	//pIOSettings->SetBoolProp (IMP_FBX_CONSTRAINT_COUNT, true) ;
	//pIOSettings->SetBoolProp (IMP_FBX_MATERIAL, true) ;
	//pIOSettings->SetBoolProp (IMP_FBX_TEXTURE, true) ;
	//pIOSettings->SetBoolProp (IMP_FBX_LINK, true) ;
	//pIOSettings->SetBoolProp (IMP_FBX_SHAPE, true) ;
	//pIOSettings->SetBoolProp (IMP_FBX_GOBO, false) ;
	//pIOSettings->SetBoolProp (IMP_FBX_ANIMATION, true) ;
	//pIOSettings->SetBoolProp (IMP_FBX_GLOBAL_SETTINGS, true) ;
	pIOSettings->SetBoolProp (EXP_FBX_EMBEDDED, false) ;
	_sdkManager->SetIOSettings (pIOSettings) ;

	// Load plug-ins from the executable directory
	FbxString path =FbxGetApplicationDirectory () ;
#if defined(_WIN64) || defined(_WIN32)
	FbxString extension ("dll") ;
#elif defined(__APPLE__)
	FbxString extension ("dylib") ;
#else // __linux
	FbxString extension ("so") ;
#endif
	_sdkManager->LoadPluginsDirectory (path.Buffer ()/*, extension.Buffer ()*/) ;
	_sdkManager->FillIOSettingsForReadersRegistered (*pIOSettings) ;
	_sdkManager->FillIOSettingsForWritersRegistered (*pIOSettings) ;
}

fbxSdkMgr::~fbxSdkMgr () {
}

/*static*/ FbxAutoPtr<fbxSdkMgr> &fbxSdkMgr::Instance () {
	if ( !fbxSdkMgr::_singleton )
		fbxSdkMgr::_singleton.Reset (new fbxSdkMgr ()) ;
	return (fbxSdkMgr::_singleton) ;
}

FbxSharedDestroyPtr<FbxManager> &fbxSdkMgr::fbxMgr () {
	_ASSERTE( _sdkManager ) ;
	return (_sdkManager) ;
}

//-----------------------------------------------------------------------------
gltfPackage::gltfPackage () : _scene(nullptr), _ioSettings ({ U("") }) {
}

gltfPackage::~gltfPackage () {
}

void gltfPackage::ioSettings (
	const utility::char_t *name /*=nullptr*/,
	bool invertTransparency /*=false*/,
	bool defaultLighting /*=false*/,
	bool copyMedia /*=false*/,
	bool embedMedia /*=false*/
) {
	FbxIOSettings *pIOSettings =fbxSdkMgr::Instance ()->fbxMgr ()->GetIOSettings () ;
	_ioSettings._name =name == nullptr ? U("") : name ;
	pIOSettings->SetBoolProp (IOSN_FBX_GLTF_INVERTTRANSPARENCY, invertTransparency) ;
	pIOSettings->SetBoolProp (IOSN_FBX_GLTF_DEFAULTLIGHTING, defaultLighting) ;
	pIOSettings->SetBoolProp (IOSN_FBX_GLTF_COPYMEDIA, copyMedia) ;
	pIOSettings->SetBoolProp (IOSN_FBX_GLTF_EMBEDMEDIA, embedMedia) ;
	//fbxSdkMgr::Instance ()->fbxMgr ()->SetIOSettings (pIOSettings) ;
}

bool gltfPackage::load (const utility::string_t &fn) {
	_ASSERTE( !_scene ) ;
	auto pMgr =fbxSdkMgr::Instance ()->fbxMgr () ;
	_scene.Reset (FbxScene::Create (pMgr, utility::conversions::to_utf8string (gltfPackage::filename (fn)).c_str ())) ;
	_ASSERTE( !!_scene ) ;
	bool bRet =LoadScene (fn) ;
	return (bRet) ;
}

bool gltfPackage::import (const utility::string_t &fn) {
	_ASSERTE( !_scene ) ;
	auto pMgr =fbxSdkMgr::Instance ()->fbxMgr () ;
	_scene.Reset (FbxScene::Create (pMgr, utility::conversions::to_utf8string (gltfPackage::filename (fn)).c_str ())) ;
	_ASSERTE( !!_scene ) ;
	bool bRet =LoadScene (fn) ;
	return (bRet) ;
}

bool gltfPackage::save (const utility::string_t &outdir) {
	_ASSERTE( !!_scene ) ;
	bool bRet =WriteScene (outdir) ;
	return (bRet) ;
}

/*static*/ utility::string_t gltfPackage::filename (const utility::string_t &path) {
	utility::string_t filename ;
	size_t pos =path.find_last_of (U("/\\")) ;
	if ( pos != utility::string_t::npos ) {
		size_t pos2 =path.find (U("."), pos) ;
		if ( pos2 != utility::string_t::npos )
			filename.assign (path.begin () + pos + 1, path.begin () + pos2) ;
		else
			filename.assign (path.begin () + pos + 1, path.end ()) ;
	} else {
		filename =path ;
	}
	return (filename) ;
}

/*static*/ utility::string_t gltfPackage::pathname (const utility::string_t &filename) {
	utility::string_t path ;
	size_t pos =filename.find_last_of (U ("/\\")) ;
	if ( pos != utility::string_t::npos )
		path.assign (filename.begin (), filename.begin () + pos + 1) ;
	else
		path =U("") ;
	return (path) ;
}

bool gltfPackage::LoadScene (const utility::string_t &fn) {
	auto pMgr =fbxSdkMgr::Instance ()->fbxMgr () ;
	FbxAutoDestroyPtr<FbxImporter> pImporter (FbxImporter::Create (pMgr, "")) ;

	if ( !pImporter->Initialize (utility::conversions::to_utf8string (fn).c_str (), -1, pMgr->GetIOSettings ()) )
		return (false) ;
	if ( pImporter->IsFBX () ) {
		// From this point, it is possible to access animation stack information without
		// the expense of loading the entire file.

		// Set the import states. By default, the import states are always set to true.
	}

	bool bStatus =pImporter->Import (_scene) ;
	if ( _ioSettings._name.length () )
		_scene->SetName (utility::conversions::to_utf8string (_ioSettings._name).c_str ()) ;
	else if ( _scene->GetName () == FbxString ("") )
		//_scene->SetName ("untitled") ;
		_scene->SetName (utility::conversions::to_utf8string (gltfPackage::filename (fn)).c_str ()) ;

	//if ( bStatus == false && pImporter->GetStatus ().GetCode () == FbxStatus::ePasswordError ) {
	//}

	FbxAxisSystem::MayaYUp.ConvertScene (_scene) ;
	FbxSystemUnit sceneSystemUnit =_scene->GetGlobalSettings ().GetSystemUnit () ;
	if ( sceneSystemUnit != FbxSystemUnit::m ) {
		const FbxSystemUnit::ConversionOptions conversionOptions ={
			false, // mConvertRrsNodes
			true, // mConvertAllLimits
			true, // mConvertClusters
			false, // mConvertLightIntensity
			true, // mConvertPhotometricLProperties
			false  // mConvertCameraClipPlanes
		} ;
		FbxSystemUnit::m.ConvertScene (_scene, conversionOptions) ;
	}
	//if ( sceneSystemUnit.GetScaleFactor () != 1.0 )
	//	FbxSystemUnit::m.ConvertScene (_scene) ;
	
	return (true) ;
}

bool gltfPackage::WriteScene (const utility::string_t &outdir) {
	auto pMgr =fbxSdkMgr::Instance ()->fbxMgr () ;
	int iFormat =pMgr->GetIOPluginRegistry ()->FindWriterIDByExtension ("gltf") ;
	if ( iFormat == -1 )
		return (false) ;
	FbxAutoDestroyPtr<FbxExporter> pExporter (FbxExporter::Create (pMgr, "")) ;
	utility::string_t newFn =outdir + utility::conversions::to_string_t (_scene->GetName ()) + U(".gltf") ;
	bool bRet =pExporter->Initialize (utility::conversions::to_utf8string (newFn).c_str (), iFormat, pMgr->GetIOSettings ()) ;
	_ASSERTE( bRet ) ;
	if ( !bRet )
		return (false) ;
	bRet =pExporter->Export (_scene) ;
	return (bRet) ;
}
