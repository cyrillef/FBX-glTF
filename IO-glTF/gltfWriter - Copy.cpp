
#include "stdafx.h"
#include "gltfWriter.h"

#include <stdlib.h>
#include <tchar.h>

namespace _GLTF_NAMESPACE_ {

//-----------------------------------------------------------------------------
/*static*/ gltfWriter::ExporterRoutes gltfWriter::_routes ={
	{ U("Camera"), &gltfWriter::WriteCamera },
	{ U("Light"), &gltfWriter::WriteLight },
	{ U("Mesh"), &gltfWriter::WriteMesh }
	// Nurb Patch
	// Marker // Not implemented in COLLADA / glTF.
	// CameraSwitcher // Not implemented in COLLADA / glTF.
} ;

/*static*/ gltfWriter::ExporterMerges gltfWriter::_merges = {
	{ U("Camera"), U("cameras") },
	{ U("Light"), U("lights") },
	{ U("Mesh"), U("meshes") }
} ;

//-----------------------------------------------------------------------------
gltfWriter::gltfWriter (FbxManager &pManager, int pID)
	: FbxWriter(pManager, pID, FbxStatusGlobal::GetRef ()),
	_fileName(), _triangulate(false), _writeDefaults(true)
{
	_samplingPeriod =1. / 30. ;
	//_routes.insert (std::make_pair (utility::string_t (U("Camera")), &gltfWriter::WriteCamera)) ;
	//_routes [U("Camera")] =&gltfWriter::WriteCamera ;
}

gltfWriter::~gltfWriter () {
}

bool gltfWriter::FileCreate (char *pFileName) {
	FbxString fileName =FbxPathUtils::Clean (pFileName) ;
	_fileName =utility::conversions::to_string_t (fileName.Buffer ()) ;

	//utility::string_t path =_GLTF_NAMESPACE_::GetModulePath () ;
	utility::string_t path =utility::conversions::to_string_t ((const char *)FbxGetApplicationDirectory ()) ;
	utility::ifstream_t input (path + U("glTF-0-8.json"), std::ios::in) ;
	_json =web::json::value::parse (input) ;
	input.close () ;

	if ( !FbxPathUtils::Create (FbxPathUtils::GetFolderName (fileName)) )
		return (GetStatus ().SetCode (FbxStatus::eFailure, "Cannot create folder!"), false) ;
	_gltf =utility::ofstream_t (fileName, std::ios::out) ;
	fileName =FbxPathUtils::GetFolderName (fileName) + "\\" + FbxPathUtils::GetFileName (fileName, false) + ".bin" ;
	_bin =utility::ofstream_t (fileName, std::ios::out | std::ios::binary) ;
	return (IsFileOpen ()) ;
}

bool gltfWriter::FileClose () { // todo: never called
	_json.serialize (_gltf) ;
	_gltf.close () ;
	_bin.close () ;
	return (true) ;
}

bool gltfWriter::IsFileOpen () {
	return (_gltf.is_open ()) ;
}

void gltfWriter::GetWriteOptions () {
}

bool gltfWriter::Write (FbxDocument *pDocument) {
	//_triangulate =IOS_REF.GetBoolProp (EXP_COLLADA_TRIANGULATE, true) ;
	//_singleMatrix =IOS_REF.GetBoolProp (EXP_COLLADA_SINGLEMATRIX, true) ;
	//_samplingPeriod =1. / IOS_REF.GetDoubleProp (EXP_COLLADA_FRAME_RATE, 30.0) ;

	FbxScene *pScene =FbxCast<FbxScene> (pDocument) ;
	if ( !pScene )
		return (GetStatus ().SetCode (FbxStatus::eFailure, "Document not supported!"), false) ;
	if ( !PreprocessScene (*pScene) )
		return (false) ;

	FbxDocumentInfo *pSceneInfo =pScene->GetSceneInfo () ;
	if ( !WriteAsset (pSceneInfo) )
		return (false) ;
	WriteBuffer () ;
	if ( !WriteScene (pScene) )
		return (false) ;
	//if ( !ExportAnimation (pScene->GetRootNode ()) )
	//	return (false) ;
	//if ( !ExportLibraries () )
	//	return (false) ;

	if ( !PostprocessScene (*pScene) )
		return (false) ;

	_json.serialize (_gltf) ; // todo: bug not in FileClose() ?
	return (true) ;
}

bool gltfWriter::PreprocessScene (FbxScene &pScene) {
	FbxSceneRenamer lRenamer (&pScene) ; // Rename ALL the nodes from FBX to Collada since GLTF is mainly based on Collada
	lRenamer.RenameFor (FbxSceneRenamer::eFBX_TO_DAE) ;

	FbxNode *pRootNode =pScene.GetRootNode () ;
	PreprocessNodeRecursive (pRootNode) ;

	/*if ( mSingleMatrix ) {
		lRootNode->ResetPivotSetAndConvertAnimation (1. / mSamplingPeriod.GetSecondDouble ());
	}

	// convert to the old material system for now
	FbxMaterialConverter lConv (*pScene.GetFbxManager ());
	lConv.AssignTexturesToLayerElements (pScene);

	FbxString lActiveAnimStackName = pScene.ActiveAnimStackName;
	mAnimStack = pScene.FindMember<FbxAnimStack> (lActiveAnimStackName.Buffer ());
	if ( !mAnimStack ) {
		// the application has an invalid ActiveAnimStackName, we fallback by using the 
		// first animStack.
		mAnimStack = pScene.GetMember<FbxAnimStack> ();
	}

	// If none of the above method succeed in returning an anim stack, we create 
	// a dummy one to avoid crashes. The correctness of the exported values cannot 
	// be guaranteed in this case
	if ( mAnimStack == NULL ) {
		mAnimStack = FbxAnimStack::Create (&pScene, "dummy");
		mAnimLayer = FbxAnimLayer::Create (&pScene, "dummyL");
		mAnimStack->AddMember (mAnimLayer);
	}

	mAnimLayer = mAnimStack->GetMember<FbxAnimLayer> ();

	// Make sure the scene has a name. If it does not, we try to use the filename
	// and, as last resort a dummy name.
	if ( strlen (pScene.GetName ()) == 0 ) {
		FbxDocumentInfo *lSceneInfo = pScene.GetSceneInfo ();
		FbxString lFilename ("dummy");
		if ( lSceneInfo ) {
			lFilename = lSceneInfo->Original_FileName.Get ();
			if ( lFilename.GetLen () > 0 ) {
				FbxString lFn = FbxPathUtils::GetFileName (lFilename.Buffer (), false);
				if ( lFn.GetLen () > 0 )
					lFilename = lFn;
			}
		}
		pScene.SetName (lFilename.Buffer ());
	}*/
	return (true) ;
}

void gltfWriter::PreprocessNodeRecursive (FbxNode *pNode) {
	FbxVector4 postR ;
	FbxNodeAttribute const *nodeAttribute =pNode->GetNodeAttribute () ;
	// Set PivotState to active to ensure ConvertPivotAnimationRecursive() execute correctly. 
	pNode->SetPivotState (FbxNode::eSourcePivot, FbxNode::ePivotActive) ;
	pNode->SetPivotState (FbxNode::eDestinationPivot, FbxNode::ePivotActive) ;
	// ~
	if ( nodeAttribute ) {
		// Special transformation conversion cases. If spotlight or directional light, 
		// rotate node so spotlight is directed at the X axis (was Z axis).
		if ( nodeAttribute->GetAttributeType () == FbxNodeAttribute::eLight ) {
			FbxLight *pLight =FbxCast<FbxLight>(pNode->GetNodeAttribute ()) ;
			if (   pLight->LightType.Get () == FbxLight::eSpot
				|| pLight->LightType.Get () == FbxLight::eDirectional
			) {
				postR =pNode->GetPostRotation (FbxNode::eSourcePivot) ;
				postR [0] +=90 ;
				pNode->SetPostRotation (FbxNode::eSourcePivot, postR) ;
			}
		}
		// If camera, rotate node so camera is directed at the -Z axis (was X axis).
		else if ( nodeAttribute->GetAttributeType () == FbxNodeAttribute::eCamera ) {
			postR =pNode->GetPostRotation (FbxNode::eSourcePivot) ;
			postR [1] +=90 ;
			pNode->SetPostRotation (FbxNode::eSourcePivot, postR) ;
		}
	}
	for ( int i =0 ; i < pNode->GetChildCount () ; ++i )
		PreprocessNodeRecursive (pNode->GetChild (i)) ;
}

bool gltfWriter::PostprocessScene (FbxScene &pScene) {
	/*web::json::value val =WriteAmbientLight (pScene) ;
	for ( const auto &iter : val.as_object () )
		_json [U("lights")] [iter.first] =iter.second ;
	*/
	return (true) ;
}

// Create our own writer - And your writer will get a pPluginID and pSubID. 
/*static*/ FbxWriter *gltfWriter::Create_gltfWriter (FbxManager &pManager, FbxExporter &pExporter, int pSubID, int pPluginID) {
	return (FbxNew<gltfWriter> (pManager, pPluginID)) ; // Use FbxNew instead of new, since FBX will take charge its deletion
}

// Get extension, description or version info about MyOwnWriter
/*static*/ void *gltfWriter::gltfFormatInfo (FbxWriter::EInfoRequest pRequest, int pId) {
	return (_GLTF_NAMESPACE_::_gltfFormatInfo (pRequest, pId)) ;
}

/*static*/ void gltfWriter::FillIOSettings (FbxIOSettings &pIOS) {
	// Here you can write your own FbxIOSettings and parse them.
}



//-----------------------------------------------------------------------------

// https://github.com/KhronosGroup/glTF/blob/master/specification/asset.schema.json
bool gltfWriter::WriteAsset (FbxDocumentInfo *pSceneInfo) {
	web::json::value asset =web::json::value::object () ;

	// unit - <meter> and <name>. In FBX we always work in centimeters, but we already converted to meter here
	//double scale =_scene->GetGlobalSettings ().GetSystemUnit ().GetScaleFactor () / 100. ;

	// Up axis
	//FbxScene *pScene =sceneInfo->GetScene () ;
	//FbxAxisSystem axisSystem =pScene->GetGlobalSettings ().GetAxisSystem () ;
	//int upAxisSign =0 ;
	//axisSystem.GetUpVector (upAxisSign) ;
	//if ( upAxisSign < 0 )
	//	GetStatus ().SetCode (FbxStatus::eFailure, "Invalid direction for up-axis: exporter should convert scene!") ;
	//if ( axisSystem.GetCoorSystem () != FbxAxisSystem::eRightHanded )
	//	GetStatus ().SetCode (FbxStatus::eFailure, "Axis system is Left Handed: exporter should convert scene!") ;

	// FBX uses author and comments, not authoring_tool(i.e. generator), and copyright.
	asset [U("copyright")] =web::json::value (utility::conversions::to_string_t (pSceneInfo->mAuthor.Buffer ())) ;
	asset [U("generator")] =web::json::value (FBX_GLTF_EXPORTER) ;
	if ( _writeDefaults )
		asset [U("premultipliedAlpha")] =false ; // default is false
	if ( _writeDefaults )
		asset [U("profile")] =web::json::value (WebGL_1_0_2) ; // default WebGL 1.0.2
	asset [U("version")] =web::json::value (GLTF_VERSION) ; // default 0.6
	
	_json [U("asset")] =asset ;
	return (true) ;
}

// https://github.com/KhronosGroup/glTF/blob/master/specification/buffer.schema.json
bool gltfWriter::WriteBuffer () {
	web::json::value buffer =web::json::value::object () ;

	FbxString filename =FbxPathUtils::GetFileName (utility::conversions::to_utf8string (_fileName).c_str (), false) ;
	buffer [U("uri")] =web::json::value (utility::conversions::to_string_t ((filename + ".bin").Buffer ())) ;
	if ( _writeDefaults )
		buffer [U("type")] =web::json::value (U("arraybuffer")) ; ; // default is arraybuffer
	buffer [U("byteLength")] =web::json::value ((int)0) ;

	_json [U ("buffers")] [utility::conversions::to_string_t (filename.Buffer ())] =buffer ;
	return (true) ;
}

bool gltfWriter::WriteScene (FbxScene *pScene) {
	FbxNode *pRoot =pScene->GetRootNode () ;
	utility::string_t szName =utility::conversions::to_string_t (pScene->GetName ()) ;
	_json [U("scene")] =web::json::value (szName) ;
	_json [U("scenes")] =web::json::value::object ({{ szName, web::json::value::object ({{ U("nodes"), web::json::value::array () }}) }}) ;
	for ( int i =0 ; i < pRoot->GetChildCount () ; i++ )
		WriteSceneNodeRecursive (pRoot->GetChild (i)) ;

	//UpdateMeshLibraryWithShapes (lVisualSceneElement);

	//// Add extra element containing visual_scene MAX3D & FCOLLADA extensions
	//xmlNode * lExtraElement = DAE_AddChildElement (lVisualSceneElement, COLLADA_EXTRA_STRUCTURE);
	//ExportVisualSceneMAX3DExtension (lExtraElement, pScene);
	//ExportVisualSceneFCOLLADAExtension (lExtraElement, pScene);

	//XmlNodePtr lSceneElement (DAE_NewElement (COLLADA_SCENE_STRUCTURE));
	//xmlNode * lInstanceVisualSceneElement = DAE_AddChildElement (lSceneElement, COLLADA_INSTANCE_VSCENE_ELEMENT);
	//const FbxString lUrlStr = FbxString ("#") + lSceneName;
	//DAE_AddAttribute (lInstanceVisualSceneElement, COLLADA_URL_PROPERTY, lUrlStr.Buffer ());

	//// Export ambient light
	//ExportSceneAmbient (lVisualSceneElement);

	//// add the node to the camera library. The libraries will be added to the document later.
	//if ( !mLibraryVisualScene ) {
	//	// If the visual scene library doesn't exist yet, create it.
	//	mLibraryVisualScene = DAE_NewElement (COLLADA_LIBRARY_VSCENE_ELEMENT);
	//}
	//xmlAddChild (mLibraryVisualScene, lVisualSceneElement.Release ());

	//return lSceneElement.Release ();
	return (true) ;
}

bool gltfWriter::WriteSceneNodeRecursive (FbxNode *pNode) {
	if ( !WriteSceneNode (pNode) )
		//return (GetStatus ().SetCode (FbxStatus::eFailure, "Could not export node " + pNode->GetName () + "!"), false) ;
		return (false) ;
	for ( int i =0; i < pNode->GetChildCount () ; i++ )
		WriteSceneNodeRecursive (pNode->GetChild (i)) ;
	return (true) ;
}

bool gltfWriter::WriteSceneNode (FbxNode *pNode) ()



//-----------------------------------------------------------------------------
utility::string_t gltfWriter::uniqueId (utility::string_t &type, FbxUInt64 id) {
	TCHAR buffer [255] ;
	_ui64tot_s (id, buffer, 255, 10) ;
	utility::string_t uid (type) ;
	uid +=U("_") + utility::conversions::to_string_t (buffer) ;
	return (uid) ;
}

utility::string_t gltfWriter::uniqueId (FbxNode *pNode) {
	return (uniqueId (utility::conversions::to_string_t (pNode->GetTypeName ()), pNode->GetUniqueID ())) ;
	//FbxString nameWithoutSpacePrefix =pNode->GetNameWithoutNameSpacePrefix () ;
	//FbxString szID =nameWithoutSpacePrefix ;
	//FbxProperty id =pNode->FindProperty ("COLLADA_ID") ;
	//if ( id.IsValid () )
	//	szID =id.Get<FbxString> () ;
	//return (utility::conversions::to_string_t (szID.Buffer ())) ;
}

// https://github.com/KhronosGroup/glTF/blob/master/specification/node.schema.json
web::json::value gltfWriter::WriteNode (FbxNode *pNode) {
	web::json::value nodeDef =web::json::value::object () ;

	//utility::string_t id =utility::conversions::to_string_t (pNode->GetNameWithoutNameSpacePrefix ().Buffer ()) ;
	//FbxProperty pid =pNode->FindProperty ("COLLADA_ID") ;
	//if ( pid.IsValid () )
	//	id =utility::conversions::to_string_t (pid.Get<FbxString> ().Buffer ()) ;
	utility::string_t id =utility::conversions::to_string_t (pNode->GetName ()) ;

	nodeDef [U("name")] =web::json::value (id) ; // https://github.com/KhronosGroup/glTF/blob/master/specification/glTFChildOfRootProperty.schema.json

	utility::string_t szType =utility::conversions::to_string_t (pNode->GetTypeName ()) ;
	std::transform (szType.begin (), szType.end(), szType.begin(), ::tolower) ;
	//if ( szType == U("camera") || szType == U("light") )
	if ( pNode->GetNodeAttribute () && (pNode->GetNodeAttribute ()->GetAttributeType () == FbxNodeAttribute::eCamera || pNode->GetNodeAttribute ()->GetAttributeType () == FbxNodeAttribute::eLight) )
		nodeDef [szType] =web::json::value (uniqueId (pNode)) ; //nodeDef [U("camera")] nodeDef [U("light")]
	// A floating-point 4x4 transformation matrix stored in column-major order.
	// A node will have either a matrix property defined or any combination of rotation, scale, and translation properties defined.
	web::json::value nodeTransform =GetTransform (pNode) ;
	//if ( !nodeTransform.is_boolean () || nodeTransform != true )
		nodeDef [U("matrix")] =nodeTransform ;
	//nodeDef [U("rotation")] =web::json::value::array ({{ 1., 0., 0., 0. }}) ;
	//nodeDef [U("scale")] =web::json::value::array ({{ 1., 1., 1. }}) ;
	//nodeDef [U("translation")] =web::json::value::array ({{ 0., 0., 0. }}) ;

	nodeDef [U("children")] =web::json::value::array () ;
	//nodeDef [U("instanceSkin")] = ;

	const FbxNodeAttribute *nodeAttribute =pNode->GetNodeAttribute () ;
	if ( nodeAttribute && nodeAttribute->GetAttributeType () == FbxNodeAttribute::eSkeleton ) {
		// The only difference between a node containing a NULL and one containing a SKELETON is the property type JOINT.
		nodeDef [U("jointName")] =web::json::value (U("JOINT")) ;
	}
	
	//if ( szType == U("mesh") )
	if ( pNode->GetNodeAttribute () && pNode->GetNodeAttribute ()->GetAttributeType () == FbxNodeAttribute::eMesh )
		nodeDef [U("meshes")] =web::json::value::array ({{ web::json::value (uniqueId (pNode)) }}) ;

	return (web::json::value::object ({{ id, nodeDef }})) ;
}

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
	cameraDef [U("yfov")] =web::json::value (DEG2RAD(focalAngle)) ;
}

