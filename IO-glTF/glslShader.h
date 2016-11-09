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
#if !defined(_WIN32) || defined(_WIN64)
#include <wchar.h>
#endif

// http://stackoverflow.com/questions/13624124/online-webgl-glsl-shader-editor
// http://glslsandbox.com/

namespace _IOglTF_NS_ {

class glslShader {

protected:
	utility::string_t _name ;
	utility::string_t _declarations ;
	utility::string_t _body ;
	std::map<utility::string_t, utility::string_t> _allSymbols ;

public:
	glslShader (const utility::char_t *glslVersion =nullptr) ;
	virtual ~glslShader () ;

	utility::string_t &name () ;
protected:
	void _addDeclaration (utility::string_t qualifier, utility::string_t symbol, unsigned int type, size_t count, bool forcesAsAnArray =false) ;
	utility::string_t body () ;
public:
	void addAttribute (utility::string_t symbol, unsigned int type, size_t count =1, bool forcesAsAnArray =false) ;
	void addUniform (utility::string_t symbol, unsigned int type, size_t count =1, bool forcesAsAnArray =false) ;
	void addVarying (utility::string_t symbol, unsigned int type, size_t count =1, bool forcesAsAnArray =false) ;
	bool hasSymbol (const utility::string_t &symbol) ;

	void appendCode (const char *format, ...) ;
#if defined(_WIN32) || defined(_WIN64)
	void appendCode (const utility::char_t *format, ...) ;
#else
	void appendCode (const wchar_t *format, ...) ;
#endif

	//void appendCode (const utility::char_t *code) ;
	utility::string_t source () ;

	static bool isVertexShaderSemantic (const utility::char_t *semantic) ;
	static bool isFragmentShaderSemantic (const utility::char_t *semantic) ;

} ;

class glslTech {
	bool _bHasNormals ;
	bool _bHasJoint ;
	bool _bHasWeight ;
	bool _bHasSkin ;
	bool _bHasTexTangent ;
	bool _bHasTexBinormal ;
	bool _bModelContainsLights ;
	bool _bLightingIsEnabled ;
	bool _bHasAmbientLight ;
	bool _bHasSpecularLight ;
	bool _bHasNormalMap ;

protected:
	glslShader _vertexShader ;
	glslShader _fragmentShader ;

public:
	glslTech (web::json::value technique, web::json::value values, web::json::value gltf, const utility::char_t *glslVersion =nullptr) ;
	virtual ~glslTech () ;

	glslShader &vertexShader () { return (_vertexShader) ; }
	glslShader &fragmentShader () { return (_fragmentShader) ; }

protected:
	static utility::string_t format (const utility::char_t *format, ...) ;
	const utility::string_t needsVarying (const utility::char_t *semantic) ;
	static bool isVertexShaderSemantic (const utility::char_t *semantic) ;
	static bool isFragmentShaderSemantic (const utility::char_t *semantic) ;
	void prepareParameters (web::json::value technique) ;
	void hwSkinning () ;
	bool lighting1 (web::json::value technique, web::json::value gltf) ;
	void texcoords (web::json::value technique) ;
	web::json::value lightNodes (web::json::value gltf) ;
	void lighting2 (web::json::value technique, web::json::value gltf) ;
	void finalizingShaders (web::json::value technique, web::json::value gltf) ;

} ;

}
