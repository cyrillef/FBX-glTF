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

void gltfWriter::cameraFOV (FbxCamera *pCamera, web::json::value &cameraDef) {
	FbxCamera::EApertureMode apertureMode =pCamera->GetApertureMode () ;
	// Camera aperture modes. The aperture mode determines which values drive the camera aperture. 
	// If the aperture mode is eHORIZONTAL_AND_VERTICAL, then the FOVX and FOVY is used. 
	// If the aperture mode is eHORIZONTAL or eVERTICAL, then the FOV is used.
	// if the aperture mode is eFOCAL_LENGTH, then the focal length is used.
	double focalAngle =static_cast<double>(pCamera->FieldOfView.Get ()) ;
	switch ( pCamera->GetApertureMode () ) {
		case FbxCamera::eHorizAndVert: // Fit the resolution gate within the film gate
		{
			_ASSERTE( false ) ; // todo: gltf wants yfov
			double focalAngleX =static_cast<double>(pCamera->FieldOfViewX.Get ()) ;
			double focalAngleY =static_cast<double>(pCamera->FieldOfViewY.Get ()) ;
			focalAngle =focalAngleY ;
			break ;
		}
		case FbxCamera::eHorizontal: // Fit the resolution gate horizontally within the film gate
		{
			_ASSERTE( false ) ; // todo: gltf wants yfov
			double focalAngleX =focalAngle ;
			// aspect_ratio = xfov / yfov
			// horizontalFOV = 2*atan(aspectRatio * tan(verticalFOV/2)) 
			break ;
		}
		case FbxCamera::eVertical: // Fit the resolution gate vertically within the film gate
		{
			double focalAngleY =focalAngle ;
			break ;
		}
		case FbxCamera::eFocalLength: // Fit the resolution gate according to the focal length
		{
			_ASSERTE( false ) ; // todo: gltf wants yfov
			double focalLength =static_cast<double>(pCamera->FocalLength.Get ()) ;
			double computedFOV =pCamera->ComputeFieldOfView (focalLength) ;
			if ( focalAngle != computedFOV ) {
				pCamera->FieldOfView.Set (computedFOV) ;
				focalAngle =computedFOV ;
			}
			break ;
		}
		default:
			_ASSERTE( false ) ;
			break ;
	}
	//cameraDef [U("yfov")] =web::json::value::number (DEG2RAD (focalAngle)) ;
	// todo montage and three wants degrees, the spec says radian, with degrees for now
	cameraDef [U("yfov")] =web::json::value::number (focalAngle) ;
}

// https://github.com/KhronosGroup/glTF/blob/master/specification/camera.schema.json
web::json::value gltfWriter::WriteCamera (FbxNode *pNode) {
	web::json::value camera =web::json::value::object () ;
	web::json::value cameraDef =web::json::value::object () ;
	camera [U("name")] =web::json::value::string (nodeId (pNode)) ; // https://github.com/KhronosGroup/glTF/blob/master/specification/glTFChildOfRootProperty.schema.json
	FbxCamera *pCamera =pNode->GetCamera () ; //FbxCast<FbxCamera>(pNode->GetNodeAttribute ()) ;
	//FbxCamera::EAspectRatioMode aspectRatioMode =pCamera->GetAspectRatioMode () ;
	switch ( pCamera->ProjectionType.Get () ) {
		case FbxCamera::EProjectionType::ePerspective: // https://github.com/KhronosGroup/glTF/blob/master/specification/cameraPerspective.schema.json
			camera [U("type")] =web::json::value::string (U("perspective")) ;
			cameraDef [U("aspectRatio")] =web::json::value::number (pCamera->FilmAspectRatio) ; // (pCamera->AspectWidth / pCamera->AspectHeight) ;
			//cameraDef [U("yfov")] =web::json::value::number (DEG2RAD(pCamera->FieldOfView)) ;
			cameraFOV (pCamera, cameraDef) ;
			cameraDef [U("zfar")] =web::json::value::number (pCamera->FarPlane) ;
			cameraDef [U("znear")] =web::json::value::number (pCamera->NearPlane) ;
			camera [U("perspective")] =cameraDef ;
			break ;
		case FbxCamera::EProjectionType::eOrthogonal: // https://github.com/KhronosGroup/glTF/blob/master/specification/cameraOrthographic.schema.json
			camera [U("type")] =web::json::value::string (U("orthographic")) ;
			//cameraDef [U("xmag")] =web::json::value::number (pCamera->_2DMagnifierX) ;
			//cameraDef [U("ymag")] =web::json::value::number (pCamera->_2DMagnifierY) ;
			cameraDef [U("xmag")] =web::json::value::number (pCamera->OrthoZoom) ;
			cameraDef [U("ymag")] =web::json::value::number (pCamera->OrthoZoom) ; // FBX Collada reader set OrthoZoom using xmag and ymag each time they appear
			cameraDef [U("zfar")] =web::json::value::number (pCamera->FarPlane) ;
			cameraDef [U("znear")] =web::json::value::number (pCamera->NearPlane) ;
			camera [U("orthographic")] =cameraDef ;
			break ;
		default:
			_ASSERTE (false) ;
			break ;
	}
	web::json::value lib =web::json::value::object ({ { nodeId (pNode), camera } }) ;
	web::json::value node =WriteNode (pNode) ;

	return (web::json::value::object ({ { U("cameras"), lib }, { U("nodes"), node } })) ;
	//return (web::json::value::object ({{ nodeId (pNode), camera }})) ;
}

}
