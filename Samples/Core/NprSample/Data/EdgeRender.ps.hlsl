__import ShaderCommon;
__import DefaultVS;
__import Shading;
#include "VertexAttrib.h"
#include "NprCommon.hlsli"

struct GsOut
{
  float4 posH : SV_POSITION;
  float4 color : COLOR;
  float3 normalW : NORMAL;
  float3 posW : POSITION;
  float2 texC : TEXCOORD;
  float3 bitangentW : BITANGENT;
};

float4 main(GsOut psIn) : SV_TARGET0
{ 
#if (defined _EDGE_U || defined _EDGE_V)
  //Edges have 0 in this
  if(psIn.color.x < 0.1f)
  {
#if defined _EDGE_U
    float x = psIn.color.y;
#else 
    float x = psIn.color.z;
#endif
    float3 color;
    if(x < 0.5)
    {
      //[0, 0.5] -> [0, 1]
      float t = x * 2;
      color = lerp(float3(1, 0, 0), float3(0, 1, 0), t);
    }
    else 
    {
      //[0.5, 1] -> [0, 1]
      float t = (x - 0.5f) * 2;
      color = lerp(float3(0, 1, 0), float3(0, 0, 1), t);
    }

    return float4(color, 1);
  }
  else
  {
    return psIn.color;
  }
#else
  //Edge
  if(psIn.color.x < 0.1f)
  {
    return psIn.color;
  }
  else
  {
    float3 color = calcColor(psIn.posW, psIn.normalW, psIn.bitangentW, psIn.texC);
    return float4(color, 1.0f);
  }

#endif
}