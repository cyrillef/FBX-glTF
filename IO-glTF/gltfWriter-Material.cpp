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

// FBX does not support Blinn yet, it would become Phong by default.
utility::string_t gltfWriter::LighthingModel (FbxSurfaceMaterial *pMaterial) {
	if ( pMaterial->Is<FbxSurfacePhong> () ) {
		return (U("Phong")) ;
	} else if ( pMaterial->Is<FbxSurfaceLambert> () ) {
		return (U("Lambert")) ;
	} else { // use shading model
		FbxString szShadingModel =pMaterial->ShadingModel.Get () ;
		// shading models supported here are: "constant", "blinn"
		// note that "lambert" and "phong" should have been treated above
		// all others default to "phong"
		if ( szShadingModel == "constant" )
			return (U("Constant")) ;
		else if ( szShadingModel == "blinn" )
			return (U("Blinn")) ;
		else
			return (U("Phong")) ;
	}
}

web::json::value gltfWriter::WriteMaterial (FbxNode *pNode, FbxSurfaceMaterial *pMaterial) {
	utility::string_t materialName =utility::conversions::to_string_t (pMaterial->GetNameWithoutNameSpacePrefix ().Buffer ()) ; // Material do not support namespaces.

	// Look if this material is already in the materials library.
	//if ( _json [U("materials")].has_field (materialName) )
	if ( isNameRegistered (materialName) )
		return (web::json::value::string (materialName)) ;

	web::json::value material =web::json::value::object () ;
	material [U("name")] =web::json::value::string (materialName) ;

	web::json::value ret =web::json::value::null () ;
	
		// Use Cg shaders in WebGL?
		// Usually you don't want to.
		//
		// You could compile your shaders into GLSL with cgc (-profile glslv or - profile glslf), then load them 
		// anywhere you want.There is, however, slight difference between desktop and ES GLSL (WebGL likely using
		// ES specification), so it may require adding a little hints at the beginning of shader 
		// (like precision mediump float; , could easily be #ifdef'd/).
		//
		// Of course you can't use some Cg functionality in this case - if it is missing in GLSL, cgc can do nothing.
		// E.g. mapping uniforms to specified registers or settings varyings to specified interpolator.

		// If the material has a CGFX implementation, export the bind table
		const FbxImplementation *pImpl =GetImplementation (pMaterial, FBXSDK_IMPLEMENTATION_HLSL) ;
		if ( pImpl == nullptr )
			pImpl =GetImplementation (pMaterial, FBXSDK_IMPLEMENTATION_CGFX) ;
		if ( pImpl ) {
			const FbxBindingTable *pTable =pImpl->GetRootTable () ;
			size_t entryCount =pTable->GetEntryCount () ;
			for ( size_t entryIndex =0 ; entryIndex < entryCount ; entryIndex++ ) {
				const FbxBindingTableEntry &bindTableEntry =pTable->GetEntry (entryIndex) ;
				const char *pszDest =bindTableEntry.GetDestination () ;
				FbxProperty pSourceProperty =pMaterial->FindPropertyHierarchical (bindTableEntry.GetSource ()) ;
				_ASSERTE( pSourceProperty.IsValid () ) ;
				//TODO: - hlsl and CGFX support
			}
		}

		// IMPORTANT NOTE:
		// Always check for the most complex class before the less one. In this case, Phong inherit from Lambert,
		// so if we would be testing for lambert classid before phong, we would never enter the phong case.
		// eh, Blinn–Phong shading model - The Blinn–Phong reflection model (also called the modified Phong reflection model) 
		// is a modification to the Phong reflection model
		if ( pMaterial->Is<FbxSurfacePhong> () ) {
			ret =WritePhongMaterial (pNode, pMaterial) ;
		} else if ( pMaterial->Is<FbxSurfaceLambert> () ) {
			ret =WriteLambertMaterial (pNode, pMaterial) ;
		} else { // use shading model
			FbxString szShadingModel =pMaterial->ShadingModel.Get () ;
			// shading models supported here are: "constant", "blinn"
			// note that "lambert" and "phong" should have been treated above
			// all others default to "phong"
			if ( szShadingModel == "constant" ) {
				ret =WriteConstantShadingModelMaterial (pNode, pMaterial) ;
			} else if ( szShadingModel == "blinn" ) {
				ret =WriteBlinnShadingModelMaterial (pNode, pMaterial) ;
			} else {
				// If has a CGFX implementation, export constant in profile common;
				// And set path with NVidia extension
				FbxImplementation *pImpl =pMaterial->GetDefaultImplementation () ;
				if ( pImpl && pImpl->Language.Get () == FBXSDK_SHADING_LANGUAGE_CGFX )
					ret =WriteDefaultShadingModelWithCGFXMaterial (pNode, pMaterial) ;
				else
					ret =WriteDefaultShadingModelMaterial (pNode, pMaterial) ;
			}
		}
	

	web::json::value techniqueParameters =web::json::value::null () ;
	if ( !ret.is_null () ) {
		material [U("values")] =ret [U("values")] ;
		techniqueParameters =ret [U("techniqueParameters")] ;
	}

	utility::string_t techniqueName =createUniqueName (materialName + U("_technique"), 0) ; // Start with 0, but will increase based on own many are yet registered
	material [U("technique")] =web::json::value::string (techniqueName) ; // technique name already registered

	registerName (materialName) ; // Register material name to avoid duplicates
	web::json::value lib =web::json::value::object ({ { materialName, material } }) ;

	web::json::value technique =web::json::value::object ({{ U("parameters"), techniqueParameters }}) ;
	web::json::value techniques =web::json::value::object ({ { techniqueName, technique } }) ;

	web::json::value full =web::json::value::object ({ { U("materials"), lib }, { U("techniques"), techniques } }) ;
	if ( !ret.is_null () ) {
		for ( auto iter =ret.as_object ().begin () ; iter != ret.as_object ().end () ; iter++ ) {
			if ( iter->first != U("values") && iter->first != U("techniqueParameters") )
				full [iter->first] =iter->second ;
		}
	}
	return (full) ;
}

