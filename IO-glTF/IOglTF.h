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

void *_gltfFormatInfo (FbxWriter::EInfoRequest pRequest, int pId) ;

class IOglTF : public FbxPlugin {
	FBXSDK_PLUGIN_DECLARE(IOglTF) ;

public:
	static const char *PLUGIN_NAME ;
	static const char *PLUGIN_VERSION ;

	// accessor::componentType (https://github.com/KhronosGroup/glTF/blob/master/specification/accessor.schema.json)
	// technique::parameters (https://github.com/KhronosGroup/glTF/blob/master/specification/techniqueParameters.schema.json)
	// texture::type (https://github.com/KhronosGroup/glTF/blob/master/specification/texture.schema.json)
	static const unsigned int BYTE =5120 ;
	static const unsigned int UNSIGNED_BYTE =5121 ;
	static const unsigned int SHORT =5122 ;
	static const unsigned int UNSIGNED_SHORT =5123 ;
	static const unsigned int INT =5124 ;
	static const unsigned int UNSIGNED_INT =5125 ;
	static const unsigned int FLOAT =5126 ;
	static const unsigned int UNSIGNED_SHORT_5_6_5 =33635 ;
	static const unsigned int UNSIGNED_SHORT_4_4_4_4 =32819 ;
	static const unsigned int UNSIGNED_SHORT_5_5_5_1 =32820 ;
	static const unsigned int FLOAT_VEC2 =35664 ;
	static const unsigned int FLOAT_VEC3 =35665 ;
	static const unsigned int FLOAT_VEC4 =35666 ;
	static const unsigned int INT_VEC2 =35667 ;
	static const unsigned int INT_VEC3 =35668 ;
	static const unsigned int INT_VEC4 =35669 ;
	static const unsigned int BOOL =35670 ;
	static const unsigned int BOOL_VEC2 =35671 ;
	static const unsigned int BOOL_VEC3 =35672 ;
	static const unsigned int BOOL_VEC4 =35673 ;
	static const unsigned int FLOAT_MAT2 =35674 ;
	static const unsigned int FLOAT_MAT3 =35675 ;
	static const unsigned int FLOAT_MAT4 =35676 ;
	static const unsigned int SAMPLER_2D =35678 ;
	static const unsigned int SAMPLER_CUBE =35680 ;
	static const unsigned int FRAGMENT_SHADER =35632 ;
	static const unsigned int VERTEX_SHADER =35633 ;
	// accessor::type (https://github.com/KhronosGroup/glTF/blob/master/specification/accessor.schema.json)
	static const utility::char_t *szSCALAR ;
	static const utility::char_t *szFLOAT ;
	static const utility::char_t *szVEC2 ;
	static const utility::char_t *szVEC3 ;
	static const utility::char_t *szVEC4 ;
	static const utility::char_t *szINT ;
	static const utility::char_t *szIVEC2 ;
	static const utility::char_t *szIVEC3 ;
	static const utility::char_t *szIVEC4 ;
	static const utility::char_t *szBOOL ;
	static const utility::char_t *szBVEC2 ;
	static const utility::char_t *szBVEC3 ;
	static const utility::char_t *szBVEC4 ;
	static const utility::char_t *szMAT2 ;
	static const utility::char_t *szMAT3 ;
	static const utility::char_t *szMAT4 ;
	// bufferView::Target (https://github.com/KhronosGroup/glTF/blob/master/specification/bufferView.schema.json)
	static const int ARRAY_BUFFER =34962 ;
	static const int ELEMENT_ARRAY_BUFFER =34963 ;
	// mesh::primitive (https://github.com/KhronosGroup/glTF/blob/master/specification/meshPrimitive.schema.json)
	static const unsigned int POINTS =0 ;
	static const unsigned int LINES =1 ;
	static const unsigned int LINE_LOOP =2 ;
	static const unsigned int LINE_STRIP =3 ;
	static const unsigned int TRIANGLES =4 ;
	static const unsigned int TRIANGLE_STRIP =5 ;
	static const unsigned int TRIANGLE_FAN =6 ;
	// pass::states::enable (https://github.com/KhronosGroup/glTF/blob/master/specification/techniquePassStates.schema.json)
	static const unsigned int CULL_FACE =2884 ;
	static const unsigned int DEPTH_TEST =2929 ;
	static const unsigned int BLEND =3042 ;
	static const unsigned int POLYGON_OFFSET_FILL =32823 ;
	static const unsigned int SAMPLE_ALPHA_TO_COVERAGE =32926 ;
	static const unsigned int SCISSOR_TEST =3089 ;
	// texture::target (https://github.com/KhronosGroup/glTF/blob/master/specification/texture.schema.json)
	static const unsigned int TEXTURE_2D =3553 ;
	// texture::format & internalFormat
	static const unsigned int ALPHA =6406 ;
	static const unsigned int RGB =6407 ; 
	static const unsigned int RGBA =6408 ; 
	static const unsigned int LUMINANCE =6409 ; 
	static const unsigned int LUMINANCE_ALPHA =6410 ;
	// sampler::magFilter & minFilter (https://github.com/KhronosGroup/glTF/blob/master/specification/sampler.schema.json)
	static const unsigned int NEAREST =9728 ; 
	static const unsigned int LINEAR =9729 ;
	static const unsigned int NEAREST_MIPMAP_NEAREST =9984 ;
	static const unsigned int LINEAR_MIPMAP_NEAREST =9985 ; 
	static const unsigned int NEAREST_MIPMAP_LINEAR =9986 ; 
	static const unsigned int LINEAR_MIPMAP_LINEAR =9987 ;
	// sampler::wrapS & wrapT
	static const unsigned int CLAMP_TO_EDGE =33071 ;
	static const unsigned int MIRRORED_REPEAT =33648 ;
	static const unsigned int REPEAT =10497 ;

