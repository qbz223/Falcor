__import DefaultVS;
#include "NprCommon.hlsli"

struct PS_OUT
{
  float4 normal : SV_TARGET0;
  float4 color : SV_TARGET1;
};

PS_OUT main(VS_OUT vOut)
{
  PS_OUT output;
  //TODO these can be 3 component textures eventually
  output.color = float4(calcColor(vOut.posW, -vOut.normalW, vOut.bitangentW, vOut.texC), 1.0f);
  output.normal = float4(vOut.normalW, 1.0f);
  return output;
}