// https://github.com/KhronosGroup/glTF/blob/master/specification/camera.schema.json
web::json::value gltfWriter::WriteCamera (FbxNode *pNode) {
	web::json::value camera =web::json::value::object () ;
	web::json::value cameraDef =web::json::value::object () ;
	camera [U ("name")] =web::json::value (uniqueId (pNode)) ; // https://github.com/KhronosGroup/glTF/blob/master/specification/glTFChildOfRootProperty.schema.json
	FbxCamera *pCamera =pNode->GetCamera () ; //FbxCast<FbxCamera> (pNode->GetNodeAttribute ()) ;
	//FbxCamera::EAspectRatioMode aspectRatioMode =pCamera->GetAspectRatioMode () ;
	switch ( pCamera->ProjectionType ) {
		case FbxCamera::EProjectionType::ePerspective: // https://github.com/KhronosGroup/glTF/blob/master/specification/cameraPerspective.schema.json
			camera [U("type")] =web::json::value (U("perspective")) ;
			cameraDef [U("aspectRatio")] =web::json::value (pCamera->FilmAspectRatio) ; // (pCamera->AspectWidth / pCamera->AspectHeight) ;
			//cameraDef [U("yfov")] =web::json::value (DEG2RAD(pCamera->FieldOfView)) ;
			cameraFOV (pCamera, cameraDef) ;
			cameraDef [U("zfar")] =web::json::value (pCamera->FarPlane) ;
			cameraDef [U("znear")] =web::json::value (pCamera->NearPlane) ;
			camera [U("perspective")] =cameraDef ;
			break ;
		case FbxCamera::EProjectionType::eOrthogonal: // https://github.com/KhronosGroup/glTF/blob/master/specification/cameraOrthographic.schema.json
			camera [U("type")] =web::json::value (U("orthographic")) ;
			//cameraDef [U("xmag")] =web::json::value (pCamera->_2DMagnifierX) ;
			//cameraDef [U("ymag")] =web::json::value (pCamera->_2DMagnifierY) ;
			cameraDef [U("xmag")] =web::json::value (pCamera->OrthoZoom) ;
			cameraDef [U("ymag")] =web::json::value (pCamera->OrthoZoom) ; // FBX Collada reader set OrthoZoom using xmag and ymag each time they appear
			cameraDef [U("zfar")] =web::json::value (pCamera->FarPlane) ;
			cameraDef [U("znear")] =web::json::value (pCamera->NearPlane) ;
			camera [U("orthographic")] =cameraDef ;
			break ;
		default:
			_ASSERTE( false ) ;
			break ;
	}
	web::json::value lib =web::json::value::object ({{ uniqueId (pNode), camera }}) ;
	web::json::value node =WriteNode (pNode) ;

	return (web::json::value::object ({{ U("cameras"), lib }, { U("nodes"), node }})) ;
	//return (web::json::value::object ({{ uniqueId (pNode), camera }})) ;
}