web::json::value gltfWriter::WriteDefaultMaterial (FbxNode *pNode) {
	utility::string_t materialName (U("defaultMaterial")) ;

	// Look if this material is already in the materials library.
	//if ( _json [U("materials")].has_field (materialName) )
	if ( isNameRegistered (materialName) )
		return (web::json::value::string (materialName)) ;

	web::json::value material =web::json::value::object () ;
	material [U("name")] =web::json::value::string (materialName) ;

	// Look if this material is already in the materials library.
	web::json::value ret =web::json::value::null () ;
	if ( !_json [U("materials")].has_field (materialName) ) {
		FbxNode::EShadingMode shadingMode =pNode->GetShadingMode () ;
		ret =WriteDefaultShadingModelMaterial (pNode) ;
	}

	web::json::value techniqueParameters =web::json::value::null () ;
	if ( !ret.is_null () ) {
		material [U("values")] =ret [U("values")] ;
		techniqueParameters =ret [U("techniqueParameters")] ;
	}

	utility::string_t techniqueName =createUniqueName (materialName + U("_technique"), 0) ; // Start with 0, but will increase based on own many are yet registered
	material [U("technique")] =web::json::value::string (techniqueName) ; // technique name already registered

	registerName (materialName) ; // Register material name to avoid duplicates
	web::json::value lib =web::json::value::object ({ { materialName, material } }) ;

	web::json::value technique =web::json::value::object ({{ U("parameters"), techniqueParameters }}) ;
	web::json::value techniques =web::json::value::object ({ { techniqueName, technique } }) ;

	web::json::value full =web::json::value::object ({ { U("materials"), lib }, { U("techniques"), techniques } }) ;
	if ( !ret.is_null () ) {
		for ( auto iter =ret.as_object ().begin () ; iter != ret.as_object ().end () ; iter++ ) {
			if ( iter->first != U("values") && iter->first != U("techniqueParameters") )
				full [iter->first] =iter->second ;
		}
	}
	return (full) ;
}

#define MultiplyDouble3By(a,b) a [0] *= b ; a [1] *= b ; a [2] *= b

// The transparency factor is usually in the range [0,1]. Maya does always export a transparency factor of 1.0, which implies full transparency.

