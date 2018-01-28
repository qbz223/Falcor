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
#ifdef VARIANCE
  float depthSq = vOut.posH.w * vOut.posH.w;
  pOut.mainColor = float4(vOut.posH.w, depthSq, 0.f, 1.0f);
#else
  pOut.mainColor = float4(vOut.posH.www, 1.0f);
#endif
  pOut.debugColor = float4(pOut.mainColor.xyz / debugCoef, 1.0f);
  return pOut;
}