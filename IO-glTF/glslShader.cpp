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

//utility::string_t vs [8] ={
//	U("position"), U("normal"), U("joint"), U("jointMat"), U("weight"),
//	U("normalMatrix"), U("modelViewMatrix"), U("projectionMatrix")
//} ;
//
//utility::string_t fs [8] ={
//	U("position"), U("normal"), U("joint"), U("jointMat"), U("weight"),
//	U("normalMatrix"), U("modelViewMatrix"), U("projectionMatrix")
//} ;

glslShader::glslShader (const utility::char_t *glslVersion /*=nullptr*/) {
	_declarations =U("") ;
	if ( glslVersion )
		_declarations +=U("#version ") + utility::string_t (glslVersion) + U("\n") ;
	_declarations +=U("precision highp float ;\n") ;
	_body =U("") ;
}

glslShader::~glslShader () {
}

utility::string_t &glslShader::name () {
	return (_name) ;
}

void glslShader::_addDeclaration (utility::string_t qualifier, utility::string_t symbol, unsigned int type, size_t count /*=1*/, bool forcesAsAnArray /*=false*/) {
	if ( this->hasSymbol (symbol) == false ) {
		utility::string_t szType =IOglTF::glslShaderType (type) ;
		utility::string_t declaration =qualifier + U(" ") + szType + U(" ") + symbol ;
		if ( count > 1 || forcesAsAnArray == true )
			declaration +=U("[") + utility::conversions::to_string_t ((int)count) + U("]") ;
		declaration +=U(" ;\n") ;
		_declarations += declaration ;

		_allSymbols [symbol] =symbol ;
	}
}

void glslShader::addAttribute (utility::string_t symbol, unsigned int type, size_t count /*=1*/, bool forcesAsAnArray /*=false*/) {
	symbol =U("a_") +  symbol ;
	_addDeclaration (U("attribute"), symbol, type, count, forcesAsAnArray) ;
}

void glslShader::addUniform (utility::string_t symbol, unsigned int type, size_t count /*=1*/, bool forcesAsAnArray /*=false*/) {
	symbol =U("u_") +  symbol ;
	_addDeclaration (U("uniform"), symbol, type, count, forcesAsAnArray) ;
}

void glslShader::addVarying (utility::string_t symbol, unsigned int type, size_t count /*=1*/, bool forcesAsAnArray /*=false*/) {
	symbol =U("v_") +  symbol ;
	_addDeclaration (U("varying"), symbol, type, count, forcesAsAnArray) ;
}

bool glslShader::hasSymbol (const utility::string_t &symbol) {
	return (_allSymbols.count (symbol) > 0) ;
}

void glslShader::appendCode (const char *format, ...) {
	char buffer [1000] ;
	va_list args ;
	va_start (args, format) ;
#if defined(_WIN32) || defined(_WIN64)
	vsprintf_s (buffer, format, args) ;
#else
	vsprintf (buffer, format, args) ;
#endif
	_body +=utility::conversions::to_string_t (buffer) ;
	va_end (args) ;
}

void glslShader::appendCode (const utility::char_t *format, ...) {
	utility::char_t buffer [1000] ;
	va_list args ;
	va_start (args, format) ;
#if defined(_WIN32) || defined(_WIN64)
	_vstprintf_s (buffer, format, args) ;
#else
	vsprintf (buffer, format, args) ;
#endif
	_body +=buffer ;
	va_end (args) ;
}

//void glslShader::appendCode (const utility::char_t *code) {
//	_body +=code ;
//}

utility::string_t glslShader::source () {
	return (_declarations + body ()) ;
}

utility::string_t glslShader::body () {
	return (U("void main (void) {\n") + _body + U("}\n")) ;
}

//-----------------------------------------------------------------------------
glslTech::glslTech (web::json::value technique, web::json::value values, web::json::value gltf, const utility::char_t *glslVersion)
	: _vertexShader (glslVersion), _fragmentShader (glslVersion),
	_bHasNormals (false), _bHasJoint (false), _bHasWeight (false), _bHasSkin (false), _bHasTexTangent (false), _bHasTexBinormal (false),
	_bModelContainsLights (false), _bLightingIsEnabled (false), _bHasAmbientLight (false), _bHasSpecularLight (false), _bHasNormalMap (false)
{
	prepareParameters (technique) ;
	hwSkinning () ;
	lighting1 (technique, gltf) ;
	texcoords (technique) ;
	lighting2 (technique, gltf) ;
	finalizingShaders (technique, gltf) ;
}

glslTech::~glslTech () {
}

/*static*/ utility::string_t glslTech::format (const utility::char_t *format, ...) {
	utility::char_t buffer [1000] ;
	va_list args ;
	va_start (args, format) ;
#if defined(_WIN32) || defined(_WIN64)
	_vstprintf_s (buffer, format, args) ;
#else
	vsprintf (buffer, format, args) ;
#endif
	va_end (args) ;
	return (buffer) ;
}