web::json::value gltfWriter::WriteMaterialTransparencyParameter (
	const utility::char_t *pszName, 
	FbxPropertyT<FbxDouble> &property, FbxPropertyT<FbxDouble3> &propertyColor, FbxProperty &propertyOpaque,
	web::json::value &values, web::json::value &techniqueParameters
) {
	web::json::value ret =web::json::value::null () ;
	double value =1. ;
	if ( propertyOpaque.IsValid () ) {
		value =1.0 - propertyOpaque.Get<double> () ;
	} else {
		if ( !property.IsValid () )
			return (ret) ;
		value =property.Get () ;
		if ( propertyColor.IsValid () ) {
			FbxDouble3 color =propertyColor.Get () ;
			value =(color [0] * value + color [1] * value + color [2] * value) / 3.0 ;
		}
	}
	if ( !GetIOSettings ()->GetBoolProp (IOSN_FBX_GLTF_INVERTTRANSPARENCY, false) )
		value =1.0 - value ;
	values [pszName] =web::json::value::number (value) ;
	techniqueParameters [pszName] =web::json::value::object ({ { U("type"), IOglTF::FLOAT } }) ;
	return (ret) ;
}

//web::json::value gltfWriter::WriteMaterialTransparencyParameter (
//	const utility::char_t *pszName, 
//	FbxPropertyT<FbxDouble> &property, 
//	web::json::value &values, web::json::value &techniqueParameters
//) {
//	web::json::value ret =web::json::value::null () ;
//	if ( !property.IsValid () )
//		return (ret) ;
//	double value =property.Get () ;
//	if ( !GetIOSettings ()->GetBoolProp (IOSN_FBX_GLTF_INVERTTRANSPARENCY, false) )
//		value =1.0 - value ;
//	values [pszName] =web::json::value::number (value) ;
//	techniqueParameters [pszName] =web::json::value::object ({ { U("type"), IOglTF::FLOAT } }) ;
//	return (ret) ;
//}

web::json::value gltfWriter::WriteMaterialParameter (const utility::char_t *pszName, FbxPropertyT<FbxDouble3> &property, double factor, web::json::value &values, web::json::value &techniqueParameters) {
	web::json::value ret =web::json::value::null () ;
	if ( !property.IsValid () )
		return (ret) ;
	if ( property.GetSrcObjectCount<FbxTexture> () != 0 ) {
		FbxTexture *pTexture =property.GetSrcObject<FbxTexture> (0) ;
		values [pszName] =web::json::value::string (createTextureName (pTexture->GetName ())) ;
		techniqueParameters [pszName] =web::json::value::object ({{ U("type"), IOglTF::SAMPLER_2D }}) ;
		ret =WriteTexture (pTexture) ;
	} else {
		FbxDouble3 color =property.Get () ;
		MultiplyDouble3By (color, factor) ;
		values [pszName] =web::json::value::array ({ { color [0], color [1], color [2], 1. } }) ;
		techniqueParameters [pszName] =web::json::value::object ({{ U("type"), IOglTF::FLOAT_VEC4 }}) ;
	}
	return (ret) ;
}

web::json::value gltfWriter::WriteMaterialParameter (const utility::char_t *pszName, FbxPropertyT<FbxDouble> &property, web::json::value &values, web::json::value &techniqueParameters) {
	web::json::value ret =web::json::value::null () ;
	if ( !property.IsValid () )
		return (ret) ;
	if ( property.GetSrcObjectCount<FbxTexture> () != 0 ) {
		FbxTexture *pTexture =property.GetSrcObject<FbxTexture> (0) ;
		values [pszName] =web::json::value::string (createTextureName (pTexture->GetName ())) ;
		techniqueParameters [pszName] =web::json::value::object ({ { U("type"), IOglTF::SAMPLER_2D } }) ;
		ret =WriteTexture (pTexture) ;
	} else {
		double value =property.Get () ;
		values [pszName] =web::json::value::number (value) ;
		techniqueParameters [pszName] =web::json::value::object ({ { U("type"), IOglTF::FLOAT } }) ;
	}
	return (ret) ;
}

