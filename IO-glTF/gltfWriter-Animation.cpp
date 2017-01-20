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

int GetNumberOfAnimationFrames(FbxScene* pScene)
{
    FbxTimeSpan interval;
    FbxNode* pRootNode = pScene->GetRootNode();

	if (pRootNode->GetAnimationInterval(interval))
	{
		FbxTime start = interval.GetStart();
		FbxTime end = interval.GetStop();

		FbxLongLong longstart = start.GetFrameCount();
		FbxLongLong longend = end.GetFrameCount();

		return int(longend - longstart);
	}

	return 0;
}

web::json::value gltfWriter::WriteSkeleton(FbxNode *pNode) {

    FbxSkeleton* lSkeleton = (FbxSkeleton*) pNode->GetNodeAttribute();

    if (lSkeleton->GetSkeletonType() == FbxSkeleton::eLimbNode &&
        pNode->GetParent() &&
        pNode->GetParent()->GetNodeAttribute() &&
        pNode->GetParent()->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton)
    {
	web::json::value node = WriteNode (pNode) ;
        web::json::value ret  = web::json::value::object ({ { U("nodes"), node } }) ;
	//WriteAnimation(pNode->GetScene(), pNode);
	return ret;	
    }
return (web::json::value::null());
}

void gltfWriter::WriteAnimationLayer(FbxAnimLayer* pAnimLayer, FbxNode* pNode, bool isSwitcher)
{
    int lModelCount;

    WriteAnimationChannels(pNode, pAnimLayer);

    for(lModelCount = 0; lModelCount < pNode->GetChildCount(); lModelCount++)
    {
        WriteAnimationLayer(pAnimLayer, pNode->GetChild(lModelCount), false);
    }
}

web::json::value gltfWriter::WriteCurveChannels(utility::string_t aName, utility::string_t trs, FbxNode *pNode, int animAccessorCount) {

    utility::string_t animAccName;
    
    web::json::value samplers   = web::json::value::object () ;
    web::json::value animationChannel = web::json::value () ;
    web::json::value sampler   = web::json::value::object ({ { U("sampler"), web::json::value::object () } }) ;

    utility::string_t samplerName = aName ;
    samplerName += utility::conversions::to_string_t (U("_")) ;
    samplerName += trs;
    samplerName += utility::conversions::to_string_t (U("_")) ;
    samplerName +=utility::conversions::to_string_t (U("sampler")) ;
    animationChannel[U("sampler")] = web::json::value::string (samplerName); 
    animationChannel[U("target")] = web::json::value::object({
			{ U("id"), web::json::value::string (utility::conversions::to_string_t (nodeId(pNode))) }, 
			{ U("path"), web::json::value::string (utility::conversions::to_string_t(trs)) } } 
			) ;
    web::json::value samplerDef =web::json::value::object () ;
    samplerDef [U("input")] = web::json::value::string (U("TIME"));
    // glTF 1.0 animation samplers support only linear interpolation.
    samplerDef [U("interpolation")] = web::json::value::string (U("LINEAR")); 
    samplerDef [U("output")] = web::json::value::string (utility::conversions::to_string_t(trs));
    _json [U("animations")][aName][U("samplers")] [samplerName]= samplerDef;


return animationChannel;
}

