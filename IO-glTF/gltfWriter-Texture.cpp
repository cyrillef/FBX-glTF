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

// https://github.com/KhronosGroup/glTF/blob/master/specification/texture.schema.json
// https://github.com/KhronosGroup/glTF/blob/master/specification/image.schema.json
// https://github.com/KhronosGroup/glTF/blob/master/specification/sampler.schema.json

//FbxLayerElementTexture* texturesLayer =pNode->GetMesh ()->GetLayer (0)->GetTextures (FbxLayerElement::eTextureDiffuse) ;
//FbxLayerElementArrayTemplate<FbxTexture *> &arrayTextures =texturesLayer->GetDirectArray() ;
//for ( int j = 0; j < arrayTextures.GetCount (); j++ )
//	ucout << U("\t  ") << j << (".- ") << arrayTextures.GetAt (j)->GetName () << std::endl
//	   << U("\t  ") << j << (".- ") << ((FbxFileTexture *)arrayTextures.GetAt (j))->GetFileName () << std::endl;

web::json::value gltfWriter::WriteTexture (FbxTexture *pTexture) {
	utility::string_t name =utility::conversions::to_string_t (pTexture->GetNameWithoutNameSpacePrefix ().Buffer ()) ;
	utility::string_t uri =utility::conversions::to_string_t (FbxCast<FbxFileTexture> (pTexture)->GetRelativeFileName ()) ;
	web::json::value image =web::json::value::object ({{
		name, web::json::value::object ({
			{ U("name"), web::json::value::string (name) }, // https://github.com/KhronosGroup/glTF/blob/master/specification/glTFChildOfRootProperty.schema.json
			{ U("uri"), web::json::value::string (uri) }
		}) }
	}) ;
	if ( GetIOSettings ()->GetBoolProp (IOSN_FBX_GLTF_EMBEDMEDIA, false) ) {
		// data:[<mime type>][;charset=<charset>][;base64],<encoded data>
		FbxString imageFile =FbxCast<FbxFileTexture> (pTexture)->GetFileName () ;
		image [name] [U("uri")] =web::json::value::string (IOglTF::dataURI (utility::conversions::to_string_t (imageFile.Buffer ()))) ;
	} /*else*/
	if ( GetIOSettings ()->GetBoolProp (IOSN_FBX_GLTF_COPYMEDIA, false) ) {
		FbxString path =FbxPathUtils::GetFolderName (utility::conversions::to_utf8string (_fileName).c_str ()) ;
#if defined(_WIN32) || defined(_WIN64)
		path +="\\" ;
#else
		path +="/" ;
#endif
		FbxString imageFile =FbxCast<FbxFileTexture> (pTexture)->GetFileName () ;
		std::ifstream src (imageFile.Buffer (), std::ios::binary) ;
		std::ofstream dst (path + FbxPathUtils::GetFileName (imageFile), std::ios::binary) ;
		dst << src.rdbuf () ;
	}

	utility::string_t texName =createTextureName (pTexture->GetNameWithoutNameSpacePrefix ()) ;
	utility::string_t samplerName =createSamplerName (pTexture->GetNameWithoutNameSpacePrefix ()) ;
	web::json::value textureDef =web::json::value::object ({
		{ U("name"), web::json::value::string (texName) },  // https://github.com/KhronosGroup/glTF/blob/master/specification/glTFChildOfRootProperty.schema.json
		{ U("format"), web::json::value::number ((int)IOglTF::RGBA) }, // todo
		{ U("internalFormat"), web::json::value::number ((int)IOglTF::RGBA) }, // todo
		{ U("sampler"), web::json::value::string (samplerName) }, // todo do I need one everytime
		{ U("source"), web::json::value::string (name) },
		{ U("target"), web::json::value::number ((int)IOglTF::TEXTURE_2D) },
		{ U("type"), web::json::value::number ((int)IOglTF::UNSIGNED_BYTE) } // todo
	}) ;

	web::json::value texture =web::json::value::object ({{ texName, textureDef }}) ;
	
	web::json::value samplerDef =web::json::value::object ({
		{ U("name"), web::json::value::string (samplerName) },  // https://github.com/KhronosGroup/glTF/blob/master/specification/glTFChildOfRootProperty.schema.json
		{ U("magFilter"), web::json::value::number ((int)IOglTF::LINEAR) },
		{ U("minFilter"), web::json::value::number ((int)IOglTF::LINEAR_MIPMAP_LINEAR) },
		{ U("wrapS"), web::json::value::number ((int)(pTexture->WrapModeU.Get () == FbxTexture::eRepeat ? IOglTF::REPEAT : IOglTF::CLAMP_TO_EDGE)) },
		{ U("wrapT"), web::json::value::number ((int)(pTexture->WrapModeV.Get () == FbxTexture::eRepeat ? IOglTF::REPEAT : IOglTF::CLAMP_TO_EDGE)) }
	}) ;

	return (web::json::value::object ({
		{ U("images"), image },
		{ U("textures"), web::json::value::object ({{ texName, textureDef }}) },
		{ U("samplers"), web::json::value::object ({{ samplerName, samplerDef }}) }
	})) ;
}

