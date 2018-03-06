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
  float3 eyePos;
};

float3 linearToneMap(float3 color)
{
  float3 linColor = pow(color, 2.2f);
  float3 val = (exposure * linColor) / (exposure * linColor + float3(1.0f, 1.0f, 1.0f));
  return pow(val, (1.0f / 2.2f));
}

float2 dirToSphereCoords(float3 dir)
{
  dir = normalize(-dir);
  float u = 0.5 - atan(dir.zx * float2(-1, 1)) / kTwoPi;
  float v = acos(-dir.y) / kPi;
  return float2(u, v);
}

float3 sphereCoordsToDir(float2 uv)
{
  float x = cos(2 * kPi * (0.5f - uv.x)) * sin(kPi * uv.y);
  float y = sin(2 * kPi * (0.5 - uv.x)) * sin(kPi * uv.y);
  float z = cos(kPi * uv.y);
  //think herron's formula assumes opengl. should test this quickly with a sphere
  return float3(x, -y, z);
}

float getSpecFactor(float3 dir, float3 view)
{
  const float ks = 1.0f;
  float3 halfVec = (dir + view) / 2.0f;
  float dirDotHalf = dot(dir, halfVec);
  float intermediate = pow((1 - dirDotHalf), 5);
  return (ks + (1.0f - ks) * intermediate) / (4 * dirDotHalf * dirDotHalf);
}

float4 main(VS_OUT vOut) : SV_TARGET
{
  //float nDotL = dot(vOut.normalW, float3(0.25f, 0.5f, 0.75f));
  //float3 view = normalize(eyePos - vOut.posW);
  //float3 r = 2 * dot(vOut.normalW, view) * vOut.normalW - view;
  float4 irr = gIrradianceMap.Sample(gSampler, dirToSphereCoords(vOut.normalW));
#ifdef USE_TONE_MAPPING
  float3 irrColor = linearToneMap(irr.xyz);
#else
  float3 irrColor = irr.xyz;
#endif
  return float4(irrColor, 1.0f);
  float kd = 1.0f;
  float3 color = (kd / kPi) * irrColor;
  return float4(irrColor.xyz, 1.0f);
  //return float4(nDotL, nDotL, nDotL, 1.0f);
}