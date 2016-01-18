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
#include "IOglTF.h"

// FBX Interface
extern "C" {
	// The DLL is owner of the plug-in
	static _IOglTF_NS_::IOglTF *pPlugin =nullptr ;

	// This function will be called when an application will request the plug-in
#if defined(_WIN64) || defined (_WIN32)
	__declspec(dllexport) void FBXPluginRegistration (FbxPluginContainer &pContainer, FbxModule pLibHandle) {
#else
	void FBXPluginRegistration (FbxPluginContainer &pContainer, FbxModule pLibHandle) {
#endif
		if ( pPlugin == nullptr ) {
			// Create the plug-in definition which contains the information about the plug-in
			FbxPluginDef pluginDef ;
			pluginDef.mName =FbxString (_IOglTF_NS_::IOglTF::PLUGIN_NAME) ;
			pluginDef.mVersion =FbxString (_IOglTF_NS_::IOglTF::PLUGIN_VERSION) ;

			// Create an instance of the plug-in.  The DLL has the ownership of the plug-in
			pPlugin =_IOglTF_NS_::IOglTF::Create (pluginDef, pLibHandle) ;

			// Register the plug-in
			pContainer.Register (*pPlugin) ;
		}
	}
}

namespace _IOglTF_NS_ {

FBXSDK_PLUGIN_IMPLEMENT(IOglTF) ;

bool IOglTF::SpecificInitialize () {
	int registeredCount =0 ;
	int gltfReaderId =0, gltfWriterId =0 ;
	GetData ().mSDKManager->GetIOPluginRegistry ()->RegisterWriter (gltfWriter::Create_gltfWriter, gltfWriter::gltfFormatInfo, gltfWriterId, registeredCount, gltfWriter::FillIOSettings) ;
	GetData ().mSDKManager->GetIOPluginRegistry ()->RegisterReader (gltfReader::Create_gltfReader, gltfReader::gltfFormatInfo, gltfReaderId, registeredCount, gltfReader::FillIOSettings) ;
	return (true) ;
}

//-----------------------------------------------------------------------------

/*static*/ const char *IOglTF::PLUGIN_NAME ="IO-glTF" ;
/*static*/ const char *IOglTF::PLUGIN_VERSION ="1.0.0" ;

#ifdef __APPLE__
/*static*/ const unsigned int IOglTF::BYTE ;
/*static*/ const unsigned int IOglTF::UNSIGNED_BYTE ;
/*static*/ const unsigned int IOglTF::SHORT ;
/*static*/ const unsigned int IOglTF::UNSIGNED_SHORT ;
/*static*/ const unsigned int IOglTF::INT ;
/*static*/ const unsigned int IOglTF::UNSIGNED_INT ;
/*static*/ const unsigned int IOglTF::FLOAT ;
/*static*/ const unsigned int IOglTF::UNSIGNED_SHORT_5_6_5 ;
/*static*/ const unsigned int IOglTF::UNSIGNED_SHORT_4_4_4_4 ;
/*static*/ const unsigned int IOglTF::UNSIGNED_SHORT_5_5_5_1 ;
/*static*/ const unsigned int IOglTF::FLOAT_VEC2 ;
/*static*/ const unsigned int IOglTF::FLOAT_VEC3 ;
/*static*/ const unsigned int IOglTF::FLOAT_VEC4 ;
/*static*/ const unsigned int IOglTF::INT_VEC2 ;
/*static*/ const unsigned int IOglTF::INT_VEC3 ;
/*static*/ const unsigned int IOglTF::INT_VEC4 ;
/*static*/ const unsigned int IOglTF::BOOL ;
/*static*/ const unsigned int IOglTF::BOOL_VEC2 ;
/*static*/ const unsigned int IOglTF::BOOL_VEC3 ;
/*static*/ const unsigned int IOglTF::BOOL_VEC4 ;
/*static*/ const unsigned int IOglTF::FLOAT_MAT2 ;
/*static*/ const unsigned int IOglTF::FLOAT_MAT3 ;
/*static*/ const unsigned int IOglTF::FLOAT_MAT4 ;
/*static*/ const unsigned int IOglTF::SAMPLER_2D ;
/*static*/ const unsigned int IOglTF::SAMPLER_CUBE ;
/*static*/ const unsigned int IOglTF::FRAGMENT_SHADER ;
/*static*/ const unsigned int IOglTF::VERTEX_SHADER ;
#endif
/*static*/ const utility::char_t *IOglTF::szSCALAR =U("SCALAR") ;
/*static*/ const utility::char_t *IOglTF::szFLOAT =U("FLOAT") ;
/*static*/ const utility::char_t *IOglTF::szVEC2 =U("VEC2") ;
/*static*/ const utility::char_t *IOglTF::szVEC3 =U("VEC3") ;
/*static*/ const utility::char_t *IOglTF::szVEC4 =U ("VEC4") ;
/*static*/ const utility::char_t *IOglTF::szINT =U("INT") ; ;
/*static*/ const utility::char_t *IOglTF::szIVEC2 =U("IVEC2") ; ;
/*static*/ const utility::char_t *IOglTF::szIVEC3 =U("IVEC3") ; ;
/*static*/ const utility::char_t *IOglTF::szIVEC4 =U("IVEC4") ; ;
/*static*/ const utility::char_t *IOglTF::szBOOL =U("BOOL") ; ;
/*static*/ const utility::char_t *IOglTF::szBVEC2 =U("BVEC2") ; ;
/*static*/ const utility::char_t *IOglTF::szBVEC3 =U("BVEC3") ; ;
/*static*/ const utility::char_t *IOglTF::szBVEC4 =U("BVEC4") ; ;
/*static*/ const utility::char_t *IOglTF::szMAT2 =U("MAT2") ;
/*static*/ const utility::char_t *IOglTF::szMAT3 =U("MAT3") ;
/*static*/ const utility::char_t *IOglTF::szMAT4 =U("MAT4") ;
#ifdef __APPLE__
/*static*/ const unsigned int IOglTF::ARRAY_BUFFER ;
/*static*/ const unsigned int IOglTF::ELEMENT_ARRAY_BUFFER ;
/*static*/ const unsigned int IOglTF::POINTS ;
/*static*/ const unsigned int IOglTF::LINES ;
/*static*/ const unsigned int IOglTF::LINE_LOOP ;
/*static*/ const unsigned int IOglTF::LINE_STRIP ;
/*static*/ const unsigned int IOglTF::TRIANGLES ;
/*static*/ const unsigned int IOglTF::TRIANGLE_STRIP ;
/*static*/ const unsigned int IOglTF::TRIANGLE_FAN ;
/*static*/ const unsigned int IOglTF::CULL_FACE ;
/*static*/ const unsigned int IOglTF::DEPTH_TEST ;
/*static*/ const unsigned int IOglTF::BLEND ;
/*static*/ const unsigned int IOglTF::POLYGON_OFFSET_FILL ;
/*static*/ const unsigned int IOglTF::SAMPLE_ALPHA_TO_COVERAGE ;
/*static*/ const unsigned int IOglTF::SCISSOR_TEST ;
/*static*/ const unsigned int IOglTF::TEXTURE_2D ;
/*static*/ const unsigned int IOglTF::ALPHA ;
/*static*/ const unsigned int IOglTF::RGB ;
/*static*/ const unsigned int IOglTF::RGBA ;
/*static*/ const unsigned int IOglTF::LUMINANCE ;
/*static*/ const unsigned int IOglTF::LUMINANCE_ALPHA ;
/*static*/ const unsigned int IOglTF::NEAREST ;
/*static*/ const unsigned int IOglTF::LINEAR ;
/*static*/ const unsigned int IOglTF::NEAREST_MIPMAP_NEAREST ;
/*static*/ const unsigned int IOglTF::LINEAR_MIPMAP_NEAREST ;
/*static*/ const unsigned int IOglTF::NEAREST_MIPMAP_LINEAR ;
/*static*/ const unsigned int IOglTF::LINEAR_MIPMAP_LINEAR ;
/*static*/ const unsigned int IOglTF::CLAMP_TO_EDGE ;
/*static*/ const unsigned int IOglTF::MIRRORED_REPEAT ;
/*static*/ const unsigned int IOglTF::REPEAT ;
#endif
/*static*/ const web::json::value IOglTF::Identity2 =web::json::value::array ({ { 1., 0., 0., 1. } }) ;
/*static*/ const web::json::value IOglTF::Identity3 =web::json::value::array ({ { 1., 0., 0., 0., 1., 0., 0., 0., 1. } }) ;
/*static*/ const web::json::value IOglTF::Identity4 =web::json::value::array ({{ 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1. }}) ;

// Get extension, description or version info about gltfWriter
void *_gltfFormatInfo (FbxWriter::EInfoRequest pRequest, int pId) {
	static const char *sExt [] = { "gltf", 0 } ;
	static const char *sDesc [] = { "glTF for WebGL (*.gltf)", 0 } ;
	static const char *sVersion [] = { "0.8", 0 } ;
	static const char *sInfoCompatible [] = { "-", 0 } ;
	static const char *sInfoUILabel [] = { "-", 0 } ;

	switch ( pRequest ) {
		case FbxWriter::eInfoExtension:
			return (sExt) ;
		case FbxWriter::eInfoDescriptions:
			return (sDesc) ;
		case FbxWriter::eInfoVersions:
			return (sVersion) ;
		case FbxWriter::eInfoCompatibleDesc:
			return (sInfoCompatible) ;
		case FbxWriter::eInfoUILabel:
			return (sInfoUILabel) ;
		default:
			return (0) ;
	}
}

//-----------------------------------------------------------------------------
/*static*/ unsigned int IOglTF::techniqueParameters (const utility::char_t *szType, int compType /*=FLOAT*/) {
	utility::string_t st (szType) ;
	if ( st == szSCALAR )
		return (compType) ;
	if ( st == szVEC2 )
		return (compType == FLOAT ? FLOAT_VEC2 : (compType == INT ? INT_VEC2 : BOOL_VEC2)) ;
	if ( st == szVEC3 )
		return (compType == FLOAT ? FLOAT_VEC3 : (compType == INT ? INT_VEC3 : BOOL_VEC3)) ;
	if ( st == szVEC4 )
		return (compType == FLOAT ? FLOAT_VEC4 : (compType == INT ? INT_VEC4 : BOOL_VEC4)) ;
	if ( st == szMAT2 )
		return (FLOAT_MAT2) ;
	if ( st == szMAT3 )
		return (FLOAT_MAT3) ;
	if ( st == szMAT4 )
		return (FLOAT_MAT4) ;
	return (0) ;
}

/*static*/ const utility::char_t *IOglTF::glslType (unsigned int glType) {
	switch ( glType ) {
		case FLOAT:
		case INT:
		case BOOL:
		case UNSIGNED_SHORT:
		case UNSIGNED_INT:
			return (szSCALAR) ;
		case FLOAT_VEC2:
		case INT_VEC2:
		case BOOL_VEC2:
			return (szVEC2) ;
		case FLOAT_VEC3:
		case INT_VEC3:
		case BOOL_VEC3:
			return (szVEC3) ;
		case FLOAT_VEC4:
		case INT_VEC4:
		case BOOL_VEC4:
			return (szVEC4) ;
		case FLOAT_MAT2:
			return (szMAT2) ;
		case FLOAT_MAT3:
			return (szMAT3) ;
		case FLOAT_MAT4:
			return (szMAT4) ;
	}
	return (U("")) ;
}

/*static*/ const utility::char_t *IOglTF::mimeType (const utility::char_t *szFilename) {
	utility::string_t contentType =U("application/octet-stream") ;
	FbxString ext =FbxPathUtils::GetExtensionName (utility::conversions::to_utf8string (szFilename).c_str ()) ;
	ext =ext.Lower () ;
	if ( ext == "png" )
		return (U("image/png")) ;
	if ( ext == "gif" )
		return (U("image/gif")) ;
	if ( ext == "jpg" || ext == "jpeg" )
		return (U("image/jpeg")) ;
	return (U("application/octet-stream")) ;
}

/*static*/ const utility::string_t IOglTF::dataURI (const utility::string_t fileName) {
	// data:[<mime type>][;charset=<charset>][;base64],<encoded data>
	utility::string_t st (U ("data:")) ;
	Concurrency::streams::file_stream<uint8_t>::open_istream (fileName)
		.then ([&] (pplx::task<Concurrency::streams::basic_istream<uint8_t> > previousTask) {
		try {
			auto fileStream =previousTask.get () ;
			fileStream.seek (0, std::ios::end) ;
			auto length =static_cast<size_t> (fileStream.tell ()) ;
			fileStream.seek (0, std::ios_base::beg) ;
			concurrency::streams::streambuf<uint8_t> buff (fileStream.streambuf ()) ;

			std::vector<uint8_t> v8 ;
			v8.resize (length) ;
			buff.seekpos (0, std::ios_base::binary) ;
			buff.getn (&v8.front (), length) ;

			st +=IOglTF::mimeType (fileName.c_str ()) ;
			//st +="[;charset=<charset>]" ;
			st +=U (";base64") ;
			st +=U (",") + utility::conversions::to_base64 (v8) ;
		} catch ( ... ) {
			//ucout << U("Error: ") << e.what () << std::endl ;
		}
	}).wait () ;
	return (st) ;
}

/*static*/ const utility::string_t IOglTF::dataURI (memoryStream<uint8_t> &stream) {
	// data:[<mime type>][;charset=<charset>][;base64],<encoded data>
	utility::string_t st (U ("data:")) ;
	st +=IOglTF::mimeType (U("")) ;
	//st +="[;charset=<charset>]" ;
	st +=U (";base64") ;
	st +=U (",") + utility::conversions::to_base64 (stream.vec ()) ;
	return (st) ;
}

}
