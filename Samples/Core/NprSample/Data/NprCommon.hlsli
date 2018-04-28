__import ShaderCommon;
__import DefaultVS;
__import Shading;
#include "VertexAttrib.h"

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

  float nDotL = dot(normalW, gLights[0].worldDir);
#ifdef _DRAW_NDOTL
  return nDotL;
#endif

  if (nDotL > 0.65)
    return float3(1.f) * albedo;
  else if (nDotL > 0.5f)
    return float3(0.7f) * albedo;
  else if (nDotL > 0.35)
    return float3(0.35f) * albedo;
  else
    return float3(0.1f) * albedo;

}