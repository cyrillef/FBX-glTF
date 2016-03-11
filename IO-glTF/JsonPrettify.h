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

class JsonPrettify {
	static web::json::value sNull ;
	web::json::value &_json ;
	int _level ;

protected:
	JsonPrettify () : _json (JsonPrettify::sNull) {}

	void serialize (web::json::value &val, utility::ostream_t &stream) ;
	void indent (utility::ostream_t &stream) ;
	void format_string (const utility::string_t &st, utility::ostream_t &stream) ;
	void format_boolean (const bool val, utility::ostream_t &stream) ;
	void format_integer (const int val, utility::ostream_t &stream) ;
	void format_double (const double val, utility::ostream_t &stream) ;
	void format_null (utility::ostream_t &stream) ;
	void formatValue (web::json::value &val, utility::ostream_t &stream) ;
	void formatPair (utility::string_t &key, web::json::value &val, utility::ostream_t &stream) ;
	void formatArray (web::json::array &arr, utility::ostream_t &stream) ;
	void formatObject (web::json::object &obj, utility::ostream_t &stream) ;

public:
	JsonPrettify (web::json::value &json) : _json (json), _level (0) {} 

	void serialize (utility::ostream_t &stream) ;

} ;

}
