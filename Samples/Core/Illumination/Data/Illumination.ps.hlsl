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
Texture2D gIrradianceMap;

cbuffer PsPerFrame
{
  float exposure;
};

float3 linearToneMap(float3 color)
{
  float val = (exposure * color) / (exposure * color + float3(1.0f, 1.0f, 1.0f));
  return pow(val, (1.0f / 2.2f));
}

float2 dirToSphereCoords(float3 dir)
{
  dir = normalize(-dir);
  float u = 0.5 - atan(dir.yx) / kTwoPi;
  float v = acos(dir.z) / kPi;
  return float2(u, v);
}

float4 main(VS_OUT vOut) : SV_TARGET
{
  //float nDotL = dot(vOut.normalW, float3(0.25f, 0.5f, 0.75f));
  float4 irr = gIrradianceMap.Sample(gSampler, dirToSphereCoords(vOut.normalW));
#ifdef USE_TONE_MAPPING
  float3 irrColor = linearToneMap(irr.xyz);
#else
  float3 irrColor = irr.xyz;
#endif
  float kd = 0.7f;
  float3 color = (kd / kPi) * irrColor;
  return float4(irrColor.xyz, 1.0f);
  //return float4(nDotL, nDotL, nDotL, 1.0f);
}