web::json::value gltfWriter::WriteTextureBindings (FbxMesh *pMesh, FbxSurfaceMaterial *pMaterial, web::json::value &params) {
	return false ;
}

//web::json::value gltfWriter::WriteTextureBindings (FbxMesh *pMesh, FbxSurfaceMaterial *pMaterial, web::json::value &params) {
//	// Export textures used by the mesh material
//	//for ( int textureTypeIndex =(int)FbxLayerElement::sTypeTextureStartIndex ; textureTypeIndex <= (int)FbxLayerElement::sTypeTextureEndIndex ; textureTypeIndex++ ) {
//		//FbxLayerElementTexture *pLayerElementTexture =pLayer->GetTextures ((FbxLayerElement::EType)textureTypeIndex) ;
//	for ( int textureTypeIndex =0 ; textureTypeIndex < (int)FbxLayerElement::sTypeTextureCount ; textureTypeIndex++ ) {
//		FbxProperty materialProperty =pMaterial->FindProperty (FbxLayerElement::sTextureChannelNames [textureTypeIndex]) ;
//	FbxString st (FbxLayerElement::sTextureChannelNames [textureTypeIndex]) ;
//		if ( !materialProperty.IsValid () )
//			continue ;
//		// Here we have to check if it's layeredtextures, or just textures:
//		int nbLayeredTexture =materialProperty.GetSrcObjectCount<FbxLayeredTexture> () ;
//		if ( nbLayeredTexture != 0 ) {
//			for ( int j =0 ; j < nbLayeredTexture ; j++ ) {
//				FbxLayeredTexture *pLayeredTexture =materialProperty.GetSrcObject<FbxLayeredTexture> (j) ;
//				int nbTexture =pLayeredTexture->GetSrcObjectCount<FbxTexture> () ;
//				for ( int k =0 ; k < nbTexture ; k++ ) {
//					FbxTexture *pTexture =pLayeredTexture->GetSrcObject<FbxTexture> (k) ;
//					if ( pTexture ) {
//						utility::string_t name =U("texture_") + utility::conversions::to_string_t (pTexture->GetNameWithoutNameSpacePrefix ().Buffer ()) ;
//						//texture_id = getTextureName (texture, True)
//						//material_params [binding_types [str (material_property.GetName ())]] = texture_id
//					}
//				}
//			}
//		} else {
//			// No layered texture simply get on the property
//			int nbTexture =materialProperty.GetSrcObjectCount<FbxFileTexture> () ;
//			for ( int j =0 ; j < nbLayeredTexture ; j++ ) {
//				FbxTexture *pTexture =materialProperty.GetSrcObject<FbxTexture> (j) ;
//				if ( pTexture ) {
//					utility::string_t name =U("texture_") + utility::conversions::to_string_t (pTexture->GetNameWithoutNameSpacePrefix ().Buffer ()) ;
//					//texture_id = getTextureName (texture, True)
//					//material_params [binding_types [str (material_property.GetName ())]] = texture_id
//				}
//			}
//		}
//	}
//	return (false) ;
//}

