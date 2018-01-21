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

cbuffer PsPerFrame
{
  matrix lightViewProj;
  float3 lightDir;
  float padding;
  float2 shadowMapDim;
}

float2 getShadowUv(float3 posW)
{
  float4 lightSpacePosH = mul(float4(posW, 1.0f), lightViewProj);
  float2 uv = lightSpacePosH.xy * float2(1.0f/ shadowMapDim.x, 1.0f/ shadowMapDim.y);
  uv.y = 1 - uv.y;
  return uv;
}

float4 main(VS_OUT vOut) : SV_TARGET
{
  float nDotL = dot(-vOut.normalW, lightDir);
  return float4(nDotL, nDotL, nDotL, 1.0f);
}