	//
	static const web::json::value Identity2 ;
	static const web::json::value Identity3 ;
	static const web::json::value Identity4 ;

protected:
	explicit IOglTF (const FbxPluginDef &pDefinition, FbxModule pLibHandle) : FbxPlugin (pDefinition, pLibHandle) {
	}

	// Implement Fbxmodules::FbxPlugin
	virtual bool SpecificInitialize () {
		int registeredCount =0 ;
		int gltfReaderId =0, gltfWriterId =0 ;
		GetData ().mSDKManager->GetIOPluginRegistry ()->RegisterWriter (gltfWriter::Create_gltfWriter, gltfWriter::gltfFormatInfo, gltfWriterId, registeredCount, gltfWriter::FillIOSettings) ;
		GetData ().mSDKManager->GetIOPluginRegistry ()->RegisterReader (gltfReader::Create_gltfReader, gltfReader::gltfFormatInfo, gltfReaderId, registeredCount, gltfReader::FillIOSettings) ;
		return (true) ;
	}

	virtual bool SpecificTerminate () {
		return true;
	}

public:
	template<class T>
	static unsigned int accessorComponentType () ;
	template<class T>
	static const utility::char_t *accessorType (int size, int dim =1) ;
	static unsigned int techniqueParameters (const utility::char_t *szType, int compType =FLOAT) ;
	static const utility::char_t *glslType (unsigned int glType) ;
	static const utility::char_t *mimeType (const utility::char_t *szFilename) ;
	// Online uri generator: http://bran.name/dump/data-uri-generator/
	static const utility::string_t dataURI (const utility::string_t fileName) ;
	static const utility::string_t dataURI (memoryStream<uint8_t> &stream) ;

} ;

template<class T>
/*static*/ unsigned int IOglTF::accessorComponentType () {
	//const type_info
	//ucout << utility::conversions::to_string_t (typeid (T).name()) ;
	switch ( typeid (T).hash_code () ) {
		case 0xf2a39391f9f8ad2c /*typeid (char).hash_code ()*/:
		case 0xdbf2be74fb6e4f03 /*typeid (int8_t).hash_code ()*/:
			return (BYTE) ;
		case 0xf9b6d9fdbc918e1b /*typeid (unsigned char).hash_code ()*/:
		//case 0xf9b6d9fdbc918e1b /*typeid (uint8_t).hash_code ()*/:
			return (UNSIGNED_BYTE) ;
		case 0xf69155480110981d /*typeid (short).hash_code ()*/:
			return (SHORT) ;
		case 0x27a436e029489774 /*typeid (unsigned short).hash_code ()*/:
			return (UNSIGNED_SHORT) ;
		case 0x2b9fff19004b3727 /*typeid (int).hash_code ()*/:
			return (INT) ;
		case 0xbaaedcffb89ab934 /*typeid (unsigned int).hash_code ()*/:
			return (UNSIGNED_INT) ;
		case 0xa00a62a9e2b863cc /*typeid (float).hash_code ()*/:
			return (FLOAT) ;
		case 0xcd2fd49b0b9fc026 /*typeid (bool).hash_code ()*/:
			return (BOOL) ;
	}
	_ASSERTE( false ) ;
	return (0) ;
}

template<class T>
/*static*/ const utility::char_t *IOglTF::accessorType (int size, int dim) {
	if ( dim == 1 ) {
		int type =IOglTF::accessorComponentType<T> () ;
		switch ( size ) {
			case 1: return (szSCALAR) ;
			case 2: return (type == FLOAT ? szVEC2 : (type == INT ? szIVEC2 : szBVEC2)) ;
			case 3: return (type == FLOAT ? szVEC3 : (type == INT ? szIVEC3 : szBVEC3)) ;
			case 4: return (type == FLOAT ? szVEC4 : (type == INT ? szIVEC2 : szBVEC4)) ;
		}
	} else if ( dim == 2 ) {
		switch ( size ) {
			case 2: return (szMAT2) ;
			case 3: return (szMAT3) ;
			case 4: return (szMAT4) ;
		}
	}
	_ASSERTE( false ) ;
	return (U("")) ;
}

}