/*
web::json::value gltfWriter::WriteTextureBindings (FbxMesh *pMesh, web::json::value &materials, web::json::value &techniques) {
	// Export textures used by the mesh
	int nbLayer =pMesh->GetLayerCount () ;
	for ( int layerIndex =0 ; layerIndex < nbLayer; layerIndex++ ) {
		FbxLayer *pLayer =pMesh->GetLayer (layerIndex) ;
		for ( int textureTypeIndex =(int)FbxLayerElement::sTypeTextureStartIndex ; textureTypeIndex <= (int)FbxLayerElement::sTypeTextureEndIndex ; textureTypeIndex++ ) {
			FbxLayerElementTexture *pLayerElementTexture =pLayer->GetTextures ((FbxLayerElement::EType)textureTypeIndex) ;
			if ( !pLayerElementTexture )
				continue ;
			FbxLayerElement::EMappingMode mappingMode =pLayerElementTexture->GetMappingMode () ;
			FbxLayerElement::EReferenceMode referenceMode =pLayerElementTexture->GetReferenceMode () ;
			// Number of textures in this current layer. Some textures could be used more than once.
			int nbTex =0 ;
			FbxArray<FbxTexture *> textureArray ;
			if (   referenceMode == FbxLayerElement::eDirect
				|| referenceMode == FbxLayerElement::eIndexToDirect
			) {
				nbTex =pLayerElementTexture->GetDirectArray ().GetCount () ;
			} else {
				pMesh->GetScene ()->FillTextureArray (textureArray) ;
				nbTex =textureArray.GetCount () ;
			}
			for ( int texIndex =0 ; texIndex < nbTex ; texIndex++ ) {
				FbxFileTexture *pTexture =nullptr ;
				if (   referenceMode == FbxLayerElement::eDirect
					|| referenceMode == FbxLayerElement::eIndexToDirect
				)
					pTexture =FbxCast<FbxFileTexture> (pLayerElementTexture->GetDirectArray ().GetAt (texIndex)) ;
				else
					pTexture =FbxCast<FbxFileTexture> (textureArray [texIndex]) ;
				if ( !pTexture )
					continue ;
				utility::string_t name =utility::conversions::to_string_t (pTexture->GetNameWithoutNameSpacePrefix ().Buffer ()) ;
				utility::string_t uri =utility::conversions::to_string_t (pTexture->GetRelativeFileName ()) ;
				web::json::value image =web::json::value::object ({{
						name, web::json::value::object ({
							{ U("name"), web::json::value::string (name) }, // https://github.com/KhronosGroup/glTF/blob/master/specification/glTFChildOfRootProperty.schema.json
							{ U("uri"), web::json::value::string (uri) }
						}) }
				}) ;

				utility::string_t texName (U("texture_") + name) ;
				web::json::value textureDef =web::json::value::object ({
					{ U("name"), web::json::value::string (texName) },  // https://github.com/KhronosGroup/glTF/blob/master/specification/glTFChildOfRootProperty.schema.json
		//			{ U("format"), web::json::value::string (uri) },
		//			{ U("internalFormat"), web::json::value::string (uri) },
		//			{ U("sampler"), web::json::value::string (uri) },
					{ U("source"), web::json::value::string (name) },
		//			{ U("target"), web::json::value::string (uri) },
					{ U("type"), web::json::value::number ((int)IOglTF::TEXTURE_2D) }
				}) ;

				web::json::value texture =web::json::value::object ({{ texName, textureDef }}) ;
				
				web::json::value samplerDef =web::json::value::object ({
					{ U("name"), web::json::value::string (texName) },  // https://github.com/KhronosGroup/glTF/blob/master/specification/glTFChildOfRootProperty.schema.json
					{ U("magFilter"), web::json::value::number ((int)IOglTF::LINEAR) },
					{ U("minFilter"), web::json::value::number ((int)IOglTF::LINEAR_MIPMAP_LINEAR) },
					{ U("wrapS"), web::json::value::number ((int)(pTexture->WrapModeU.Get () == FbxTexture::eRepeat ? IOglTF::REPEAT : IOglTF::CLAMP_TO_EDGE)) },
					{ U("wrapT"), web::json::value::number ((int)(pTexture->WrapModeV.Get () == FbxTexture::eRepeat ? IOglTF::REPEAT : IOglTF::CLAMP_TO_EDGE)) }
				}) ;


			}
		}
	}
	return (false) ;
}*/