web::json::value gltfWriter::GetTransform (FbxNode *pNode) {
	// Export the node's default transforms

	// If the mesh is a skin binded to a skeleton, the bind pose will include its transformations.
	// In that case, do not export the transforms twice.
	//if (   pNode->GetNodeAttribute ()
	//	&& pNode->GetNodeAttribute ()->GetAttributeType () == FbxNodeAttribute::eMesh
	//) {
	//	int deformerCount =FbxCast<FbxMesh>(pNode->GetNodeAttribute ())->GetDeformerCount (FbxDeformer::eSkin) ;
	//	_ASSERTE( deformerCount <= 1 ) ; // "Unexpected number of skin greater than 1") ;
	//	int clusterCount =0 ;
	//	// It is expected for deformerCount to be equal to 1
	//	for ( int i =0 ; i < deformerCount ; ++i )
	//		clusterCount +=FbxCast<FbxSkin>(FbxCast<FbxMesh>(pNode->GetNodeAttribute ())->GetDeformer (i, FbxDeformer::eSkin))->GetClusterCount () ;
	//	if ( clusterCount )
	//		return (true) ;
	//}

	FbxAMatrix identity ;
	identity.SetIdentity () ;
	FbxAMatrix thisLocal ;
	// For Single Matrix situation, obtain transform matrix from eDestinationPivot, which include pivot offsets and pre/post rotations.
	FbxAMatrix &thisGlobal =const_cast<FbxNode *>(pNode)->EvaluateGlobalTransform (FBXSDK_TIME_ZERO, FbxNode::eDestinationPivot) ;
	const FbxNode *pParentNode =pNode->GetParent () ;
	if ( pParentNode ) {
		// For Single Matrix situation, obtain transform matrix from eDestinationPivot, which include pivot offsets and pre/post rotations.
		FbxAMatrix &parentGlobal =const_cast<FbxNode *>(pParentNode)->EvaluateGlobalTransform (FBXSDK_TIME_ZERO, FbxNode::eDestinationPivot) ;
		FbxAMatrix parentInverted =parentGlobal.Inverse () ;
		thisLocal =parentInverted * thisGlobal ;
	} else {
		thisLocal =thisGlobal ;
	}

	FbxAMatrix::kDouble44 &r =thisLocal.Double44 () ;
	std::vector<web::json::value> ar ;
	for ( int i =0 ; i < 4 ; i++ )
		for ( int j =0 ; j < 4 ; j++ )
			ar.push_back (web::json::value (r [i] [j])) ;
	return (web::json::value::array (ar)) ;
}

