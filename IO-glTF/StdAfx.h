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

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

#include <assert.h>
#include <tchar.h>
#define GLTF_UNREFERENCED_PARAMETER(x) (void)x
#define _ASSERTE(x) assert(x)

#ifdef GLTF_DEPRECATION_NO_WARNINGS
#define GLTF_DEPRECATED(x)
#else
#define GLTF_DEPRECATED(x) __attribute__((deprecated(x)))
#endif

#include <memory>
#include <map>
#include <limits>
#include <fstream>

// C++ REST SDK (codename "Casablanca")
// https://casablanca.codeplex.com/
#include <cpprest/filestream.h>
#include <cpprest/json.h>					// JSON library
#include <cpprest/uri.h>                    // URI library
typedef web::json::value JsonValue ;
typedef web::json::value::value_type JsonValueType ;

// C++ FBX SDK
// http://www.autodeks.com/developfbx
#define FBXSDK_SHARED
#include <fbxsdk.h>
#pragma comment (lib, "libfbxsdk.lib")
//#pragma comment (lib, "libfbxsdk-md.lib")
//#include "webgl-idl.h"
#include "glTF.h"

#include "ns_exports.h"
#include "string_t_utils.h"
#include "memoryStream.h"
#include "IOglTF.h"
#include "gltfReader.h"
#include "gltfWriter.h"