const utility::string_t glslTech::needsVarying (const utility::char_t *semantic) {
	static utility::string_t varyings [] ={
		U("^position$"), U("^normal$"),
		U("^texcoord([0-9]+)$")
		//U("^light([0-9]+)Transform$")
	} ;

	//static utility::string_t varyingsMapping [] ={
	//	U("texcoord([0-9]+)"), U("texcoord%s"),
	//	U("light([0-9]+)Transform"), U("light%sDirection")
	//} ;

	int nb =sizeof (varyings) / sizeof (utility::string_t) ;
	//int nb2 =sizeof (varyingsMapping) / sizeof (utility::string_t) / 2 ;
	for ( int i =0 ; i < nb ; i++ ) {
		uregex regex (varyings [i], std::regex_constants::ECMAScript | std::regex_constants::icase) ;
		if ( std::regex_search (semantic, regex) ) {
			/*for ( int j =0 ; j < nb2 ; j++ ) {
				if ( varyings [i] == varyingsMapping [j * 2] ) {
					uregex regex2 (varyings [i]) ;
					utility::string_t st (semantic) ;

					//umatch matches ;
					//if ( std::regex_match (st.cbegin (), st.cend (), matches, regex2) )
					//	ucout << L"The matching text is:" << matches.str () << std::endl ;

					umatch mr ;
					//bool results =std::regex_search (st.cbegin (), st.cend (), regex2, std::regex_constants::match_default) ;
					bool results =std::regex_search (st, mr, regex2) ;
					//for ( size_t k =0 ; k < mr.size () ; ++k )
					//	ucout << k << U(" ") << mr [k].str () << std::endl ;
					utility::string_t st2 =format (varyingsMapping [j * 2 + 1].c_str (), mr [1].str ().c_str ()) ;
					return (st2) ;
				}
			}*/
			return (varyings [i]) ;
		}
	}
	return (U("")) ;
}

/*static*/ bool glslTech::isVertexShaderSemantic (const utility::char_t *semantic) {
	static utility::string_t vs_semantics [] ={
		U("^position$"), U("^normal$"), U("^normalMatrix$"), U("^modelViewMatrix$"), U("^projectionMatrix$"),
		U("^texcoord([0-9]+)$"), U("^light([0-9]+)Transform$")
	} ;
	int nb =sizeof (vs_semantics) / sizeof (utility::string_t) ;
	for ( int i =0 ; i < nb ; i++ ) {
		uregex regex (vs_semantics [i], std::regex_constants::ECMAScript | std::regex_constants::icase) ;
		if ( std::regex_search (semantic, regex) ) 
			return (true) ;
	}
	return (false) ;
}

/*static*/ bool glslTech::isFragmentShaderSemantic (const utility::char_t *semantic) {
	static utility::string_t fs_semantics [] ={
		U("^ambient$"), U("^diffuse$"), U("^emission$"),  U("^specular$"),  U("^shininess$"),
		U("^reflective$"), U("^reflectivity$"), U("^transparent$"), U("^transparency$"),
		U("^light([0-9]+)Color$")
	} ;
	int nb =sizeof (fs_semantics) / sizeof (utility::string_t) ;
	for ( int i =0 ; i < nb ; i++ ) {
		uregex regex (fs_semantics [i], std::regex_constants::ECMAScript | std::regex_constants::icase) ;
		if ( std::regex_search (semantic, regex) ) 
			return (true) ;
	}
	return (false) ;
}

void glslTech::prepareParameters (web::json::value technique) {
	// Parameters / attribute - uniforms - varying
	web::json::value parameters =technique [U("parameters")] ;
	for ( auto iter =parameters.as_object ().begin () ; iter != parameters.as_object ().end () ; iter++ ) {
		unsigned int iType =iter->second.as_object () [U("type")].as_integer () ;
		bool bIsAttribute =technique [U("attributes")].has_field (U("a_") + iter->first) ;
		
		if ( glslTech::isVertexShaderSemantic (iter->first.c_str ()) ) {
			if ( bIsAttribute )
				_vertexShader.addAttribute (iter->first, iType) ;
			else
				_vertexShader.addUniform (iter->first, iType) ;
			utility::string_t v =needsVarying (iter->first.c_str ()) ;
			if ( v != U("") ) {
				_vertexShader.addVarying (iter->first, iType) ;
				_fragmentShader.addVarying (iter->first, iType) ;
			}
		}
		if ( glslTech::isFragmentShaderSemantic (iter->first.c_str ()) ) {
			if ( bIsAttribute )
				_fragmentShader.addAttribute (iter->first, iType) ;
			else
				_fragmentShader.addUniform (iter->first, iType) ;
		}

		if ( iter->second.has_field (U("semantic")) ) {
			utility::string_t semantic =iter->second.as_object() [U("semantic")].as_string () ;
			_bHasNormals |=semantic == U("NORMAL") ;
			_bHasJoint |=semantic == U("JOINT") ;
			_bHasWeight |=semantic == U("WEIGHT") ;
			_bHasTexTangent |=semantic == U("TEXTANGENT") ;
			_bHasTexBinormal |=semantic == U("TEXBINORMAL") ;
		}
	}
	_bHasSkin =_bHasJoint && _bHasWeight ;
}

