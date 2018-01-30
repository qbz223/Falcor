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
  float4 shadowSample = gShadowMap.Sample(gTestSampler, uv);
  float shadowDepth = shadowSample.x;
  if(shadowSample.w < 0.001f)
    shadowDepth = 9999.f;
#ifdef VARIANCE 
  float shadowDepthSq = shadowSample.y;
  float variance = max(shadowDepthSq - shadowDepth * shadowDepth, depthBias);
  float depthDelta = depth - shadowDepth;
  float p = variance / (variance + depthDelta * depthDelta);
  if(depth > shadowDepth)
    return p;
  else
    return 1.0f;  
#elif defined MOMENT
  float4 bPrime = (1.f - depthBias) * shadowSample + depthBias * 0.5f;
  float3x3 decomp = {        1,  bPrime.x, bPrime.y,
                      bPrime.x,   bPrime.y, bPrime.z,
                      bPrime.y,   bPrime.z, bPrime.w};
  float3 depthVec = float3(1.f, depth, depth * depth);
  //-- Calc C. decomp = M, depthVec = D
  //M * C = D
  //M-1 * M * C = M-1 * D
  //C = M-1 * D
  //I can do it above with inverse as described but that seems wrong/slow?
  //Paper advics choelsku decomposition but idk how that solves??

  //-- Calc roots
  //solve c3 * z^2 + c2 * z + c1 = 0 for z using quadratic formula 
  // -c2 +- sqrt(c2^2 - 4 * c3 * c1) / (2 * c3)
  //Let z2 <= z3 denote solutions 

  //-- If depth <= z2
  //  return zero
  
  //-- If depth <= z3
  //  g = (depth * z3 - b'1(depth + z3) + b'2) / ((z3 - z2) * (depth - z2))
  //  return g

  //-- if depth > z3
  //  g = (z2 * z3 - b'1(z2 + z3) + b'2) / ((depth - z2) * (depth - z3))
  //  return 1 - g

  return bPrime.z;
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
#elif defined MOMENT
  float nDotL = shadowFactor;
#else
  float nDotL = dot(-vOut.normalW, lightDir) * shadowFactor;
#endif
  return float4(nDotL.xxx, 1.0f);
}