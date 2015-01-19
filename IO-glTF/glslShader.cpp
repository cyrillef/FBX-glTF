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
#include "glslShader.h"

namespace _IOglTF_NS_ {

glslShader::glslShader (const utility::char_t *glslVersion /*=nullptr*/) {
	_declarations =U("") ;
	if ( glslVersion )
		_declarations +=U("#version ") + utility::string_t (glslVersion) + U("\n") ;
	_declarations +=U("precision highp float ;\n") ;
	_body =U("void main (void) {\n") ;
}

glslShader::~glslShader () {
}

void glslShader::setName (utility::string_t name) {
	_name =name ;
}

utility::string_t glslShader::getName () {
	return (_name) ;
}

void glslShader::_addDeclaration (utility::string_t qualifier, utility::string_t symbol, unsigned int type, size_t count, bool forcesAsAnArray /*=false*/) {
	if ( this->hasSymbol (symbol) == false ) {
		utility::string_t declaration = qualifier + U(" ") ;
		declaration +=IOglTF::glslType (type) ;
		declaration +=U(" ") + symbol ;
		if ( (count > 1) || forcesAsAnArray )
			declaration +=U("[") + utility::conversions::to_string_t ((int)count) + U("]") ;
		declaration +=U(";\n") ;
		_declarations += declaration ;

		_allSymbols [symbol] =symbol ;
	}
}

void glslShader::addAttribute (utility::string_t symbol, unsigned int type) {
	_addDeclaration (U("attribute"), symbol, type, true) ;
}

void glslShader::addUniform (utility::string_t symbol, unsigned int type, size_t count, bool forcesAsAnArray /*=false*/) {
	_addDeclaration (U("uniform"), symbol, type, count, forcesAsAnArray) ;
}

void glslShader::addVarying (utility::string_t symbol, unsigned int type) {
	_addDeclaration (U("varying"), symbol, type, true) ;
}

void glslShader::appendCode (const char *format, ...) {
	char buffer [1000] ;
	va_list args ;
	va_start (args, format) ;
	vsprintf_s (buffer, format, args) ;
	_body +=utility::conversions::to_string_t (buffer) ;
	va_end (args) ;
}

utility::string_t glslShader::source () {
	return (_declarations + _body) ;
}

bool glslShader::hasSymbol (const utility::string_t &symbol) {
	return (_allSymbols.count (symbol) > 0) ;
}

}