web::json::value gltfWriter::WriteMaterialParameter (const utility::char_t *pszName, FbxSurfaceMaterial *pMaterial, const char *propertyName, const char *factorName, web::json::value &values, web::json::value &techniqueParameters) {
	web::json::value ret =web::json::value::null () ;
	FbxProperty property =pMaterial->FindProperty (propertyName, FbxColor3DT, false) ;
	if ( property.IsValid () && property.GetSrcObjectCount<FbxTexture> () != 0 ) {
		FbxTexture *pTexture =property.GetSrcObject<FbxTexture> (0) ;
		values [pszName] =web::json::value::string (createTextureName (pTexture->GetName ())) ;
		techniqueParameters [pszName] =web::json::value::object ({{ U("type"), IOglTF::SAMPLER_2D }}) ;
		ret =WriteTexture (pTexture) ;
	} else {
		FbxProperty factorProperty =pMaterial->FindProperty (factorName, FbxDoubleDT, false) ;
		double factor =factorProperty.IsValid () ? factorProperty.Get<FbxDouble> () : 1. ;
		FbxDouble3 color =property.IsValid () ? property.Get<FbxDouble3> () : FbxDouble3 (1., 1., 1.) ;
		if ( !property.IsValid () && !factorProperty.IsValid () )
			factor =0. ;
		MultiplyDouble3By (color, factor) ;
		values [pszName] =web::json::value::array ({ { color [0], color [1], color [2], 1. } }) ;
		techniqueParameters [pszName] =web::json::value::object ({{ U("type"), IOglTF::FLOAT_VEC4 }}) ;
	}
	return (ret) ;
}

web::json::value gltfWriter::WritePhongMaterial (FbxNode *pNode, FbxSurfaceMaterial *pMaterial) {
	web::json::value values =web::json::value::object () ;
	web::json::value techniqueParameters =web::json::value::object () ;
	FbxSurfacePhong *pPhongSurface =FbxCast<FbxSurfacePhong> (pMaterial) ;

	web::json::value ret =web::json::value::object () ;
	MergeJsonObjects (ret, WriteMaterialParameter (U("ambient"), pPhongSurface->Ambient, pPhongSurface->AmbientFactor.Get (), values, techniqueParameters)) ;
	MergeJsonObjects (ret, WriteMaterialParameter (U("diffuse"), pPhongSurface->Diffuse, pPhongSurface->DiffuseFactor.Get (), values, techniqueParameters)) ;
	MergeJsonObjects (ret, WriteMaterialParameter (U("emission"), pPhongSurface->Emissive, pPhongSurface->EmissiveFactor.Get (), values, techniqueParameters)) ;
	MergeJsonObjects (ret, WriteMaterialParameter (U("specular"), pPhongSurface->Specular, pPhongSurface->SpecularFactor.Get (), values, techniqueParameters)) ;
	MergeJsonObjects (ret, WriteMaterialParameter (U("shininess"), pPhongSurface->Shininess, values, techniqueParameters)) ;
	MergeJsonObjects (ret, WriteMaterialParameter (U("reflective"), pPhongSurface->Reflection, 1., values, techniqueParameters)) ;
	MergeJsonObjects (ret, WriteMaterialParameter (U("reflectivity"), pPhongSurface->ReflectionFactor, values, techniqueParameters)) ;
	MergeJsonObjects (ret, WriteMaterialParameter (U("transparent"), pPhongSurface->TransparentColor, 1., values, techniqueParameters)) ;

	// gltf - opaque is 1. / transparency is 0.
	// pPhongSurface->TransparentColor // Transparent color property.
	// pPhongSurface->TransparencyFactor // Transparency factor property. This property is used to make a surface more or less opaque (0 = opaque, 1 = transparent).
	MergeJsonObjects (ret, WriteMaterialTransparencyParameter (
		U("transparency"),
		pPhongSurface->TransparencyFactor, pPhongSurface->TransparentColor, pPhongSurface->FindProperty ("Opacity"),
		values, techniqueParameters
	)) ;

	// todo bumpScale? https://github.com/zfedoran/convert-to-threejs-json/blob/master/convert_to_threejs.py #364
	// 		eTextureNormalMap,
	//		eTextureBump,
	//		eTextureDisplacement,
	//		eTextureDisplacementVector,

	// Note: 
	// INDEXOFREFRACTION is not supported by FBX.

	MergeJsonObjects (ret, web::json::value::object ({ { U("values"), values }, { U("techniqueParameters"), techniqueParameters } })) ;
	return (ret) ;
}