//web::json::value gltfWriter::WriteTextures (FbxMesh *pMesh) {
//	// Export textures used by the mesh
//	FbxNode *pNode =pMesh->GetNode () ;
//	int nbLayer =pMesh->GetLayerCount () ;
//	for ( int layerIndex =0 ; layerIndex < nbLayer; layerIndex++ ) {
//		FbxLayer *pLayer =pMesh->GetLayer (layerIndex) ;
//		if ( !pLayer )
//			continue ;
//		int textureTypeCount =sizeof (LayerTypes) / sizeof (FbxLayerElement::EType) ;
//		int textureTypeIndex =0 ;
//		for ( textureTypeIndex =0 ; textureTypeIndex < textureTypeCount; textureTypeIndex++ ) {
//			FbxLayerElementTexture *pLayerElementTexture =pLayer->GetTextures (LayerTypes [textureTypeIndex]) ;
//			if ( !pLayerElementTexture )
//				continue ;
//			FbxString szGLTFMaterialName =GetGLTFMaterialName (textureTypeIndex) ;
//			FbxLayerElement::EMappingMode mappingMode =pLayerElementTexture->GetMappingMode () ;
//			FbxLayerElement::EReferenceMode referenceMode =pLayerElementTexture->GetReferenceMode () ;
//			// Number of textures in this current layer. Some textures could be used more than once.
//			int nbTex =0 ;
//			FbxArray<FbxTexture *> textureArray ;
//			if (   referenceMode == FbxLayerElement::eDirect
//				|| referenceMode == FbxLayerElement::eIndexToDirect
//			) {
//				nbTex =pLayerElementTexture->GetDirectArray ().GetCount () ;
//			} else {
//				pMesh->GetScene ()->FillTextureArray (textureArray) ;
//				nbTex =textureArray.GetCount () ;
//			}
//			for ( int texIndex =0 ; texIndex < nbTex ; texIndex++ ) {
//				FbxFileTexture *pTexture =nullptr ;
//				if (   referenceMode == FbxLayerElement::eDirect
//					|| referenceMode == FbxLayerElement::eIndexToDirect
//				) {
//					pTexture =FbxCast<FbxFileTexture>(pLayerElementTexture->GetDirectArray ().GetAt (texIndex)) ;
//				} else {
//					pTexture =FbxCast<FbxFileTexture>(textureArray [texIndex]) ;
//				}
//				if ( !pTexture ) {
//					_ASSERTE( false ) ;
//					//FbxString msg = FbxString ("Could not find texture ") + lTexIndex;
//					//msg += FbxString (" of layer ") + layerIndex;
//					//if ( pMesh->GetNode () ) {
//					//	msg += " in mesh for node " + pMesh->GetNode ()->GetNameWithoutNameSpacePrefix () + ".";
//					//}
//					//AddNotificationWarning (msg);
//					continue ;
//				}
//
//				FbxString szName =pTexture->GetNameWithoutNameSpacePrefix () ;
//				FbxString szTextureName =szName + "-" + szGLTFMaterialName ; // To not confuse texture id with image id
//				//FbxString imageId =ExportImage (pTexture) ;
//				// The texture has to be linked to a material in GLTF.
//				// If the texture does not use a material, create a new material.
//				// Else, if the layer already has a material, link the texture to it. 
//				// Else, if a previous layer already has a material, link the texture to it.
//				// Else, create a new material.
//				FbxArray<FbxSurfaceMaterial *> usedMaterialArray ;
//				usedMaterialArray.Clear () ;
//				if ( pTexture->GetMaterialUse () == FbxFileTexture::eDefaultMaterial ) {
//					// Texture does not use material, so create a new material.
//					// Actually the material should already have been created and it
//					// would be more efficient to just find it and use it. TODO.
//					FbxString szMaterialName =szTextureName + "-Material" ;
//					FbxSurfaceMaterial *pMaterial =FbxSurfaceMaterial::Create (&mManager, szMaterialName.Buffer ());
//					usedMaterialArray.Add (pMaterial) ;
//					if ( !pNode->IsConnectedSrcObject (pMaterial) )
//						pNode->ConnectSrcObject (pMaterial) ;
//				} else {
//					// Find the right material to export the texture
//					FbxLayerElementMaterial *pLayerElementMaterial =pLayer->GetMaterials () ;
//					if ( !pLayerElementMaterial ) {
//						// No materials on this layer.
//						// Look for materials on previous layers.
//						for ( int i =0; i < layerIndex ; i++ ) {
//							pLayerElementMaterial =pMesh->GetLayer (i)->GetMaterials () ;
//							if ( pLayerElementMaterial )
//								break ;
//						}
//					}
//					if ( !pLayerElementMaterial ) {
//						// No materials at all on this mesh, this means we have to create a material to support the texture.
//						FbxString szMaterialName =szTextureName + "-Material" ;
//						// Actually the material should already have been created and it
//						// would be more efficient to just find it and use it. TODO.
//						FbxSurfaceMaterial *pMaterial =FbxSurfaceMaterial::Create (&mManager, szMaterialName.Buffer ());
//						usedMaterialArray.Add (pMaterial) ;
//						if ( !pNode->IsConnectedSrcObject (pMaterial) )
//							pNode->ConnectSrcObject (pMaterial) ;
//					} else {
//						FbxLayerElement::EMappingMode materialMappingMode =pLayerElementMaterial->GetMappingMode () ;
//						// Get the material for the texture
//						// if the texture mapping is ALL_SAME, the first material will do.
//						// if the texture mapping is BY_POLYGON, find all polygons using that texture;
//						// then find which material each polygon is using.
//						if ( mappingMode == FbxLayerElement::eAllSame || materialMappingMode == FbxLayerElement::eAllSame ) {
//							FbxSurfaceMaterial *pMaterial =nullptr ;
//							if ( pLayerElementMaterial->GetReferenceMode () == FbxLayerElement::eDirect ) {
//								pMaterial =pNode->GetMaterial (0) ;
//							} else if ( pLayerElementMaterial->GetReferenceMode () == FbxLayerElement::eIndexToDirect ) {
//								// To include eIndexToDirect case, where GetDirectArray ().GetCount () > 0
//								int materialIndex = pLayerElementMaterial->GetIndexArray ().GetAt (0) ;
//								pMaterial =pNode->GetMaterial (materialIndex) ;
//							} else {
//								if ( pNode->GetMaterialCount () > 0 ) {
//									FbxArray<FbxSurfaceMaterial *> materialArray ;
//									pMesh->GetScene ()->FillMaterialArray (materialArray) ;
//									int materialIndex =pLayerElementMaterial->GetIndexArray ().GetAt (0) ;
//									pMaterial =materialArray.GetAt (materialIndex) ;
//									_ASSERTE( false ) ; // ???
//								} else {
//									int materialIndex =pLayerElementMaterial->GetIndexArray ().GetAt (0) ;
//									pMaterial =pNode->GetMaterial (materialIndex) ;
//								}
//							}
//							usedMaterialArray.Add (pMaterial) ;
//						}
//						// We should check if the material is by polygon, we don't really care
//						// if the textures are by polygon as the mapping mode for the material and the texture
//						// are independent...
//						else if ( materialMappingMode == FbxLayerElement::eByPolygon ) {
//							for ( int i =0 ; i < pMesh->GetPolygonCount () ; i++ ) {
//								int polygonTextureIndex =0 ;
//								if ( referenceMode == FbxLayerElement::eDirect )
//									polygonTextureIndex =texIndex ;
//								else
//									polygonTextureIndex =pLayerElementTexture->GetIndexArray ().GetAt (i) ;
//								if ( polygonTextureIndex == texIndex ) {
//									// Found first polygon using the texture
//									FbxSurfaceMaterial *pMaterial =nullptr ;
//									if ( pLayerElementMaterial->GetReferenceMode () == FbxLayerElement::eDirect ) {
//										pMaterial =pNode->GetMaterial (i) ;
//									} else if ( pLayerElementMaterial->GetReferenceMode () == FbxLayerElement::eIndexToDirect ) {
//										int materialIndex =pLayerElementMaterial->GetIndexArray ().GetAt (i) ;
//										pMaterial =pNode->GetMaterial (materialIndex) ;
//									} else {
//										FbxArray<FbxSurfaceMaterial*> materialArray ;
//										pMesh->GetScene ()->FillMaterialArray (materialArray) ;
//										int materialIndex =pLayerElementMaterial->GetIndexArray ().GetAt (i) ;
//										pMaterial =materialArray [materialIndex] ;
//									}
//									usedMaterialArray.Add (pMaterial) ;
//								}
//							}
//						} else {
//							// unsupported mapping mode.
//							//FbxString msg = FbxString ("Mapping mode not supported for textures: ") + lMappingMode;
//							//AddNotificationWarning (msg);
//							// Use first material of the mesh, if available                           
//							if ( pNode->GetMaterialCount () > 0 ) {
//								FbxSurfaceMaterial *pMaterial =pNode->GetMaterial (0) ;
//							} else {
//								//FbxString msg = FbxString ("Could not find a material to export the texture ") + lTexIndex;
//								//msg += FbxString (" of layer ") + layerIndex;
//								//if ( pMesh->GetNode () ) {
//								//	msg += " in mesh " + pMesh->GetNode ()->GetNameWithoutNameSpacePrefix () + ".";
//								//}
//								//AddNotificationError (msg);
//								return false;
//							}
//						}
//					}
//				}
//
//				// Export the materials if needed
//				for ( int materialIndex =0 ; materialIndex < usedMaterialArray.GetCount () ; materialIndex++ ) {
//					FbxSurfaceMaterial *pMaterial =usedMaterialArray [materialIndex] ;
//					//ExportMaterial (pMaterial) ;
//					// Add the texture as input to the effect.
//					//mStatus = AddMaterialTextureInput (lXmlMaterial, lTexture, lImageId, layerIndex, lTextureTypeIndex);
//					//if ( !mStatus ) return false;
//				}
//			}
//		}
//	}
//
//	return false;
//}

}