void gltfWriter::lightAttenuation (FbxLight *pLight, web::json::value &lightDef) {
	float attenuation [3] ={
		1.0f, // Constant
		0.0f, // Linear
		0.0f, // Quadratic
	} ;
	switch ( pLight->DecayType ) {
		case FbxLight::EDecayType::eLinear:
			attenuation [0] =0.0f ;
			attenuation [1] =1.0f ;
			break ;
		case FbxLight::EDecayType::eCubic:
			attenuation [0] =0.0f ;
			attenuation [2] =1.0f ; // OpenGL doesn't support cubic, so default to quadratic
			break ;
		case FbxLight::EDecayType::eQuadratic:
			attenuation [0] =0.0f ;
			attenuation [2] =1.0f ;
			break ;
		case FbxLight::EDecayType::eNone:
		default:
			attenuation[0] =1.0f ;
			break ;
	}
	if ( attenuation [0] != 1.0f )
		lightDef [U("constantAttenuation")] =web::json::value (attenuation [0]) ;
	if ( attenuation [1] != 0.0f )
		lightDef [U("linearAttenuation")] =web::json::value (attenuation [1]) ;
	if ( attenuation [2] != 0.0f )
		lightDef [U("quadraticAttenuation")] =web::json::value (attenuation [2]) ;
}

