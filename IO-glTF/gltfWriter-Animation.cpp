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

std::vector<float> WriteCurveKeys(FbxAnimCurve* pCurve);

web::json::value gltfWriter::WriteSkeleton(FbxNode *pNode) {

    FbxSkeleton* lSkeleton = (FbxSkeleton*) pNode->GetNodeAttribute();

    if (lSkeleton->GetSkeletonType() == FbxSkeleton::eLimbNode &&
        pNode->GetParent() &&
        pNode->GetParent()->GetNodeAttribute() &&
        pNode->GetParent()->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton)
    {
	web::json::value node = WriteNode (pNode) ;
        web::json::value ret  = web::json::value::object ({ { U("nodes"), node } }) ;
	return ret;	
    }
return (web::json::value::null());
}

void gltfWriter::WriteAnimationLayer(FbxAnimLayer* pAnimLayer, FbxNode* pNode, bool isSwitcher)
{
    int lModelCount;

    WriteAnimationChannels(pNode, pAnimLayer, WriteCurveKeys, false);

    for(lModelCount = 0; lModelCount < pNode->GetChildCount(); lModelCount++)
    {
        WriteAnimationLayer(pAnimLayer, pNode->GetChild(lModelCount), false);
    }
}

static int InterpolationFlagToIndex(int flags)
{
    if( (flags & FbxAnimCurveDef::eInterpolationConstant) == FbxAnimCurveDef::eInterpolationConstant ) return 1;
    if( (flags & FbxAnimCurveDef::eInterpolationLinear) == FbxAnimCurveDef::eInterpolationLinear ) return 2;
    if( (flags & FbxAnimCurveDef::eInterpolationCubic) == FbxAnimCurveDef::eInterpolationCubic ) return 3;
    return 0;
}


void WriteKeyTimeAndValues(FbxAnimCurve* xyzAnimCurve, std::vector<float>& xyzKeyValues, std::vector<float>& KeyValues, std::vector<unsigned int>& KeyTime) {
if (xyzAnimCurve) {
                xyzKeyValues = WriteCurveKeys(xyzAnimCurve);
                KeyValues.insert(std::end(KeyValues), std::begin(xyzKeyValues), std::end(xyzKeyValues));
                for (unsigned int index=0; index < xyzKeyValues.size(); index++)
                        KeyTime.push_back(index);
        }
}

web::json::value gltfWriter::WriteCurveChannels(utility::string_t aName, utility::string_t trs, FbxAnimCurve* xAnimCurve, FbxNode *pNode, std::vector<float> KeyValues, int animAccessorCount) {

    static const char* interPolation[] = { "?", "constant", "linear", "cubic"};
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
    samplerDef [U("interpolation")] = web::json::value::string (interPolation[ InterpolationFlagToIndex(xAnimCurve->KeyGetInterpolation(0)) ]); 
    samplerDef [U("output")] = web::json::value::string (utility::conversions::to_string_t(trs));
    _json [U("animations")][aName][U("samplers")] [samplerName]= samplerDef;


return animationChannel;
}

template<class Type>
web::json::value gltfWriter::WriteAnimParameters(FbxNode *pNode, std::vector<Type> KeyValues, int animAccessorCount, utility::string_t trs) {

   utility::string_t animAccName;
   web::json::value ret;
   web::json::value accessorsAndBufferViews =web::json::value::object ({
                { U("accessors"), web::json::value::object () },
                { U("bufferViews"), web::json::value::object () }
        }) ;

    animAccName = utility::conversions::to_string_t (U("_animAccessor_")) ; 
    animAccName += utility::conversions::to_string_t ((int)animAccessorCount);
    web::json::value animAccessor =WriteArray <Type>(KeyValues, 1, pNode, animAccName.c_str()) ;
    ret = web::json::value::string (GetJsonFirstKey (animAccessor [U("accessors")])) ;
    MergeJsonObjects (accessorsAndBufferViews, animAccessor) ;
    MergeJsonObjects (_json, accessorsAndBufferViews) ;

   return ret;
}

