__import ShaderCommon;
__import DefaultVS;
__import Shading;
#include "VertexAttrib.h"

Texture2DArray gHatchTex;
SamplerState gSampler;

cbuffer NprShadingData
{
  //only uses xyz but needs to be aligned
  float4 toonThresholds;
  float4 toonScalars; 
  float3 warmColor;
  float warmAlbedoMix;
  float3 coolColor;
  float coolAlbedoMix;
  int hatchDebugIndex;
  float3 cameraPos;
}

#if !defined _LIGHT_COUNT
#define _LIGHT_COUNT 1
#endif

float3 calcColor(float3 posW, float3 normalW, float3 bitanW, float2 texC)
{
  ShadingAttribs shAttr;
  prepareShadingAttribs(gMaterial, posW, gCam.position, normalW, bitanW, texC, shAttr);
  ShadingOutput result;
  result.finalValue = 0;
  float4 finalColor = 0;
  float envMapFactor = 1;
  float opacity = 1;
  for (uint l = 0; l < _LIGHT_COUNT; l++)
  {
    evalMaterial(shAttr, gLights[l], 1, result, l == 0);
  }
  float3 albedo = result.diffuseAlbedo;
#ifdef _DRAW_ALBEDO
  return albedo;
#endif

  float nDotL = dot(normalize(normalW), normalize(gLights[0].worldDir));
#ifdef _DRAW_NDOTL
  return nDotL.xxx;
#endif

#ifdef _TOON
  nDotL = (nDotL + 1) / 2.0f;
  if (nDotL > toonThresholds.z)
    return float3(toonScalars.w) * albedo;
  else if (nDotL > toonThresholds.y)
    return float3(toonScalars.z) * albedo;
  else if (nDotL > toonThresholds.x)
    return float3(toonScalars.y) * albedo;
  else
    return float3(toonScalars.x) * albedo;
#endif

#ifdef _GOOCH
  float3 objWarm = warmColor + albedo * warmAlbedoMix;
  float3 objCool = coolColor + albedo * coolAlbedoMix;
  return lerp(objCool, objWarm, nDotL);
#endif

#ifdef _HATCHDEBUG
  //return gHatchTex.Sample(gSampler, float3(texC, hatchDebugIndex));
  return gHatchTex.Sample(gSampler, float3(texC, hatchDebugIndex));
#endif

#ifdef _HATCH
  const int numHatchTex = 4;
  float distance = length(cameraPos - posW) * 0.25f;
  int hatchIndex = 0;
  nDotL = (nDotL + 1) / 2.0f;
  if (nDotL > 0.925f)
    //return float4(nDotL/100.0f, 0.f, 0, 1);
    hatchIndex = 0;
  else if (nDotL > 0.7f)
    //  return float4(0, 1, 0, 1);
    hatchIndex = 1;
  else if (nDotL > 0.35f)
    //  return float4(0, 0, 1, 1);
    hatchIndex = 2;
  else
    //  return float4(1, 1, 0, 1);
    hatchIndex = 3;

  return gHatchTex.SampleLevel(gSampler, float3(texC, hatchIndex), distance);
#endif
//If no defines, is edge only
  return float3(1, 1, 1);
}