void glslTech::hwSkinning () {
	// Handle hardware skinning, for now with a fixed limit of 4 influences
	if ( _bHasSkin ) {
		_vertexShader.appendCode (U("mat4 skinMat =a_weight.x * u_jointMat [int(a_joint.x)] ;\n")) ;
		_vertexShader.appendCode (U("skinMat +=a_weight.y * u_jointMat [int(a_joint.y)] ;\n")) ;
		_vertexShader.appendCode (U("skinMat +=a_weight.z * u_jointMat [int(a_joint.z)] ;\n")) ;
		_vertexShader.appendCode (U("skinMat +=a_weight.w * u_jointMat [int(a_joint.w)] ;\n")) ;
		_vertexShader.appendCode (U("vec4 pos =u_modelViewMatrix * skinMat * vec4(a_position, 1.0) ;\n")) ;
		if ( _bHasNormals )
			_vertexShader.appendCode (U("v_normal =u_normalMatrix * mat3(skinMat) * a_normal ;\n")) ;
	} else {
		_vertexShader.appendCode (U("vec4 pos =u_modelViewMatrix * vec4(a_position, 1.0) ;\n")) ;
		if ( _bHasNormals )
			_vertexShader.appendCode (U("v_normal =u_normalMatrix * a_normal ;\n")) ;
	}
}

bool glslTech::lighting1 (web::json::value technique, web::json::value gltf) {
	// Lighting
	web::json::value lights =gltf [U("lights")] ;
	for ( auto iter =lights.as_object ().begin () ; iter != lights.as_object ().end () ; iter++ )
		_bModelContainsLights |=iter->second.as_object () [U("type")].as_string () != U("ambient") ;
	_bLightingIsEnabled =_bHasNormals && (_bModelContainsLights || true) ; // todo - default lighting option
	if ( _bLightingIsEnabled ) {
		_fragmentShader.appendCode (U("vec3 normal =normalize (v_normal) ;\n")) ;
		bool bDoubledSide =technique [U("extras")] [U("doubleSided")].as_bool () ;
		if ( bDoubledSide )
			_fragmentShader.appendCode ("if ( gl_FrontFacing == false ) normal =-normal ;\n") ;
	} else {
		// https://github.com/KhronosGroup/glTF/issues/121
		// We want to keep consistent the parameter in the instanceTechnique and the ones actually in use in the technique.
		// Given the context, some parameters from instanceTechnique will be removed because they aren't used in the resulting
		// shader. As an example, we may have no light declared and the default lighting disabled == no lighting at all, but 
		// still a specular color and a shininess. in this case specular and shininess won't be used.
	}

	// Color to cumulate all components and light contribution
	_fragmentShader.appendCode (U("vec4 color =vec4(0., 0., 0., 0.) ;\n")) ;
	_fragmentShader.appendCode (U("vec4 diffuse =vec4(0., 0., 0., 1.) ;\n")) ;
	if ( _bModelContainsLights )
		_fragmentShader.appendCode (U("vec3 diffuseLight =vec3(0., 0., 0.) ;\n")) ;
	web::json::value parameters =technique [U("parameters")] ;
	if ( parameters.has_field (U("emission")) )
		_fragmentShader.appendCode (U("vec4 emission ;\n")) ;
	if ( parameters.has_field (U("reflective")) )
		_fragmentShader.appendCode (U("vec4 reflective ;\n")) ;
	if ( _bLightingIsEnabled && parameters.has_field (U("ambient")) )
		_fragmentShader.appendCode (U("vec4 ambient ;\n")) ;
	if ( _bLightingIsEnabled && parameters.has_field (U("specular")) )
		_fragmentShader.appendCode ("vec4 specular ;\n") ;
	return (_bLightingIsEnabled) ;
}

