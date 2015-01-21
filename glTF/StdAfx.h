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

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#include <memory>
#include <map>

// C++ REST SDK (codename "Casablanca")
// https://casablanca.codeplex.com/
#include <cpprest/filestream.h>
#include <cpprest/json.h>					// JSON library
typedef web::json::value JsonValue ;
typedef web::json::value::value_type JsonValueType ;

// C++ FBX SDK
// http://www.autodeks.com/developfbx
#define FBXSDK_SHARED
#include <fbxsdk.h>
#pragma comment (lib, "libfbxsdk.lib")
//#pragma comment (lib, "libfbxsdk-md.lib")

#include "gltf.h"
#include "gltfPackage.h"

#define STATIC_GETOPT