// https://github.com/KhronosGroup/glTF/blob/master/specification/light.schema.json
web::json::value gltfWriter::WriteLight (FbxNode *pNode) {
	web::json::value light =web::json::value::object () ;
	web::json::value lightDef =web::json::value::object () ;
	light [U ("name")] =web::json::value (uniqueId (pNode)) ; // https://github.com/KhronosGroup/glTF/blob/master/specification/glTFChildOfRootProperty.schema.json
	FbxLight *pLight =pNode->GetLight () ; //FbxCast<FbxLight> (pNode->GetNodeAttribute ()) ;
	static const FbxDouble3 defaultLightColor (1.0, 1.0, 1.0) ;
	auto color =pLight->Color ;
	if ( _writeDefaults || color.Get () != defaultLightColor ) {
		lightDef [U("color")] =web::json::value::array () ;
		lightDef [U("color")] [0] =static_cast<float> (color.Get () [0]) ;
		lightDef [U("color")] [1] =static_cast<float> (color.Get () [1]) ;
		lightDef [U("color")] [2] =static_cast<float> (color.Get () [2]) ;
	}
	switch ( pLight->LightType ) {
		case FbxLight::EType::ePoint: // https://github.com/KhronosGroup/glTF/blob/master/specification/lightPoint.schema.json
			light [U("type")] =web::json::value (U("point")) ;
			lightAttenuation (pLight, lightDef) ;
			light [U("point")] =lightDef ;
			break ;
		case FbxLight::EType::eSpot: // https://github.com/KhronosGroup/glTF/blob/master/specification/lightSpot.schema.json
			light [U("type")] =web::json::value (U("spot")) ;
			lightAttenuation (pLight, lightDef) ;
			if ( pLight->OuterAngle.Get () != 180.0 ) // default is PI
				lightDef [U("fallOffAngle")] =DEG2RAD(pLight->OuterAngle) ;
			if ( _writeDefaults )
				lightDef [U("fallOffExponent")] =web::json::value ((double)0.) ;
			light [U("spot")] =lightDef ;
			break ;
		case FbxLight::EType::eDirectional: // https://github.com/KhronosGroup/glTF/blob/master/specification/lightDirectional.schema.json
			light [U("type")] =web::json::value (U("directional")) ;
			light [U("directional")] =lightDef ;
			break ;
		case FbxLight::EType::eArea:
		case FbxLight::EType::eVolume:
		default: // ambient - https://github.com/KhronosGroup/glTF/blob/master/specification/lightAmbient.schema.json
			_ASSERTE( false ) ;
			return (web::json::value::object ({{}})) ;
			break ;
	}
	
	web::json::value lib =web::json::value::object ({ { uniqueId (pNode), light } }) ;
	web::json::value node =WriteNode (pNode) ;

	return (web::json::value::object ({ { U ("lights"), lib }, { U ("nodes"), node } })) ;
	//return (web::json::value::object ({{ uniqueId (pNode), light }})) ;
}