void glslTech::texcoords (web::json::value technique) {
	web::json::value parameters =technique [U("parameters")] ;
	utility::string_t texcoordAttributeSymbol =U("a_texcoord") ;
	utility::string_t texcoordVaryingSymbol =U("v_texcoord") ;
	std::map<utility::string_t, utility::string_t> declaredTexcoordAttributes ;
	std::map<utility::string_t, utility::string_t> declaredTexcoordVaryings ;
	utility::string_t slots []={ U("ambient"), U("diffuse"), U("emission"), U("reflective"), U("specular"), U("bump") } ;
	const int slotsCount =sizeof (slots) / sizeof (utility::string_t) ;
	for ( size_t slotIndex =0 ; slotIndex < slotsCount ; slotIndex++ ) {
		utility::string_t slot =slots [slotIndex] ;
		if ( parameters.has_field (slot) == false )
			continue ;
		unsigned int slotType =parameters [slot].as_object () [U("type")].as_integer () ;
		if ( (!_bLightingIsEnabled) && ((slot == U("ambient")) || (slot == U("specular"))) ) {
			// As explained in https://github.com/KhronosGroup/glTF/issues/121 export all parameters when details is specified
//todo			addParameter (slot, slotType);
			continue ;
		}
		if ( _bLightingIsEnabled && (slot == U("bump")) ) {
			_bHasNormalMap =slotType == IOglTF::SAMPLER_2D && _bHasTexTangent && _bHasTexBinormal ;
			if ( _bHasNormalMap == true ) {
				_vertexShader.addAttribute (U("textangent"), IOglTF::FLOAT_VEC3, 1, true) ;
				_vertexShader.addAttribute (U("texbinormal"), IOglTF::FLOAT_VEC3, 1, true) ;
				_vertexShader.appendCode ("v_texbinormal =u_normalMatrix * a_texbinormal ;\n") ;
				_vertexShader.appendCode ("v_textangent =u_normalMatrix * a_textangent ;\n") ;
				_fragmentShader.appendCode ("vec4 bump ;\n") ;
			}
		}

		if ( slotType == IOglTF::FLOAT_VEC4 ) {
			utility::string_t slotColorSymbol =U("u_") + slot ;
			_fragmentShader.appendCode (U("%s =%s ;\n"), slot.c_str (), slotColorSymbol.c_str ()) ;
			//_fragmentShader.addUniformValue (slotType, 1, slot) ;
		} else if ( slotType == IOglTF::SAMPLER_2D ) {
			utility::string_t semantic =technique [U("extras")] [U("texcoordBindings")] [slot].as_string () ;
			utility::string_t texSymbol, texVSymbol ;
			if ( slot == U("reflective") ) {
				texVSymbol =texcoordVaryingSymbol + utility::conversions::to_string_t ((int)declaredTexcoordVaryings.size ()) ;
				unsigned int reflectiveType =IOglTF::FLOAT_VEC2 ;
			//	_vertexShader.addVarying (texVSymbol, reflectiveType) ;
			//	_fragmentShader.addVarying (texVSymbol, reflectiveType) ;
				// Update Vertex shader for reflection
				utility::string_t normalType =IOglTF::szVEC3 ;
				_vertexShader.appendCode (U("%s normalizedVert =normalize (%s(pos)) ;\n"), normalType.c_str (), normalType.c_str ()) ;
				_vertexShader.appendCode (U("%s r =reflect (normalizedVert, v_normal) ;\n"), normalType.c_str ()) ;
				_vertexShader.appendCode (U("r.z +=1.0 ;\n")) ;
				_vertexShader.appendCode (U("float m =2.0 * sqrt (dot (r, r)) ;\n")) ;
				_vertexShader.appendCode (U("%s =(r.xy / m) + 0.5 ;\n"), texVSymbol.c_str ()) ;
				declaredTexcoordVaryings [semantic] =texVSymbol ;
			} else {
				if ( declaredTexcoordAttributes.count (semantic) == 0 ) {
					texSymbol =texcoordAttributeSymbol + utility::conversions::to_string_t ((int)declaredTexcoordAttributes.size ()) ;
					texVSymbol =texcoordVaryingSymbol + utility::conversions::to_string_t ((int)declaredTexcoordVaryings.size ()) ;
					unsigned int texType =IOglTF::FLOAT_VEC2 ;
			//		_vertexShader.addAttribute (U("texcoord") + utility::conversions::to_string_t ((int)declaredTexcoordAttributes.size ()), texType) ;
			//		_vertexShader.addVarying (texVSymbol, texType) ;
			//		_fragmentShader.addVarying (texVSymbol, texType) ;
					_vertexShader.appendCode (U("%s =%s ;\n"), texVSymbol.c_str (), texSymbol.c_str ()) ;

					declaredTexcoordAttributes [semantic] =texSymbol ;
					declaredTexcoordVaryings [semantic] =texVSymbol ;
				} else {
					texSymbol =declaredTexcoordAttributes [semantic] ;
					texVSymbol =declaredTexcoordVaryings [semantic] ;
				}
			}

			utility::string_t textureSymbol =U("u_") + slot ;
			web::json::value textureParameter =parameters [slot] ; // get the texture
			//_vertexShader.addUniform (texVSymbol, textureParameter [U("type")].as_integer ()) ;
			//_fragmentShader.addUniform (texVSymbol, textureParameter [U("type")].as_integer ()) ;
			if ( _bHasNormalMap == false && slot == U("bump") )
				continue ;
			_fragmentShader.appendCode (U("%s =texture2D (%s, %s) ;\n"), slot.c_str (), textureSymbol.c_str (), texVSymbol.c_str ()) ;
		}
	}

	if ( _bHasNormalMap ) {
		//_fragmentShader.appendCode("vec3 binormal = normalize( cross(normal,v_textangent));\n");
		_fragmentShader.appendCode ("mat3 bumpMatrix = mat3(normalize(v_textangent), normalize(v_texbinormal), normal);\n");
		_fragmentShader.appendCode ("normal = normalize(-1.0 + bump.xyz * 2.0);\n");
	}
}