web::json::value gltfWriter::WriteLambertMaterial (FbxNode *pNode, FbxSurfaceMaterial *pMaterial) {
	web::json::value values =web::json::value::object () ;
	web::json::value techniqueParameters =web::json::value::object () ;
	FbxSurfaceLambert *pLambertSurface =FbxCast<FbxSurfaceLambert> (pMaterial) ;

	web::json::value ret =web::json::value::object () ;
	MergeJsonObjects (ret, WriteMaterialParameter (U("ambient"), pLambertSurface->Ambient, pLambertSurface->AmbientFactor.Get (), values, techniqueParameters)) ;
	MergeJsonObjects (ret, WriteMaterialParameter (U("diffuse"), pLambertSurface->Diffuse, pLambertSurface->DiffuseFactor.Get (), values, techniqueParameters)) ;
	MergeJsonObjects (ret, WriteMaterialParameter (U("emission"), pLambertSurface->Emissive, pLambertSurface->EmissiveFactor.Get (), values, techniqueParameters)) ;
	values [U("reflectivity")] =web::json::value::number (1.) ;
	techniqueParameters [U("reflectivity")] =web::json::value::object ({{ U("type"), IOglTF::FLOAT }}) ;
	MergeJsonObjects (ret, WriteMaterialParameter (U("transparent"), pLambertSurface->TransparentColor, 1., values, techniqueParameters)) ;

	// gltf - opaque is 1. / transparency is 0.
	// pPhongSurface->TransparentColor // Transparent color property.
	// pPhongSurface->TransparencyFactor // Transparency factor property. This property is used to make a surface more or less opaque (0 = opaque, 1 = transparent).
	MergeJsonObjects (ret, WriteMaterialTransparencyParameter (
		U("transparency"),
		pLambertSurface->TransparencyFactor, pLambertSurface->TransparentColor, pLambertSurface->FindProperty ("Opacity"),
		values, techniqueParameters
	)) ;

	// Note: 
	// REFLECTIVITY, INDEXOFREFRACTION are not supported by FBX.

	//return (web::json::value::object ({ { U("values"), values }, { U("techniqueParameters"), techniqueParameters } })) ;
	MergeJsonObjects (ret, web::json::value::object ({ { U("values"), values }, { U("techniqueParameters"), techniqueParameters } })) ;
	return (ret) ;
}

web::json::value gltfWriter::WriteConstantShadingModelMaterial (FbxNode *pNode, FbxSurfaceMaterial *pMaterial) {
	web::json::value values =web::json::value::object () ;
	web::json::value techniqueParameters =web::json::value::object () ;

	web::json::value ret =web::json::value::object () ;
	MergeJsonObjects (ret, WriteMaterialParameter (U("emission"), pMaterial, FbxSurfaceMaterial::sEmissive, FbxSurfaceMaterial::sEmissiveFactor, values, techniqueParameters)) ;
	FbxPropertyT<FbxDouble> factorProperty =pMaterial->FindProperty (FbxSurfaceMaterial::sReflectionFactor, FbxDoubleDT, false) ;
	MergeJsonObjects (ret, WriteMaterialParameter (U("reflectivity"), factorProperty, values, techniqueParameters)) ;
	FbxPropertyT<FbxDouble3> colorProperty =pMaterial->FindProperty (FbxSurfaceMaterial::sTransparentColor, FbxDouble3DT, false) ;
	MergeJsonObjects (ret, WriteMaterialParameter (U("transparent"), colorProperty, 1., values, techniqueParameters)) ;
	
	// gltf - opaque is 1. / transparency is 0.
	// pPhongSurface->TransparentColor // Transparent color property.
	// pPhongSurface->TransparencyFactor // Transparency factor property. This property is used to make a surface more or less opaque (0 = opaque, 1 = transparent).
	factorProperty =pMaterial->FindProperty (FbxSurfaceMaterial::sTransparencyFactor, FbxDoubleDT, false) ;
	MergeJsonObjects (ret, WriteMaterialTransparencyParameter (
		U("transparency"),
		factorProperty, colorProperty, pMaterial->FindProperty ("Opacity"),
		values, techniqueParameters
	)) ;

	// Note: 
	// REFLECTIVE, INDEXOFREFRACTION are not supported by FBX.

	MergeJsonObjects (ret, web::json::value::object ({ { U("values"), values }, { U("techniqueParameters"), techniqueParameters } })) ;
	return (ret) ;
}