web::json::value gltfWriter::WriteAmbientLight (FbxScene &pScene) {
	web::json::value light =web::json::value::object () ;
	web::json::value lightDef =web::json::value::object () ;
	static const FbxDouble3 defaultLightColor (1.0, 1.0, 1.0) ;
	FbxColor color (pScene.GetGlobalSettings ().GetAmbientColor ()) ;
	if ( !color.mRed && !color.mGreen && !color.mBlue )
		return (web::json::value::object ({{}})) ;
	if ( !_writeDefaults && color == defaultLightColor )
		return (web::json::value::object ({{}})) ;
	lightDef [U("color")] =web::json::value::array () ;
	lightDef [U("color")] [0] =static_cast<float> (color.mRed) ;
	lightDef [U("color")] [1] =static_cast<float> (color.mGreen) ;
	lightDef [U("color")] [2] =static_cast<float> (color.mBlue) ;
	light [U("type")] =web::json::value (U("ambient")) ;
	light [U("ambient")] =lightDef ;
	return (web::json::value::object ({{ uniqueId (utility::string_t (U("defaultambient")), 0x00), light }})) ;

}

web::json::value gltfWriter::WriteVertexPositions (FbxMesh *pMesh, bool pInGeometry, bool pExportControlPoints) {
	// In an ordinary geometry, export the control points.
	// In a binded geometry, export transformed control points...
	// In a controller, export the control points.
	pExportControlPoints =true ;

	FbxString szName =pMesh->GetName () ;
	szName +=(pInGeometry ? "-Position"/* Normal geometry case */ : "-BindPos"/* inside a controller */) ;

	FbxArray<FbxVector4> controlPoints ;
	// Get Control points.
	// Translate a FbxVector4* into FbxArray<FbxVector4>
	FbxVector4 *pTemp =pMesh->GetControlPoints () ;
	int nbControlPoints =pMesh->GetControlPointsCount () ;
	for ( int i =0 ; i < nbControlPoints ; i++ )
		controlPoints.Add (pTemp [i]) ;

	FbxArray<FbxVector4> positions ;
	if ( pExportControlPoints ) {
		positions =controlPoints ;
		if ( !pInGeometry ) {
			FbxAMatrix transform =pMesh->GetNode ()->EvaluateGlobalTransform () ;
			for ( int i =0; i < nbControlPoints ; i++ )
				positions [i] =transform.MultT (positions [i]) ;
		}
	} else {
		// Initialize positions
		positions.Resize (nbControlPoints) ;
		// Get the transformed control points.
		int deformerCount =pMesh->GetDeformerCount (FbxDeformer::eSkin) ;
		_ASSERTE( deformerCount <= 1 ) ; // Unexpected number of skin greater than 1
		// It is expected for deformerCount to be equal to 1
		for ( int i =0 ; i < deformerCount ; i++ ) {
			int clusterCount =FbxCast<FbxSkin>(pMesh->GetDeformer (i, FbxDeformer::eSkin))->GetClusterCount () ;
			for ( int indexLink =0 ; indexLink < clusterCount ; indexLink++ ) {
				FbxCluster *pLink =FbxCast<FbxSkin>(pMesh->GetDeformer (i, FbxDeformer::eSkin))->GetCluster (indexLink) ;
				FbxAMatrix jointPosition =pLink->GetLink ()->EvaluateGlobalTransform () ;
				FbxAMatrix transformLink ;
				pLink->GetTransformLinkMatrix (transformLink) ;
				FbxAMatrix m =transformLink.Inverse () * jointPosition ;
				for ( int j =0; j < pLink->GetControlPointIndicesCount () ; j++ ) {
					int index =pLink->GetControlPointIndices () [j] ;
					FbxVector4 controlPoint =controlPoints [index] ;
					double weight =pLink->GetControlPointWeights () [j] ;
					FbxVector4 pos =m.MultT (controlPoint) ;
					pos =pos * weight ;
					positions [index] =positions [index] + pos ;
				}
			}
		}
	}

	//positions
	return true;
}