web::json::value glslTech::lightNodes (web::json::value gltf) {
	web::json::value ret =web::json::value::array () ;
	for ( auto iter : gltf [U("nodes")].as_object () ) {
		if ( iter.second.has_field (U("light")) )
			ret [ret.size ()] =web::json::value (iter.first) ;
	}
	return (ret) ;
}

void glslTech::lighting2 (web::json::value technique, web::json::value gltf) {
	web::json::value parameters =technique [U("parameters")] ;

	utility::string_t lightingModel =technique [U("extras")] [U("lightingModel")].as_string () ;
	bool bHasSpecular =
		   parameters.has_field (U("specular")) && parameters.has_field (U("shininess"))
		&& (lightingModel == U("Phong") || lightingModel == U("Blinn")) ;

	size_t lightIndex =0 ;
	size_t nbLights =gltf [U("lights")].size () ;
	if ( _bLightingIsEnabled && nbLights ) {
		web::json::value lightNodes =glslTech::lightNodes (gltf) ;
		//if ( parameters.has_field (U("shininess")) && (!shininessObject) ) {

		//std::vector<utility::string_t> ids ;
		for ( auto iter : gltf [U("lights")].as_object () ) {
			//ids.push_back (iter.first) ;
		
			utility::string_t lightType =iter.second [U("type")].as_string () ;
			// We ignore lighting if the only light we have is ambient
			if ( (lightType == U("ambient")) && nbLights == 1 )
				continue ;

			for ( size_t j =0 ; j < lightNodes.size () ; j++, lightIndex++ ) {
				// Each light needs to be re-processed for each node
				utility::string_t szLightIndex =glslTech::format (U("light%d"), (int)lightIndex) ;
				utility::string_t szLightColor =glslTech::format (U("%sColor"), szLightIndex.c_str ()) ;
				utility::string_t szLightTransform =glslTech::format (U("%sTransform"), szLightIndex.c_str ()) ;
				utility::string_t szLightInverseTransform =glslTech::format (U("%sInverseTransform"), szLightIndex.c_str ()) ;
				utility::string_t szLightConstantAttenuation =glslTech::format (U("%sConstantAttenuation"), szLightIndex.c_str ()) ;
				utility::string_t szLightLinearAttenuation =glslTech::format (U("%sLinearAttenuation"), szLightIndex.c_str ()) ;
				utility::string_t szLightQuadraticAttenuation =glslTech::format (U("%sQuadraticAttenuation"), szLightIndex.c_str ()) ;
				utility::string_t szLightFallOffAngle =glslTech::format (U("%sFallOffAngle"), szLightIndex.c_str ()) ;
				utility::string_t szLightFallOffExponent =glslTech::format (U("%sFallOffExponent"), szLightIndex.c_str ()) ;
				
				if ( bHasSpecular ==true && _bHasSpecularLight == false ) {
					_fragmentShader.appendCode (U("vec3 specularLight =vec3(0., 0., 0.) ;\n")) ;
					_bHasSpecularLight =true ;
				}

				if ( lightType == U("ambient") ) {
					if ( _bHasAmbientLight == false ) {
						_fragmentShader.appendCode (U("vec3 ambientLight =vec3(0., 0., 0.) ;\n")) ;
						_bHasAmbientLight =true ;
					}

					_fragmentShader.appendCode (U("{\n")) ;
					//FIXME: what happens if multiple ambient light ?
					_fragmentShader.appendCode (U("ambientLight +=u_%s ;\n"), szLightColor.c_str ()) ;
					_fragmentShader.appendCode (U("}\n")) ;
				} else {
					utility::string_t szVaryingLightDirection =glslTech::format (U("%sDirection"), szLightIndex.c_str ()) ;
					_vertexShader.addVarying (szVaryingLightDirection, IOglTF::FLOAT_VEC3) ;
					_fragmentShader.addVarying (szVaryingLightDirection, IOglTF::FLOAT_VEC3) ;
					if (   /*_vertexShader.hasSymbol (U("v_position")) == false
						&&*/ (lightingModel == U("Phong") || lightingModel == U("Blinn") || lightType == U("spot"))
					) {
						//_vertexShader.addVarying (U("v_position"), IOglTF::FLOAT_VEC3) ;
						//_fragmentShader.addVarying (U("v_position"), IOglTF::FLOAT_VEC3) ;
						_vertexShader.appendCode (U("v_position =pos.xyz ;\n")) ;
					}
					_fragmentShader.appendCode (U("{\n")) ;
					_fragmentShader.appendCode (U("float specularIntensity =0. ;\n")) ;
					if ( lightType != U("directional") ) {
	//					shared_ptr <JSONObject> lightConstantAttenuationParameter=addValue ("fs", "uniform", floatType, 1, lightConstantAttenuation, asset);
	//					lightConstantAttenuationParameter->setValue ("value", description->getValue ("constantAttenuation"));
	//					shared_ptr <JSONObject> lightLinearAttenuationParameter=addValue ("fs", "uniform", floatType, 1, lightLinearAttenuation, asset);
	//					lightLinearAttenuationParameter->setValue ("value", description->getValue ("linearAttenuation"));
	//					shared_ptr <JSONObject> lightQuadraticAttenuationParameter=addValue ("fs", "uniform", floatType, 1, lightQuadraticAttenuation, asset);
	//					lightQuadraticAttenuationParameter->setValue ("value", description->getValue ("quadraticAttenuation"));
					}
	//				shared_ptr <JSONObject> lightColorParameter=addValue ("fs", "uniform", vec3Type, 1, lightColor, asset);
	//				lightColorParameter->setValue ("value", description->getValue ("color"));
	//				shared_ptr <JSONObject> lightTransformParameter=addValue ("vs", "uniform", mat4Type, 1, lightTransform, asset);
	//				lightTransformParameter->setValue (kNode, nodesIds [j]);
	//				lightTransformParameter->setString (kSemantic, MODELVIEW);

					if ( lightType == U("directional") )
						_vertexShader.appendCode (U("v_%s =mat3(u_%s) * vec3(0., 0., 1.) ;\n"), szVaryingLightDirection.c_str (), szLightTransform.c_str ()) ;
					else
						_vertexShader.appendCode (U("v_%s =u_%s [3].xyz - pos.xyz ;\n"), szVaryingLightDirection.c_str (), szLightTransform.c_str ()) ;
					_fragmentShader.appendCode (U("float attenuation =1.0 ;\n")) ;

					if ( lightType != U("directional") ) {
						// Compute light attenuation from non-normalized light direction
						_fragmentShader.appendCode (U("float range =length (%s) ;\n"), szVaryingLightDirection.c_str ()) ;
						_fragmentShader.appendCode (U("attenuation =1.0 / (u_%s + (u_%s * range) + (u_%s * range * range)) ;\n"),
							szLightConstantAttenuation.c_str (), szLightLinearAttenuation.c_str (), szLightQuadraticAttenuation.c_str ()
						) ;
					}
					// Compute lighting from normalized light direction
					_fragmentShader.appendCode (U("vec3 l =normalize (v_%s) ;\n"), szVaryingLightDirection.c_str ()) ;

					if ( lightType == U("spot") ) {
	//					shared_ptr <JSONObject> lightInverseTransformParameter=addValue ("fs", "uniform", mat4Type, 1, lightInverseTransform, asset);
	//					lightInverseTransformParameter->setValue (kNode, nodesIds [j]);
	//					lightInverseTransformParameter->setString (kSemantic, MODELVIEWINVERSE);

	//					shared_ptr <JSONObject> lightFallOffExponentParameter=addValue ("fs", "uniform", floatType, 1, lightFallOffExponent, asset);
	//					lightFallOffExponentParameter->setValue ("value", description->getValue ("fallOffExponent"));
	//					shared_ptr <JSONObject> lightFallOffAngleParameter=addValue ("fs", "uniform", floatType, 1, lightFallOffAngle, asset);
	//					lightFallOffAngleParameter->setValue ("value", description->getValue ("fallOffAngle"));

						// As in OpenGL ES 2.0 programming guide
						// Raise spec issue about the angle
						// we can test this in the shader generation
						_fragmentShader.appendCode (U("vec4 spotPosition =u_%s * vec4(v_position, 1.) ;\n"), szLightInverseTransform.c_str ()) ;
						_fragmentShader.appendCode (U("float cosAngle =dot (vec3 (0., 0., -1.), normalize (spotPosition.xyz)) ;\n")) ;
						// doing this cos each pixel is just wrong (for performance)
						// need to find a way to specify that we pass the cos of a value
						_fragmentShader.appendCode (U("if ( cosAngle > cos (radians (u_%s * 0.5)) ) {\n"), szLightFallOffAngle) ;
						_fragmentShader.appendCode (U("attenuation *=max (0., pow (cosAngle, u_%s)) ;\n"), szLightFallOffExponent) ;
					}

					// We handle phong, blinn, constant and lambert
					if ( bHasSpecular ) {
						_fragmentShader.appendCode (U("vec3 viewDir =-normalize (v_position) ;\n")) ;
						if ( lightingModel == U("Phong") ) {
							if ( _bHasNormalMap ) {
								_fragmentShader.appendCode (U("l *=bumpMatrix ;\n")) ;
								_fragmentShader.appendCode (U("position *=bumpMatrix ;\n")) ;
							}
							_fragmentShader.appendCode (U("float phongTerm =max (0.0, dot (reflect (-l, normal), viewDir)) ;\n")) ;
							_fragmentShader.appendCode (U("specularIntensity =max (0., pow (phongTerm , u_shininess)) * attenuation ;\n")) ;
						} else if ( lightingModel == U("Blinn") ) {
							_fragmentShader.appendCode (U("vec3 h =normalize (l + viewDir) ;\n")) ;
							if ( _bHasNormalMap ) {
								_fragmentShader.appendCode (U("h *=bumpMatrix ;\n")) ;
								_fragmentShader.appendCode (U("l *=bumpMatrix ;\n")) ;
							}
							_fragmentShader.appendCode (U("specularIntensity =max (0., pow (max (dot (normal, h), 0.), u_shininess)) * attenuation ;\n")) ;
						}
						_fragmentShader.appendCode (U("specularLight +=u_%s * specularIntensity ;\n"), szLightColor.c_str ()) ;
					}

					// Write diffuse
					_fragmentShader.appendCode (U("diffuseLight +=u_%s * max (dot (normal, l), 0.) * attenuation ;\n"), szLightColor.c_str ()) ;
					if ( lightType == U("spot") ) // Close previous scope beginning with "if (cosAngle > " ...
						_fragmentShader.appendCode (U("}\n")) ;
					_fragmentShader.appendCode (U("}\n")) ;
				}
			}
		}
	}
}

