struct VS_OUT
{
  float3 normalW    : NORMAL;
  float3 bitangentW : BITANGENT;
  float2 texC       : TEXCRD;
  float3 posW       : POSW;
  float3 colorV     : COLOR;
  float4 prevPosH   : PREVPOSH;
  float2 lightmapC  : LIGHTMAPUV;
  float4 posH : SV_POSITION;
};

struct PS_OUT
{
  float4 mainColor : SV_TARGET0;
  float4 debugColor : SV_TARGET1;
};

cbuffer PsPerFrame
{
  float debugCoef;
};

PS_OUT main(VS_OUT vOut) : SV_TARGET
{ 
  PS_OUT pOut;
  pOut.mainColor = float4(vOut.posH.www, 1.0f);
  pOut.debugColor = float4(vOut.posH.www / debugCoef, 1.0f);
  return pOut;
}