void gltfWriter::CopyMesh (FbxMesh *pNewMesh, FbxMesh *pRefMesh) {
	// Copy mesh parameters from pRefMesh to pNewMesh.
	// If pOnlyLayer0 is true, only layer 0 is copied. Else, all layers are copied.

	// Vertices
	int controlPointsCount =pRefMesh->GetControlPointsCount () ;
	FbxVector4 *refControlPoints =pRefMesh->GetControlPoints () ;
	FbxVector4 *refNormals =NULL ;
	FbxLayerElementArrayTemplate<FbxVector4> *direct ;
	pRefMesh->GetNormals (&direct) ;
	if ( direct )
		refNormals =direct->GetLocked (refNormals, FbxLayerElementArray::eReadLock) ;
	pNewMesh->InitControlPoints (controlPointsCount) ;
	pNewMesh->InitNormals (controlPointsCount) ;
	for ( int i =0 ; i < controlPointsCount ; i++ ) {
		FbxVector4 controlPoint =refControlPoints [i] ;
		FbxVector4 normal =refNormals [i] ;
		pNewMesh->SetControlPointAt (controlPoint, normal, i) ;
	}
	if ( direct )
		direct->Release (&refNormals, refNormals) ;
	// Polygons
	int polygonCount =pRefMesh->GetPolygonCount () ;
	int *refPolygonVertices =pRefMesh->GetPolygonVertices () ;
	int materialIndex =-1 ;
	int textureIndex =-1 ;
	int index =0 ;
	for ( int polygonIndex =0 ; polygonIndex < polygonCount ; polygonIndex++ ) {
		int polygonSize =pRefMesh->GetPolygonSize (polygonIndex) ;
		pNewMesh->BeginPolygon (materialIndex, textureIndex) ;
		for ( int vertexIndex =0 ; vertexIndex < polygonSize ; vertexIndex++ )
			pNewMesh->AddPolygon (refPolygonVertices [index++]) ;
		pNewMesh->EndPolygon () ;
	}
	// Layer elements: Normals, UVs, Vertex colors, Materials, Textures and Polygon groups (the latter is not supported by glTF/COLLADA).
	for ( int layerIndex =0 ; layerIndex < pRefMesh->GetLayerCount () ; layerIndex++ ) {
		FbxLayer *refLayer =pRefMesh->GetLayer (layerIndex) ;
		FbxLayer *newLayer =pNewMesh->GetLayer (layerIndex) ;
		while ( !newLayer ) {
			pNewMesh->CreateLayer () ;
			newLayer =pNewMesh->GetLayer (layerIndex) ;
		}
		newLayer->Clone (*refLayer) ;
	}
}

bool gltfWriter::WriteControllerShape (FbxMesh *pMesh) {
	// Export controller in case of a morph controller (shape deformer).
	//FbxString szName =pMesh->GetNode ()->GetNameWithoutNameSpacePrefix () ;
	//FbxString id =szName + "-lib-morph" ;
	//FbxString targetUrl =FbxString ("#") + lName + "-lib" ;
	//FbxString lShapesId =id + "-targets" ; // yes, this always mixes me up too
	//FbxString lWeightsId =id + "-weights" ;

	// Targets (shapes) sources and
	// Shape weights.
	FbxStringList szShapesNames ;
	FbxArray<double> shapesWeights ;

	// glTF/Collada does not support In-Between blend shape by default;
	// So for each blend shape channel, only take account of the first target shape.
	int blendShapeDeformerCount =pMesh->GetDeformerCount (FbxDeformer::eBlendShape) ;
	for ( int blendShapeIndex =0 ; blendShapeIndex < blendShapeDeformerCount ; blendShapeIndex++ ) {
		FbxBlendShape *pBlendShape =FbxCast<FbxBlendShape>(pMesh->GetDeformer (blendShapeIndex, FbxDeformer::eBlendShape)) ;
		int blendShapeChannelCount =pBlendShape->GetBlendShapeChannelCount () ;
		for ( int channelIndex =0 ; channelIndex < blendShapeChannelCount ; channelIndex++ ) {
			FbxBlendShapeChannel *pChannel =pBlendShape->GetBlendShapeChannel (channelIndex) ;
			FbxShape *pShape =pChannel->GetTargetShape (0) ;
			FbxString szShapeName =pShape->GetName () ;
			szShapeName +="-lib" ;
			szShapesNames.Add (szShapeName.Buffer ()) ;

			double lWeight =pChannel->DeformPercent.Get () ;
			shapesWeights.Add (lWeight / 100) ;

		}
	}

	//xmlNode *lXmlShapesSource = DAE_ExportSource14 (lMorph, lShapesId.Buffer (), lShapesNames, COLLADA_NAME_TYPE, true);
	//FbxStringList lAccessorParams;
	//lAccessorParams.Add ("WEIGHT", 0);
	//xmlNode *lXmlShapesWeights = DAE_ExportSource14 (lMorph, lWeightsId.Buffer (), lAccessorParams, lShapesWeights);

	//// Targets
	//xmlNode* lXmlTargets = xmlNewChild (lMorph, NULL, XML_STR COLLADA_TARGETS_ELEMENT, NULL);
	//DAE_AddInput14 (lXmlTargets, COLLADA_MORPH_TARGET_SEMANTIC, lShapesId.Buffer ());
	//DAE_AddInput14 (lXmlTargets, COLLADA_MORPH_WEIGHT_SEMANTIC, lWeightsId.Buffer ());

	// Update internal info for later export of shape geometries, required for COLLADA.
	blendShapeDeformerCount =pMesh->GetDeformerCount (FbxDeformer::eBlendShape) ;
	for ( int blendShapeIndex =0 ; blendShapeIndex < blendShapeDeformerCount ; blendShapeIndex++ ) {
		FbxBlendShape *pBlendShape =FbxCast<FbxBlendShape>(pMesh->GetDeformer (blendShapeIndex, FbxDeformer::eBlendShape)) ;
		int blendShapeChannelCount =pBlendShape->GetBlendShapeChannelCount () ;
		for ( int channelIndex =0 ; channelIndex < blendShapeChannelCount ; channelIndex++ ) {
			FbxBlendShapeChannel *pChannel =pBlendShape->GetBlendShapeChannel (channelIndex) ;
			FbxShape *pShape =pChannel->GetTargetShape (0) ;
			// Create a mesh deformed by the shape.
			FbxMesh *pMeshShape =FbxMesh::Create (fbxSdkMgr::Instance ()->fbxMgr ().Get (), "") ; 
			CopyMesh (pMeshShape, pMesh) ;
			// Deform the mesh vertices with the shape.
			int nbVertex =pMeshShape->GetControlPointsCount () ;
			FbxVector4 *pShapeControlPoints =pShape->GetControlPoints () ;
			FbxVector4 *pShapeNormals =NULL ;
			FbxLayerElementArrayTemplate<FbxVector4> *direct ;
			pShape->GetNormals (&direct) ;
			if ( direct )
				pShapeNormals =direct->GetLocked (pShapeNormals, FbxLayerElementArray::eReadLock) ;
			for ( int vertexIndex =0 ; vertexIndex < nbVertex; vertexIndex++ ) {
				FbxVector4 newControlPoint =pShapeControlPoints [vertexIndex] ;
				if ( pShapeNormals ) {
					FbxVector4 newNormal =pShapeNormals [vertexIndex] ;
					pMeshShape->SetControlPointAt (newControlPoint, newNormal, vertexIndex) ;
				} else {
					pMeshShape->SetControlPointAt (newControlPoint, vertexIndex) ;
				}
			}
			if ( direct )
				direct->Release (&pShapeNormals, pShapeNormals) ;
			FbxString targetId =FbxString (pShape->GetName ()) + "-lib" ;
			//shapeMeshesList->Add (targetId.Buffer (), (FbxHandle)pMeshShape) ;
		}
	}

	return true;
}