void glslTech::finalizingShaders (web::json::value technique, web::json::value gltf) {
	web::json::value parameters =technique [U("parameters")] ;

	if ( parameters.has_field (U("reflective")) )
		_fragmentShader.appendCode (U("diffuse.xyz +=reflective.xyz ;\n")) ;
	if ( _bHasAmbientLight && _bLightingIsEnabled && parameters.has_field (U("ambient")) ) {
		_fragmentShader.appendCode (U("ambient.xyz *=ambientLight ;\n")) ;
		_fragmentShader.appendCode (U("color.xyz +=ambient.xyz;\n")) ;
	}
	if ( _bHasSpecularLight && _bLightingIsEnabled && parameters.has_field (U("specular")) ) {
		_fragmentShader.appendCode (U("specular.xyz *=specularLight ;\n")) ;
		_fragmentShader.appendCode (U("color.xyz +=specular.xyz ;\n")) ;
	}
	if ( _bModelContainsLights )
		_fragmentShader.appendCode (U("diffuse.xyz *=diffuseLight ;\n")) ;
	else if ( _bHasNormals )
		_fragmentShader.appendCode (U("diffuse.xyz *=max (dot (normal, vec3(0., 0., 1.)), 0.) ;\n")) ;
	_fragmentShader.appendCode (U("color.xyz +=diffuse.xyz ;\n")) ;

	if ( parameters.has_field (U("emission")) )
		_fragmentShader.appendCode (U("color.xyz +=emission.xyz ;\n")) ;

	bool hasTransparency =parameters.has_field (U("transparency")) ;
	if ( hasTransparency )
		_fragmentShader.appendCode (U("color =vec4(color.rgb * diffuse.a, diffuse.a * u_transparency) ;\n")) ;
	else
		_fragmentShader.appendCode (U("color =vec4(color.rgb * diffuse.a, diffuse.a) ;\n")) ;

	_fragmentShader.appendCode (U("gl_FragColor =color ;\n")) ;
	_vertexShader.appendCode (U("gl_Position =u_projectionMatrix * pos ;\n")) ;
}

