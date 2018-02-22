__import DefaultVS;

//It's like implicit ambient
static const float kShadowMin = 0.25f;

struct VS_OUT_SHADOWS
{
  VS_OUT defaultOut;
  float4 lightPosH : LIGHTPOS;
};

cbuffer PsPerFrame
{
  float3 lightDir;
  float depthBias;
  float debugCoef;
}

SamplerComparisonState gSampler;
SamplerState gTestSampler;
Texture2D gShadowMap;

float2 getShadowUv(float4 lightPosH)
{
  float2 uv = lightPosH.xy / lightPosH.w;
  uv = (uv * 0.5f + 0.5f);
  uv.y = 1 - uv.y;
  return uv;
}

bool uvInBounds(float2 uv)
{
  if (uv.x > 1 || uv.y > 1 || uv.x < 0 || uv.y < 0)
    return false;
  else
    return true;
}

float3 solveCramers(float4 bPrime, float3 result)
{
  float3x3 mat0 = 
  float3x3(1,  bPrime.x, bPrime.y,
    bPrime.x,   bPrime.y, bPrime.z,
    bPrime.y,   bPrime.z, bPrime.w);
  float3x3 mat1 = 
  float3x3(result.x,  bPrime.x, bPrime.y,
    result.y,   bPrime.y, bPrime.z,
    result.z,   bPrime.z, bPrime.w);
  float3x3 mat2 = 
  float3x3(1,  result.x, bPrime.y,
    bPrime.x,   result.y, bPrime.z,
    bPrime.y,   result.z, bPrime.w);
  float3x3 mat3 = 
  float3x3(1,  bPrime.x, result.x,
    bPrime.x,   bPrime.y, result.y,
    bPrime.y,   bPrime.z, result.z);
  
  float det0 = determinant(mat0);
  float det1 = determinant(mat1);
  float det2 = determinant(mat2);
  float det3 = determinant(mat3);

  float x = det1 / det0;
  float y = det2 / det0;
  float z = det3 / det0;

  return float3(x, y, z);
}

float4 getShadowSample(float4 lightPosH)
{
  return gShadowMap.Sample(gTestSampler, getShadowUv(lightPosH));
}

float momentShadowFactor(float depth, float4 shadowSample)
{
  float4 bPrime = (1.f - -depthBias) * shadowSample + -depthBias * 0.5f;
  float3 depthVec = float3(1.f, depth, depth * depth);
  //-- Calc C. decomp = M, depthVec = D
  //M * C = D
  //M-1 * M * C = M-1 * D
  //C = M-1 * D
  //Paper advises solving with cholesky decomposition but this is easier 
  float3 c = solveCramers(bPrime, depthVec);

  //-- Calc roots
  //solve c3 * z^2 + c2 * z + c1 = 0 for z using quadratic formula 
  // -c2 +- sqrt(c2^2 - 4 * c3 * c1) / (2 * c3)
  //Let z2 <= z3 denote solutions 
  float sqrtTerm = sqrt(c.y * c.y - 4 * c.z * c.x);
  float z2 = (-c.y - sqrtTerm) / (2 * c.z);
  float z3 = (-c.y + sqrtTerm) / (2 * c.z);

  if(z2 > z3)
  {
    float temp = z2;
    z2 = z3;
    z3 = temp;
  }

  //-- If depth <= z2
  //  return zero
  if(depth <= z2)
    return 0;

  //-- If depth <= z3
  //  g = (depth * z3 - b'1(depth + z3) + b'2) / ((z3 - z2) * (depth - z2))
  //  return g
  if(depth <= z3)
  {
    return (depth * z3 - bPrime.x * (depth + z3) + bPrime.y) / ((z3 - z2) * (depth - z2));
  }
  //-- if depth > z3
  //  g = (z2 * z3 - b'1(z2 + z3) + b'2) / ((depth - z2) * (depth - z3))
  //  return 1 - g
  else
  {
    return 1 - (z2 * z3 - bPrime.x * (z2 + z3) + bPrime.y) / ((depth - z2) * (depth - z3));
  }
}

float applyImplicitAmbient(float shadowFactor)
{
  return lerp(kShadowMin, 1, shadowFactor);
}

float getShadowFactor(float4 lightPosH)
{
  float depth = lightPosH.w;
  float4 shadowSample = getShadowSample(lightPosH);
  float shadowDepth = shadowSample.x;
  if(shadowSample.w < 0.001f)
    shadowDepth = 9999.f;
#ifdef VARIANCE 
  float shadowDepthSq = shadowSample.y;
  float variance = max(shadowDepthSq - shadowDepth * shadowDepth, depthBias);
  float depthDelta = depth - shadowDepth;
  float p = variance / (variance + depthDelta * depthDelta);
  if(depth > shadowDepth + depthBias)
  {
    //[0 - 1] -> [0.25 -> 1]
    return applyImplicitAmbient(p);
  }
  else
    return 1.0f;  
#elif defined MOMENT
  float2 uv = getShadowUv(lightPosH);
  if(uvInBounds(uv))
  {
    float p = 1 - saturate(momentShadowFactor(depth, shadowSample));
    return applyImplicitAmbient(p);
  }
  else
    return 1;
#else 
  if(depth >= shadowDepth + depthBias)
    return 0.25f;
  else
    return 1.0f;
#endif

}

float4 main(VS_OUT_SHADOWS vOut) : SV_TARGET
{
#ifdef DRAW_MAP
  float4 shadowSample = getShadowSample(vOut.lightPosH);
  shadowSample.xyz /= debugCoef;
  return shadowSample;
#endif

#ifdef DRAW_UV
  float2 uv = getShadowUv(vOut.lightPosH);
  if(uv.x > 1 || uv.y > 1 || uv.x < 0 || uv.y < 0)
    return float4(0, 0, 1, 1);
  return float4(uv, 0, 1);
#endif

  float shadowFactor = getShadowFactor(vOut.lightPosH);
  float nDotL = dot(-vOut.defaultOut.normalW, lightDir) * shadowFactor;
  return float4(nDotL.xxx, 1.0f);
}