void gltfWriter::WriteAnimationChannels(FbxNode* pNode, FbxAnimLayer* pAnimLayer, std::vector<float> (*WriteCurve) (FbxAnimCurve* pCurve), bool isSwitcher)
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
   
    utility::string_t samplerName;

    utility::string_t aName;
    utility::string_t animAccName;
    aName = utility::conversions::to_string_t (U("animation_")) ; 
    aName += utility::conversions::to_string_t ((int)++animCount);

    std::vector<float> transKeyValues, rotKeyValues, scaleKeyValues;
    std::vector<float> txKeyValues, tyKeyValues, tzKeyValues;
    std::vector<float> rxKeyValues, ryKeyValues, rzKeyValues;
    std::vector<float> sxKeyValues, syKeyValues, szKeyValues;
    std::vector<unsigned int> trsKeyTime;

   // Write general curves.
    if (!isSwitcher)
    {
	// translation 
	FbxAnimCurve* txAnimCurve = pNode->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X);
	FbxAnimCurve* tyAnimCurve = pNode->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y);
	FbxAnimCurve* tzAnimCurve = pNode->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);

	if (txAnimCurve) WriteKeyTimeAndValues(txAnimCurve, txKeyValues, transKeyValues, trsKeyTime);
	if (tyAnimCurve) WriteKeyTimeAndValues(tyAnimCurve, tyKeyValues, transKeyValues, trsKeyTime);
	if (tzAnimCurve) WriteKeyTimeAndValues(txAnimCurve, tzKeyValues, transKeyValues, trsKeyTime);

        if (txAnimCurve || tyAnimCurve || tzAnimCurve)
        {
	    channels[channels.size()] = WriteCurveChannels(aName, "translation", txAnimCurve, pNode, transKeyValues, ++animAccessorCount);;
	    parameters[U("translation")] = WriteAnimParameters(pNode, transKeyValues, ++animAccessorCount, "translation");
         }

	// rotation 
	FbxAnimCurve* rxAnimCurve = pNode->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X);
	FbxAnimCurve* ryAnimCurve = pNode->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y);
	FbxAnimCurve* rzAnimCurve = pNode->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);

	if (rxAnimCurve) WriteKeyTimeAndValues(rxAnimCurve, rxKeyValues, rotKeyValues, trsKeyTime);
	if (ryAnimCurve) WriteKeyTimeAndValues(ryAnimCurve, ryKeyValues, rotKeyValues, trsKeyTime);
	if (rzAnimCurve) WriteKeyTimeAndValues(rxAnimCurve, rzKeyValues, rotKeyValues, trsKeyTime);

        if (rxAnimCurve || ryAnimCurve || rzAnimCurve)
        {
	       channels[channels.size()] = WriteCurveChannels(aName, "rotation", rxAnimCurve, pNode, rotKeyValues, ++animAccessorCount); 
	       parameters[U("rotation")] = WriteAnimParameters(pNode, rotKeyValues, ++animAccessorCount, "rotation");
        } 

	// scaling 
	FbxAnimCurve* sxAnimCurve = pNode->LclScaling.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X);
	FbxAnimCurve* syAnimCurve = pNode->LclScaling.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y);
	FbxAnimCurve* szAnimCurve = pNode->LclScaling.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);

	if (sxAnimCurve) WriteKeyTimeAndValues(sxAnimCurve, sxKeyValues, scaleKeyValues, trsKeyTime);
	if (syAnimCurve) WriteKeyTimeAndValues(syAnimCurve, syKeyValues, scaleKeyValues, trsKeyTime);
	if (szAnimCurve) WriteKeyTimeAndValues(sxAnimCurve, szKeyValues, scaleKeyValues, trsKeyTime);
	
       if(sxAnimCurve || syAnimCurve || szAnimCurve)
        {
	       channels[channels.size()] = WriteCurveChannels(aName, "scale", sxAnimCurve, pNode, scaleKeyValues, ++animAccessorCount); 
	       parameters[U("scale")]    = WriteAnimParameters(pNode, scaleKeyValues, ++animAccessorCount, "scale");
        }
		// [parameters][time]
	       parameters[U("TIME")] = WriteAnimParameters(pNode, trsKeyTime, ++animAccessorCount, "TIME");
		if (channels.size()){
		//Write channels and parameters
                _json [U("animations")][aName][U("channels")] = channels;
                _json [U("animations")][aName][U("parameters")] = parameters;
		}
    } 
}

std::vector<float> WriteCurveKeys(FbxAnimCurve* pCurve)
{
    float   lKeyValue;
    int     lCount;
    int     lKeyCount = pCurve->KeyGetCount();
    std::vector<float> trsKeyValues;


    for(lCount = 0; lCount < lKeyCount; lCount++)
    {
        lKeyValue = static_cast<float>(pCurve->KeyGetValue(lCount));
	trsKeyValues.push_back(lKeyValue);
    }

return trsKeyValues;
}

bool gltfWriter::WriteAnimation (FbxScene *pScene) 
{
    int nAnimStack, nAnimLayer;

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