// NVidia hardware indices are reserved for built-in attributes:
// gl_Vertex			0
// gl_Normal			2
// gl_Color				3
// gl_SecondaryColor	4
// gl_FogCoord			5
// gl_MultiTexCoord0	8
// gl_MultiTexCoord1	9
// gl_MultiTexCoord2	10
// gl_MultiTexCoord3	11
// gl_MultiTexCoord4	12
// gl_MultiTexCoord5	13
// gl_MultiTexCoord6	14
// gl_MultiTexCoord7	15

//unsigned int semanticType (const utility::string_t &semantic) {
//	static std::map<utility::string_t, unsigned int> semanticTypes ;
//	if ( semantic.find (U("TEXCOORD")) != utility::string_t::npos )
//		return (IOglTF::FLOAT_VEC2) ;
//	if ( semantic.find (U("COLOR")) != utility::string_t::npos )
//		return (IOglTF::FLOAT_VEC4) ;
//	if ( semanticTypes.empty () ) {
//		// attributes
//		semanticTypes [U("POSITION")] =IOglTF::FLOAT_VEC3 ; // Babylon.js does not like VEC4
//		semanticTypes [U("NORMAL")] =IOglTF::FLOAT_VEC3;
//		semanticTypes [U("REFLECTIVE")] =IOglTF::FLOAT_VEC2 ;
//		semanticTypes [U("WEIGHT")] =IOglTF::FLOAT_VEC4 ;
//		semanticTypes [U("JOINT")] =IOglTF::FLOAT_VEC4 ;
//		semanticTypes [U("TEXTANGENT")] =IOglTF::FLOAT_VEC3 ;
//		semanticTypes [U("TEXBINORMAL")] =IOglTF::FLOAT_VEC3 ;
//		// uniforms
//		semanticTypes [U("MODELVIEWINVERSETRANSPOSE")] =IOglTF::FLOAT_MAT3 ; //typically the normal matrix
//		semanticTypes [U("MODELVIEWINVERSE")]=IOglTF::FLOAT_MAT4 ;
//		semanticTypes [U("MODELVIEW")] =IOglTF::FLOAT_MAT4 ;
//		semanticTypes [U("PROJECTION")] =IOglTF::FLOAT_MAT4 ;
//		semanticTypes [U("JOINTMATRIX")] =IOglTF::FLOAT_MAT4 ;
//	}
//	return (semanticTypes [semantic]) ;
//}