web::json::value gltfWriter::WriteAnimParameters(FbxNode *pNode, std::vector<float> KeyValues, int animAccessorCount, utility::string_t trs) {

   utility::string_t animAccName;
   web::json::value ret;


   web::json::value accessorsAndBufferViews =web::json::value::object ({
                { U("accessors"), web::json::value::object () },
                { U("bufferViews"), web::json::value::object () }
        }) ;

    animAccName = createUniqueName (U("_animAccessor"), animAccessorCount);
    web::json::value animAccessor =WriteArray <float>(KeyValues,1, pNode, animAccName.c_str()) ;
    ret = web::json::value::string (GetJsonFirstKey (animAccessor [U("accessors")])) ;
    MergeJsonObjects (accessorsAndBufferViews, animAccessor) ;
    MergeJsonObjects (_json, accessorsAndBufferViews) ;

   return ret;
}
template <class T>
web::json::value gltfWriter::WriteAnimParameters(FbxNode *pNode, std::vector<T> KeyValues, int animAccessorCount, utility::string_t trs) {

   utility::string_t animAccName;
   web::json::value ret;


   web::json::value accessorsAndBufferViews =web::json::value::object ({
                { U("accessors"), web::json::value::object () },
                { U("bufferViews"), web::json::value::object () }
        }) ;

    animAccName = createUniqueName (U("_animAccessor"), animAccessorCount);
    web::json::value animAccessor =WriteArray <T,float>(KeyValues,pNode, animAccName.c_str()) ;
    ret = web::json::value::string (GetJsonFirstKey (animAccessor [U("accessors")])) ;
    MergeJsonObjects (accessorsAndBufferViews, animAccessor) ;
    MergeJsonObjects (_json, accessorsAndBufferViews) ;

   return ret;
}

void getTRS(FbxAnimEvaluator *animEvaluator, FbxAnimCurve* animCurve, FbxNode* pNode, std::vector<float> &trsKeyTime, std::vector<FbxDouble3> &transAtTime, std::vector<FbxDouble4> &rotAtTime, std::vector<FbxDouble3> &scaleAtTime)
{
	if(animCurve) {
            int keyCount = animCurve->KeyGetCount();
            for (int index=0; index < keyCount; index++)
            {
            	    FbxTime time;
                    time = animCurve->KeyGetTime(index);
		    FbxDouble3 trans = animEvaluator->GetNodeLocalTranslation(pNode, time);
                    FbxDouble4 rot   = animEvaluator->GetNodeLocalRotation(pNode, time);
                    FbxDouble3 scale = animEvaluator->GetNodeLocalScaling(pNode, time);
                    trsKeyTime.push_back(time.GetSecondDouble());
			rotAtTime.push_back(rot);
			transAtTime.push_back(trans);
			scaleAtTime.push_back(scale);
		    

            }
    }

}

