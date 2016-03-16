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

#define GLTF_EXTENTIONS						"GLTFExtentions"
#define IOSN_FBX_GLTF_EXTENTIONS			IOSN_EXPORT "|" GLTF_EXTENTIONS
#define GLTF_ANGLEINDEGREEE					"angleInDegree"
#define IOSN_FBX_GLTF_ANGLEINDEGREE			IOSN_FBX_GLTF_EXTENTIONS "|" GLTF_ANGLEINDEGREEE
#define GLTF_INVERTTRANSPARENCY				"invertTransparency"
#define IOSN_FBX_GLTF_INVERTTRANSPARENCY	IOSN_FBX_GLTF_EXTENTIONS "|" GLTF_INVERTTRANSPARENCY
#define GLTF_DEFAULTLIGHTING				"defaultLighting"
#define IOSN_FBX_GLTF_DEFAULTLIGHTING		IOSN_FBX_GLTF_EXTENTIONS "|" GLTF_DEFAULTLIGHTING
#define GLTF_COPYMEDIA						"copyMedia"
#define IOSN_FBX_GLTF_COPYMEDIA				IOSN_FBX_GLTF_EXTENTIONS "|" GLTF_COPYMEDIA 
#define IOSN_FBX_GLTF_EMBEDMEDIA			EXP_FBX_EMBEDDED