//unsigned int semanticAttributeType (const utility::string_t &semantic) {
//	static std::map<utility::string_t, unsigned int> semanticAttributeTypes ;
//	if ( semantic.find (U("TEXCOORD")) != utility::string_t::npos )
//		return (IOglTF::FLOAT_VEC2) ;
//	if ( semantic.find (U("COLOR")) != utility::string_t::npos )
//		return (IOglTF::FLOAT_VEC4) ;
//	if ( semanticAttributeTypes.empty () ) {
//		semanticAttributeTypes [U("POSITION")] =IOglTF::FLOAT_VEC3 ;
//		semanticAttributeTypes [U("NORMAL")] =IOglTF::FLOAT_VEC3;
//		semanticAttributeTypes [U("REFLECTIVE")] =IOglTF::FLOAT_VEC2 ;
//		semanticAttributeTypes [U("WEIGHT")] =IOglTF::FLOAT_VEC4 ;
//		semanticAttributeTypes [U("JOINT")] =IOglTF::FLOAT_VEC4 ;
//		semanticAttributeTypes [U("TEXTANGENT")] =IOglTF::FLOAT_VEC3 ;
//		semanticAttributeTypes [U("TEXBINORMAL")] =IOglTF::FLOAT_VEC3 ;
//	}
//	return (semanticAttributeTypes [semantic]) ;
//}
//
//unsigned int semanticUniformType (const utility::string_t &semantic) {
//	static std::map<utility::string_t, unsigned int> semanticUniformTypes ;
//	if ( semanticUniformTypes.empty () ) {
//		semanticUniformTypes [U("MODELVIEWINVERSETRANSPOSE")] =IOglTF::FLOAT_MAT3 ; //typically the normal matrix
//		semanticUniformTypes [U("MODELVIEWINVERSE")]=IOglTF::FLOAT_MAT4 ;
//		semanticUniformTypes [U("MODELVIEW")] =IOglTF::FLOAT_MAT4 ;
//		semanticUniformTypes [U("PROJECTION")] =IOglTF::FLOAT_MAT4 ;
//		semanticUniformTypes [U("JOINTMATRIX")] =IOglTF::FLOAT_MAT4 ;
//	}
//	return (semanticUniformTypes [semantic]) ;
//}
//
//enum parameterContext {
//	eAttribute,
//	eUniform,
//	eVarying
//} ;
//
//bool addSemantic (glslShader &shader, parameterContext attributeOrUniform,
//	utility::string_t semantic,
//	utility::string_t parameterID,
//	size_t count,
//	bool includesVarying,
//	bool forcesAsAnArray =false) {
//
//	utility::string_t symbol =(attributeOrUniform ? U("a_") : U("u_")) + parameterID ;
//	unsigned int type =semanticType (semantic) ;
//	shared_ptr <JSONObject> parameter (new GLTF::JSONObject ());
//	parameter->setString (kSemantic, semantic);
//	parameter->setUnsignedInt32 (kType, type);
//	_parameters->setValue (parameterID, parameter);
//	if ( attributeOrUniform == parameterContext::eAttribute )
//		_program->attributes ()->setString (symbol, parameterID);
//	else if ( uniformOrAttribute == parameterContext::eUniform )
//		_program->uniforms ()->setString (symbol, parameterID) ;
//	else
//		return (false) ;
//	if ( attributeOrUniform == parameterContext::eAttribute ) {
//		shader.addAttribute (symbol, type) ;
//		if ( includesVarying )
//			_program->addVarying ("v_" + parameterID, type) ;
//	} else {
//		shader.addUniform (symbol, type, count, forcesAsAnArray) ;
//		if ( (count > 1) || forcesAsAnArray )
//			parameter->setUnsignedInt32 (kCount, count) ;
//	}
//	return (true) ;
//}

}
