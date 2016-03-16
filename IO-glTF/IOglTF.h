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
	static const utility::char_t *szSAMPLER_2D ;
	// bufferView::Target (https://github.com/KhronosGroup/glTF/blob/master/specification/bufferView.schema.json)
	static const unsigned int ARRAY_BUFFER =34962 ;
	static const unsigned int ELEMENT_ARRAY_BUFFER =34963 ;
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
	virtual bool SpecificInitialize () ;
	virtual bool SpecificTerminate () { return (true) ; }

public:
	template<class T>
	static unsigned int accessorComponentType () ;
	template<class T>
	static const utility::char_t *accessorType (int size, int dim =1) ;
	static unsigned int techniqueParameters (const utility::char_t *szType, int compType =FLOAT) ;
	static const utility::char_t *glslAccessorType (unsigned int glType) ;
	static const utility::string_t glslShaderType (unsigned int glType) ;
	static const utility::char_t *mimeType (const utility::char_t *szFilename) ;
	// Online uri generator: http://bran.name/dump/data-uri-generator/
	static const utility::string_t dataURI (const utility::string_t fileName) ;
	static const utility::string_t dataURI (memoryStream<uint8_t> &stream) ;

} ;

template<class T>
/*static*/ unsigned int IOglTF::accessorComponentType () {
	static const size_t iChar =typeid (char).hash_code () ;
	static const size_t iInt8_t =typeid (int8_t).hash_code () ;
	static const size_t iUnsignedChar =typeid (unsigned char).hash_code () ;
	static const size_t iUInt8_t =typeid (uint8_t).hash_code () ;
	static const size_t iShort =typeid (short).hash_code () ;
	static const size_t iUnsignedShort =typeid (unsigned short).hash_code () ;
	static const size_t iInt =typeid (int).hash_code () ;
	static const size_t iUnsignedInt =typeid (unsigned int).hash_code () ;
	static const size_t iFloat =typeid (float).hash_code () ;
	static const size_t iBool =typeid (bool).hash_code () ;
	
	size_t tp =typeid (T).hash_code () ;
	if ( tp == iChar || tp == iInt8_t ) return (BYTE) ;
	if ( tp == iUnsignedChar || tp == iUInt8_t ) return (UNSIGNED_BYTE) ;
	if ( tp == iShort ) return (SHORT) ;
	if ( tp == iUnsignedShort ) return (UNSIGNED_SHORT) ;
	if ( tp == iInt ) return (INT) ;
	if ( tp == iUnsignedInt ) return (UNSIGNED_INT) ;
	if ( tp == iFloat ) return (FLOAT) ;
	if ( tp == iBool ) return (BOOL) ;
		
	ucout << U("IOglTF::accessorComponentType() / hash code: ") << std::hex << typeid (T).hash_code () << std::endl ;
	ucout << U("char: ") << std::hex << typeid (char).hash_code () << std::endl ;
	ucout << U("int8_t: ") << std::hex << typeid (int8_t).hash_code () << std::endl ;
	ucout << U("unsigned char: ") << std::hex << typeid (unsigned char).hash_code () << std::endl ;
	ucout << U("uint8_t: ") << std::hex << typeid (uint8_t).hash_code () << std::endl ;
	ucout << U("short: ") << std::hex << typeid (short).hash_code () << std::endl ;
	ucout << U("unsigned short: ") << std::hex << typeid (unsigned short).hash_code () << std::endl ;
	ucout << U("int: ") << std::hex << typeid (int).hash_code () << std::endl ;
	ucout << U("unsigned int: ") << std::hex << typeid (unsigned int).hash_code () << std::endl ;
	ucout << U("float: ") << std::hex << typeid (float).hash_code () << std::endl ;
	ucout << U("bool: ") << std::hex << typeid (bool).hash_code () << std::endl ;
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