void gltfWriter::WriteAnimationChannels(FbxNode* pNode, FbxAnimLayer* pAnimLayer)
{

    FbxAnimCurve* lAnimCurve = NULL;
    int     lCount;
    static int animCount = -1;
    static int animAccessorCount = -1;
    web::json::value tCurveChannels   = web::json::value () ;
    web::json::value rCurveChannels   = web::json::value () ;
    web::json::value sCurveChannels   = web::json::value () ;
    web::json::value channels         = web::json::value::array ();
    web::json::value animations = web::json::value::object ({ { U("animations"), web::json::value::object () } }) ;
    web::json::value parameters = web::json::value::object ({
		{ U("TIME"), web::json::value::object () },
                { U("translation"), web::json::value::object () },
		{ U("rotation"), web::json::value::object () },
                { U("scale"), web::json::value::object () }	
    }) ;
   web::json::value accessorsAndBufferViews =web::json::value::object ({
                { U("accessors"), web::json::value::object () },
                { U("bufferViews"), web::json::value::object () }
   }) ;
   

    utility::string_t aName = createUniqueName (U("animation"), ++animCount);

    std::vector<FbxDouble3> transAtTime, scaleAtTime;
    std::vector<FbxDouble4> rotAtTime;
    std::vector<float> trsKeyTime;

    FbxAnimEvaluator *animEvaluator = pNode->GetAnimationEvaluator();

    FbxAnimCurve* txAnimCurve = pNode->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X);
	    getTRS(animEvaluator, txAnimCurve, pNode, trsKeyTime, transAtTime, rotAtTime, scaleAtTime);
    FbxAnimCurve* tyAnimCurve = pNode->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y);
	    getTRS(animEvaluator, tyAnimCurve, pNode, trsKeyTime, transAtTime, rotAtTime, scaleAtTime);
    FbxAnimCurve* tzAnimCurve = pNode->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);
	    getTRS(animEvaluator, tzAnimCurve, pNode, trsKeyTime, transAtTime, rotAtTime, scaleAtTime);
    FbxAnimCurve* rxAnimCurve = pNode->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X);
	    getTRS(animEvaluator, rxAnimCurve, pNode, trsKeyTime, transAtTime, rotAtTime, scaleAtTime);
    FbxAnimCurve* ryAnimCurve = pNode->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y);
	    getTRS(animEvaluator, ryAnimCurve, pNode, trsKeyTime, transAtTime, rotAtTime, scaleAtTime);
    FbxAnimCurve* rzAnimCurve = pNode->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);
	    getTRS(animEvaluator, rzAnimCurve, pNode, trsKeyTime, transAtTime, rotAtTime, scaleAtTime);
    FbxAnimCurve* sxAnimCurve = pNode->LclScaling.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X);
	    getTRS(animEvaluator, sxAnimCurve, pNode, trsKeyTime, transAtTime, rotAtTime, scaleAtTime);
    FbxAnimCurve* syAnimCurve = pNode->LclScaling.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y);
	    getTRS(animEvaluator, syAnimCurve, pNode, trsKeyTime, transAtTime, rotAtTime, scaleAtTime);
    FbxAnimCurve* szAnimCurve = pNode->LclScaling.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);
	    getTRS(animEvaluator, szAnimCurve, pNode, trsKeyTime, transAtTime, rotAtTime, scaleAtTime);


    if(txAnimCurve|| tyAnimCurve || tzAnimCurve || rxAnimCurve || ryAnimCurve || rzAnimCurve || sxAnimCurve || syAnimCurve || szAnimCurve) {	


	    channels[channels.size()] = WriteCurveChannels(aName, "translation", pNode, ++animAccessorCount);;
	    parameters[U("translation")] = WriteAnimParameters(pNode, transAtTime, ++animAccessorCount, "translation");
	    channels[channels.size()] = WriteCurveChannels(aName, "rotation", pNode, ++animAccessorCount); 
	    parameters[U("rotation")] = WriteAnimParameters(pNode, rotAtTime, ++animAccessorCount, "rotation");
	    channels[channels.size()] = WriteCurveChannels(aName, "scale", pNode, ++animAccessorCount); 
	    parameters[U("scale")]    = WriteAnimParameters(pNode, scaleAtTime, ++animAccessorCount, "scale");
    }

    if (channels.size() && parameters.size()){
	    parameters[U("TIME")] = WriteAnimParameters(pNode, trsKeyTime, ++animAccessorCount, "TIME");
	    _json [U("animations")][aName][U("channels")] = channels;
	    _json [U("animations")][aName][U("parameters")] = parameters;
    }
}

bool gltfWriter::WriteAnimation (FbxScene *pScene) 
{
    int nAnimStack, nAnimLayer;
    
    std::cout << "Animation frames "<< GetNumberOfAnimationFrames(pScene) << "\n";

    for (nAnimStack = 0; nAnimStack < pScene->GetSrcObjectCount<FbxAnimStack>(); nAnimStack++)
    {
        FbxAnimStack* lAnimStack = pScene->GetSrcObject<FbxAnimStack>(nAnimStack);
        int nAnimLayersTotal = lAnimStack->GetMemberCount<FbxAnimLayer>();
	for (nAnimLayer = 0; nAnimLayer < nAnimLayersTotal; nAnimLayer++)
	{
                FbxAnimLayer* lAnimLayer = lAnimStack->GetMember<FbxAnimLayer>(nAnimStack);
		FbxNode *pNode = pScene->GetRootNode();
                WriteAnimationLayer(lAnimLayer, pNode, false);
	}
    }
    
return true;
}
}