// https://github.com/KhronosGroup/glTF/blob/master/specification/mesh.schema.json
// https://github.com/KhronosGroup/glTF/blob/master/specification/meshPrimitive.schema.json
// https://github.com/KhronosGroup/glTF/blob/master/specification/meshPrimitiveAttribute.schema.json
web::json::value gltfWriter::WriteMesh (FbxNode *pNode) {
	web::json::value meshDef =web::json::value::object () ;
	web::json::value meshPrimitives =web::json::value::array () ;
	meshDef [U ("name")] =web::json::value (uniqueId (pNode)) ; // https://github.com/KhronosGroup/glTF/blob/master/specification/glTFChildOfRootProperty.schema.json
	FbxMesh *pMesh =pNode->GetMesh () ; //FbxCast<FbxMesh>(pNode->GetNodeAttribute ()) ;

	// If the mesh is a skin binded to a skeleton, the bind pose will include its transformations.
	// In that case, do not export the transforms twice.
	int deformerCount =pMesh->GetDeformerCount (FbxDeformer::eSkin) ;
	_ASSERTE( deformerCount <= 1 ) ; // "Unexpected number of skin greater than 1") ;
	int clusterCount =0 ;
	// It is expected for deformerCount to be equal to 1
	for ( int i =0 ; i < deformerCount ; ++i )
		clusterCount +=FbxCast<FbxSkin> (pMesh->GetDeformer (i, FbxDeformer::eSkin))->GetClusterCount () ;
	
	//FbxString nameWithoutNameSpacePrefix =pNode->GetNameWithoutNameSpacePrefix () ;
	web::json::value vertex =WriteVertexPositions (pMesh, true, (clusterCount == 0)) ; // Vertex Positions
	//WriteVertices (pMesh) ; // Vertices

	const FbxNodeAttribute *nodeAttribute =NULL ;
	if ( _triangulate ) {
		FbxGeometryConverter converter (&mManager) ;
		pMesh =FbxCast<FbxMesh>(converter.Triangulate (FbxCast<FbxNodeAttribute>(pNode->GetNodeAttribute ()), true)) ;
	}
	//pMesh->get


	web::json::value primitive =web::json::value::object () ;
	primitive [U("attributes")] =web::json::value::object () ;
	primitive [U ("attributes")] [U ("POSITION")] =web::json::value (uniqueId (pNode) + U("-position")) ;
	//meshPrimitives [U("attributes")] [U("NORMAL")] =web::json::value (uniqueId (pNode) + U("-normal")) ; ;
	//meshPrimitives [U("attributes")] [U("TEXCOORD_0")] =web::json::value (uniqueId (pNode) + U("-uv")) ; ;
	
	//meshPrimitives [U("indices")] =web::json::value (uniqueId (pNode) + U("-face")) ;
	//meshPrimitives [U("material")] = ;
	//meshPrimitives [U("primitive")] = ; // Allowed values are 0 (POINTS), 1 (LINES), 2 (LINE_LOOP), 3 (LINE_STRIP), 4 (TRIANGLES), 5 (TRIANGLE_STRIP), and 6 (TRIANGLE_FAN).

	meshPrimitives [meshPrimitives.size ()] =primitive ;

	meshDef [U("primitives")] =meshPrimitives ;
	//mesh [U("spot")] =meshDef ;


	web::json::value lib =web::json::value::object ({ { uniqueId (pNode), meshDef } }) ;
	web::json::value node =WriteNode (pNode) ;
	//if ( pMesh->GetShapeCount () )
	//	WriteControllerShape (pMesh) ; // Create a controller

	return (web::json::value::object ({ { U ("meshes"), lib }, /*{ U ("accessors"), acc },*/ { U ("nodes"), node } })) ;

}

}
