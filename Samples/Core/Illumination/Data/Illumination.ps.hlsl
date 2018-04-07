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

static const float kTwoPi = 6.28318530718f;
static const float kPi = 3.14159265359f;

SamplerState gSampler;
Texture2D gSkybox;
Texture2D gIrradianceMap;
Buffer<float2> gRandomPoints;

cbuffer PsPerFrame
{
  float exposure;
  float3 eyePos;
  float alpha; 
  float kd;
  float ks; 
  int lodBias;
};

float3 linearToneMap(float3 color)
{
  float3 linColor = pow(color, 2.2f);
  float3 val = (exposure * linColor) / (exposure * linColor + float3(1.0f, 1.0f, 1.0f));
  return pow(val, (1.0f / 2.2f));
}

float2 dirToSphereCoords(float3 dir)
{
  dir.y *= -1;
  dir = normalize(dir);
  float u = 0.5 - atan2(dir.z, dir.x) / kTwoPi;
  float v = acos(-dir.y) / kPi;
  return float2(u, v);
}

float3 sphereCoordsToDir(float2 uv)
{
  float x = cos(2 * kPi * (0.5f - uv.x)) * sin(kPi * uv.y);
  float y = cos(kPi * uv.y);
  float z = sin(2 * kPi * (0.5 - uv.x)) * sin(kPi * uv.y);
  return float3(x, y, z);
}

float getSpecFactor(float3 dir, float3 halfVec)
{
  float dirDotHalf = dot(dir, halfVec);
  float intermediate = pow((1 - dirDotHalf), 5);
  return (ks + (1.0f - ks) * intermediate) / (4 * dirDotHalf);
}

float3 vecToColor(float3 vec)
{
  return (vec + 1) * 0.5f;
}

float getMipLevel(float3 normal, float3 h, float alpha, float numSamples)
{
  float d = ((alpha + 2) / kTwoPi) * pow(dot(normal, h), alpha);
  uint width, height, samples;
  //TODO this is slow
  gSkybox.GetDimensions(0, width, height, samples);
  float intermediate = (width * height) / (float)numSamples;
  return (0.5f * log2(intermediate) - 0.5f * log2(d / 4.f)) + lodBias;
}

float3 calcSpecular(float3 r, float3 view, float3 n)
{
  const int numSamples = 40;
  float3 a = normalize(cross(float3(0, 1, 0), r));
  float3 b = normalize(cross(r, a));

  float3 color = float3(0, 0, 0);
  [unroll(numSamples)]
  for(int i = 0; i < numSamples; ++i)
  {
    float2 currentPoint = gRandomPoints[i];
    float u = currentPoint.x;
    float v = acos(pow(currentPoint.y, (1.0f / (alpha + 1)))) / kPi;
    float3 dir = sphereCoordsToDir(float2(u, v));
    float3 skewDir = normalize(dir.x * b + dir.y * r + dir.z * a);
    float3 halfVec = normalize(skewDir + view);
    float lod = getMipLevel(n, halfVec, alpha, numSamples);
    float3 skyboxColor = gSkybox.SampleLevel(gSampler, dirToSphereCoords(skewDir), lod).xyz;
#ifdef USE_TONE_MAPPING
    skyboxColor = linearToneMap(skyboxColor);
#endif
    float3 contribution = getSpecFactor(skewDir, halfVec) * skyboxColor * (dot(n, r)); //n dot r is cos theta 
    color += contribution;
  }
  return color / (float)numSamples;
}

float4 main(VS_OUT vOut) : SV_TARGET
{
  //Diffuse
  float3 dif = gIrradianceMap.Sample(gSampler, dirToSphereCoords(vOut.normalW)).xyz;
#ifdef USE_TONE_MAPPING
  dif = linearToneMap(dif);
#endif 
  dif *= (kd / kPi);

  //Specular
  float3 view = normalize(eyePos - vOut.posW);
  float3 r = 2 * dot(vOut.normalW, view) * vOut.normalW - view;
  float3 spec = calcSpecular(r, view, vOut.normalW);

  float3 irr = spec + dif;
  return float4(irr, 1.0f);
  //return float4(nDotL, nDotL, nDotL, 1.0f);
}