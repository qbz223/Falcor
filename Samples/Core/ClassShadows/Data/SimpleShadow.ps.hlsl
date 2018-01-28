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
  float4 lightPosH : LIGHTPOSITION;
};

cbuffer PsPerFrame
{
  float3 lightDir;
  float depthBias;
}

SamplerComparisonState gSampler;
SamplerState gTestSampler;
Texture2D gShadowMap;

float getShadowFactor(float4 lightPosH, float3 worldPos)
{
  float depth = lightPosH.w;
  float2 uv = lightPosH.xy;
  uv.y = 1 - uv.y;
  float shadowDepth = gShadowMap.Sample(gTestSampler, uv).x;

  if(depth >= shadowDepth + depthBias)
    return 0.25f;
  else
    return 1.0f;
}

float4 main(VS_OUT vOut) : SV_TARGET
{
  float shadowFactor = getShadowFactor(vOut.lightPosH, vOut.posW);
  float nDotL = dot(-vOut.normalW, lightDir) * shadowFactor;
  return float4(nDotL.xxx, 1.0f);
}