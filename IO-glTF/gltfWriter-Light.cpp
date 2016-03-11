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
#include "gltfWriter.h"

namespace _IOglTF_NS_ {

void gltfWriter::lightAttenuation (FbxLight *pLight, web::json::value &lightDef) {
	float attenuation [3] = {
		1.f, // Constant
		0.f, // Linear
		0.f // Quadratic
	} ;
	switch ( pLight->DecayType.Get () ) {
		case FbxLight::EDecayType::eLinear:
			attenuation [0] =0.0f ;
			attenuation [1] =1.0f ;
			break ;
		case FbxLight::EDecayType::eCubic:
			attenuation [0] =0.0f ;
			attenuation [2] =1.0f ; // OpenGL doesn't support cubicU( so default to quadratic
			break ;
		case FbxLight::EDecayType::eQuadratic:
			attenuation [0] =0.0f ;
			attenuation [2] =1.0f ;
			break ;
		case FbxLight::EDecayType::eNone:
		default:
			attenuation [0] =1.0f ;
			break ;
	}
	if ( attenuation [0] != 1.0f )
		lightDef [U("constantAttenuation")] =web::json::value::number (attenuation [0]) ;
	if ( attenuation [1] != 0.0f )
		lightDef [U("linearAttenuation")] =web::json::value::number (attenuation [1]) ;
	if ( attenuation [2] != 0.0f )
		lightDef [U("quadraticAttenuation")] =web::json::value::number (attenuation [2]) ;
}

web::json::value gltfWriter::WriteLight (FbxNode *pNode) {
	web::json::value light =web::json::value::object () ;
	web::json::value lightDef =web::json::value::object () ;
	light [U("name")] =web::json::value::string (nodeId (pNode, true)) ;

	if ( isKnownId (pNode->GetNodeAttribute ()->GetUniqueID ()) ) {
		// The light was already exported, create only the transform node
		web::json::value node =WriteNode (pNode) ;
		web::json::value ret =web::json::value::object ({ { U("nodes"), node } }) ;
		return (ret) ;
	}

	FbxLight *pLight =pNode->GetLight () ; //FbxCast<FbxLight>(pNode->GetNodeAttribute ()) ;
	static const FbxDouble3 defaultLightColor (1., 1., 1.) ;
	auto color =pLight->Color ;
	if ( _writeDefaults || color.Get () != defaultLightColor )
		lightDef [U("color")] =web::json::value::array ({{ 
			static_cast<float>(color.Get () [0]),
			static_cast<float>(color.Get () [1]),
			static_cast<float>(color.Get () [2])
		}}) ;
	switch ( pLight->LightType.Get () ) {
		case FbxLight::EType::ePoint:
			light [U("type")] =web::json::value::string (U("point")) ;
			lightAttenuation (pLight, lightDef) ;
			light [U("point")] =lightDef ;
			break ;
		case FbxLight::EType::eSpot:
			light [U("type")] =web::json::value::string (U("spot")) ;
			lightAttenuation (pLight, lightDef) ;
			if ( pLight->OuterAngle.Get () != 180.0 ) // default is PI
				lightDef [U("fallOffAngle")] =DEG2RAD (pLight->OuterAngle) ;
			if ( _writeDefaults )
				lightDef [U("fallOffExponent")] =web::json::value::number ((double)0.) ;
			light [U("spot")] =lightDef ;
			break ;
		case FbxLight::EType::eDirectional:
			light [U("type")] =web::json::value::string (U("directional")) ;
			light [U("directional")] =lightDef ;
			break ;
		case FbxLight::EType::eArea:
		case FbxLight::EType::eVolume:
		default: // ambient
			_ASSERTE (false) ;
			return (web::json::value::object ()) ;
			break ;
	}

	web::json::value lib =web::json::value::object ({ { nodeId (pNode, true, true), light } }) ;
	web::json::value node =WriteNode (pNode) ;

	return (web::json::value::object ({ { U("lights"), lib }, { U("nodes"), node } })) ;
}

web::json::value gltfWriter::WriteAmbientLight (FbxScene &pScene) {
	web::json::value light =web::json::value::object () ;
	web::json::value lightDef =web::json::value::object () ;
	static const FbxDouble3 defaultLightColor (1., 1., 1.) ;
	FbxColor color (pScene.GetGlobalSettings ().GetAmbientColor ()) ;
	if ( !color.mRed && !color.mGreen && !color.mBlue )
		return (web::json::value::object ()) ;
	if ( !_writeDefaults && color == defaultLightColor )
		return (web::json::value::object ()) ;
	lightDef [U("color")] =web::json::value::array () ;
	lightDef [U("color")] [0] =static_cast<float> (color.mRed) ;
	lightDef [U("color")] [1] =static_cast<float> (color.mGreen) ;
	lightDef [U("color")] [2] =static_cast<float> (color.mBlue) ;
	light [U("type")] =web::json::value::string (U("ambient")) ;
	light [U("ambient")] =lightDef ;

	utility::string_t buffer =utility::conversions::to_string_t ((int)0) ;
	utility::string_t uid (U("defaultambient")) ;
	uid +=U("_") + buffer ;
	return (web::json::value::object ({ { uid, light } })) ;
	//return (web::json::value::object ({ { nodeId (U("defaultambient"), 0x00), light } })) ;
}

}