web::json::value gltfWriter::WriteBlinnShadingModelMaterial (FbxNode *pNode, FbxSurfaceMaterial *pMaterial) {
	web::json::value values =web::json::value::object () ;
	web::json::value techniqueParameters =web::json::value::object () ;

	web::json::value ret =web::json::value::object () ;
	MergeJsonObjects (ret, WriteMaterialParameter (U("ambient"), pMaterial, FbxSurfaceMaterial::sAmbient, FbxSurfaceMaterial::sAmbientFactor, values, techniqueParameters)) ;
	MergeJsonObjects (ret, WriteMaterialParameter (U("diffuse"), pMaterial, FbxSurfaceMaterial::sDiffuse, FbxSurfaceMaterial::sDiffuseFactor, values, techniqueParameters)) ;
	MergeJsonObjects (ret, WriteMaterialParameter (U("emission"), pMaterial, FbxSurfaceMaterial::sEmissive, FbxSurfaceMaterial::sEmissiveFactor, values, techniqueParameters)) ;
	MergeJsonObjects (ret, WriteMaterialParameter (U("specular"), pMaterial, FbxSurfaceMaterial::sSpecular, FbxSurfaceMaterial::sSpecularFactor, values, techniqueParameters)) ;
	FbxPropertyT<FbxDouble> factorProperty =pMaterial->FindProperty (FbxSurfaceMaterial::sShininess, FbxDoubleDT, false) ;
	MergeJsonObjects (ret, WriteMaterialParameter (U("shininess"), factorProperty, values, techniqueParameters)) ;
	FbxPropertyT<FbxDouble3> colorProperty =pMaterial->FindProperty (FbxSurfaceMaterial::sReflection, FbxDouble3DT, false) ;
	MergeJsonObjects (ret, WriteMaterialParameter (U("reflective"), colorProperty, 1., values, techniqueParameters)) ;
	factorProperty =pMaterial->FindProperty (FbxSurfaceMaterial::sReflectionFactor, FbxDoubleDT, false) ;
	MergeJsonObjects (ret, WriteMaterialParameter (U("reflectivity"), factorProperty, values, techniqueParameters)) ;
	colorProperty =pMaterial->FindProperty (FbxSurfaceMaterial::sTransparentColor, FbxDouble3DT, false) ;
	MergeJsonObjects (ret, WriteMaterialParameter (U("transparent"), colorProperty, 1., values, techniqueParameters)) ;

	// gltf - opaque is 1. / transparency is 0.
	// pPhongSurface->TransparentColor // Transparent color property.
	// pPhongSurface->TransparencyFactor // Transparency factor property. This property is used to make a surface more or less opaque (0 = opaque, 1 = transparent).
	factorProperty =pMaterial->FindProperty (FbxSurfaceMaterial::sTransparencyFactor, FbxDoubleDT, false) ;
	MergeJsonObjects (ret, WriteMaterialTransparencyParameter (
		U("transparency"),
		factorProperty, colorProperty, pMaterial->FindProperty ("Opacity"),
		values, techniqueParameters
	)) ;

	// Note: 
	// INDEXOFREFRACTION is not supported by FBX.

	MergeJsonObjects (ret, web::json::value::object ({ { U("values"), values }, { U("techniqueParameters"), techniqueParameters } })) ;
	return (ret) ;
}

web::json::value gltfWriter::WriteDefaultShadingModelWithCGFXMaterial (FbxNode *pNode, FbxSurfaceMaterial *pMaterial) {
	// Set default material to constant, in case when the reader doesn't support NVidia extension.
	//TODO: hlsl and CGFX support

	_ASSERTE( false ) ;
	return (web::json::value::null ()) ;
}

