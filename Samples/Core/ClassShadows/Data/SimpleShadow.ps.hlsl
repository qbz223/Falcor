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
  float4 shadowSample = gShadowMap.Sample(gTestSampler, uv).x;
  float shadowDepth = shadowSample.x;
#ifdef VARIANCE 
  float shadowDepthSq = shadowSample.y;
  float variance = max(shadowDepthSq - shadowDepth * shadowDepth, depthBias);
  float depthDelta = depth - shadowDepth;
  float p = variance / (variance + depthDelta * depthDelta);
  if(depth > shadowDepth)
    return p;
  else
    return 1.0f;  
#else 
  if(depth >= shadowDepth + depthBias)
    return 0.25f;
  else
    return 1.0f;
#endif

}

float4 main(VS_OUT vOut) : SV_TARGET
{
  float shadowFactor = getShadowFactor(vOut.lightPosH, vOut.posW);
#ifdef VARIANCE
  float nDotL = min(dot(-vOut.normalW, lightDir), shadowFactor) * shadowFactor;
#else
  float nDotL = dot(-vOut.normalW, lightDir) * shadowFactor;
#endif
  return float4(nDotL.xxx, 1.0f);
}