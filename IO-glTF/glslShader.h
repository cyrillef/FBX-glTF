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

// http://stackoverflow.com/questions/13624124/online-webgl-glsl-shader-editor
// http://glslsandbox.com/

namespace _IOglTF_NS_ {

class glslShader {

private:
	utility::string_t _name ;
	utility::string_t _declarations ;
	utility::string_t _body ;
	std::map<utility::string_t, utility::string_t> _allSymbols ;

public:
	glslShader (const utility::char_t *glslVersion =nullptr) ;
	virtual ~glslShader () ;

	void setName (utility::string_t name) ;
	utility::string_t getName () ;
	void _addDeclaration (utility::string_t qualifier, utility::string_t symbol, unsigned int type, size_t count, bool forcesAsAnArray =false) ;
	void addAttribute (utility::string_t symbol, unsigned int type) ;
	void addUniform (utility::string_t symbol, unsigned int type, size_t count, bool forcesAsAnArray =false) ;
	void addVarying (utility::string_t symbol, unsigned int type) ;
	void appendCode (const char * format, ...) ;
	utility::string_t source () ;
	bool hasSymbol (const utility::string_t &symbol) ;

} ;

}