web::json::value gltfWriter::WriteDefaultShadingModelMaterial (FbxNode *pNode, FbxSurfaceMaterial *pMaterial) {
	web::json::value values =web::json::value::object () ;
	web::json::value techniqueParameters =web::json::value::object () ;

	web::json::value ret =web::json::value::object () ;
	MergeJsonObjects (ret, WriteMaterialParameter (U("ambient"), pMaterial, FbxSurfaceMaterial::sAmbient, FbxSurfaceMaterial::sAmbientFactor, values, techniqueParameters)) ;
	MergeJsonObjects (ret, WriteMaterialParameter (U("diffuse"), pMaterial, FbxSurfaceMaterial::sDiffuse, FbxSurfaceMaterial::sDiffuseFactor, values, techniqueParameters)) ;
	MergeJsonObjects (ret, WriteMaterialParameter (U("emission"), pMaterial, FbxSurfaceMaterial::sEmissive, FbxSurfaceMaterial::sEmissiveFactor, values, techniqueParameters)) ;
	MergeJsonObjects (ret, WriteMaterialParameter (U("specular"), pMaterial, FbxSurfaceMaterial::sSpecular, FbxSurfaceMaterial::sSpecularFactor, values, techniqueParameters)) ;
	//FbxProperty<FbxDouble> factorProperty =pMaterial->FindProperty (FbxSurfaceMaterial::sShininess, FbxDoubleDT, false) ;
	FbxPropertyT<FbxDouble> factorProperty =pMaterial->FindProperty (FbxSurfaceMaterial::sShininess, FbxDoubleDT, false) ;
	MergeJsonObjects (ret, WriteMaterialParameter (U("shininess"), factorProperty, values, techniqueParameters)) ;
	FbxPropertyT<FbxDouble3> colorProperty =pMaterial->FindProperty (FbxSurfaceMaterial::sReflection, FbxDouble3DT, false) ;
	MergeJsonObjects (ret, WriteMaterialParameter (U("reflective"), colorProperty, 1., values, techniqueParameters)) ;
	factorProperty =pMaterial->FindProperty (FbxSurfaceMaterial::sReflectionFactor, FbxDoubleDT, false) ;
	MergeJsonObjects (ret, WriteMaterialParameter (U("reflectivity"), factorProperty, values, techniqueParameters)) ;
	colorProperty =pMaterial->FindProperty (FbxSurfaceMaterial::sTransparentColor, FbxDouble3DT, false) ;
	MergeJsonObjects (ret, WriteMaterialParameter (U("transparent"), colorProperty, 1., values, techniqueParameters)) ;
	
	// gltf - opaque is 1. / transparency is 0.
	// pPhongSurface->TransparentColor // Transparent color property.
	// pPhongSurface->TransparencyFactor // Transparency factor property. This property is used to make a surface more or less opaque (0 = opaque, 1 = transparent).
	factorProperty =pMaterial->FindProperty (FbxSurfaceMaterial::sTransparencyFactor, FbxDoubleDT, false) ;
	MergeJsonObjects (ret, WriteMaterialTransparencyParameter (
		U("transparency"),
		factorProperty, colorProperty, pMaterial->FindProperty ("Opacity"),
		values, techniqueParameters
	)) ;

	// Note: 
	// INDEXOFREFRACTION is not supported by FBX.

	MergeJsonObjects (ret, web::json::value::object ({ { U("values"), values }, { U("techniqueParameters"), techniqueParameters } })) ;
	return (ret) ;
}

#define AssignDefaultColor(pszName, defaultColor) \
	{ \
		values [pszName] =web::json::value::array ({ { defaultColor [0], defaultColor [1], defaultColor [2], 1. } }) ; \
		techniqueParameters [pszName] =web::json::value::object ({{ U("type"), IOglTF::FLOAT_VEC4 }}) ; \
	}

web::json::value gltfWriter::WriteDefaultShadingModelMaterial (FbxNode *pNode) {
	web::json::value values =web::json::value::object () ;
	web::json::value techniqueParameters =web::json::value::object () ;

	web::json::value ret =web::json::value::object () ;
	//FbxScene *pScene =pNode->GetScene () ;
	//FbxColor ambient (pScene->GetGlobalSettings ().GetAmbientColor ()) ;
	//AssignDefaultColor (U("ambient"), ambient) ;

	FbxProperty property =pNode->GetNodeAttribute ()->Color ;
	FbxDouble3 color =property.IsValid () ? property.Get<FbxDouble3> () : FbxNodeAttribute::sDefaultColor ;
	AssignDefaultColor (U("diffuse"), color) ;

	MergeJsonObjects (ret, web::json::value::object ({ { U("values"), values }, { U("techniqueParameters"), techniqueParameters } })) ;
	return (ret) ;
}

}
