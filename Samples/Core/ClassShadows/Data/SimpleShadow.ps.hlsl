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
  float padding;
  float3 lightPos;
  float padding2;
  float2 shadowMapDim;
  //matrix lightViewProj
}

SamplerComparisonState gSampler;
SamplerState gTestSampler;
Texture2D gShadowMap;

//float getShadowFactor(float4 lightPosH)
float getShadowFactor(float4 lightPosH, float3 worldPos)
{
  //float depth = length(worldPos - lightPos);
  float depth = lightPosH.w;
  //float4 lightSpacePosH = mul(lightViewProj, float4(posW, 1.0f));
  //float2 uv = (lightPosH.xy * 0.5) + 0.5f; //* float2(1.0f/ shadowMapDim.x, 1.0f/shadowMapDim.y);
  //lightPosH /= lightPosH.w;
  //float2 uv = (lightPosH.xy + 1) * 0.5f;
  //float2 uv = lightSpacePosH.xy * float2(1.f/1920, 0);
  float2 uv = lightPosH.xy;
  uv.y = 1 - uv.y;
  //uv /= lightPosH.w;
  //float shadowDepth = gShadowMap.SampleCmpLevelZero(gSampler, uv, lightPos.z);
  float shadowDepth = gShadowMap.Sample(gTestSampler, uv).x;
  //return shadowDepth;
  //return (shadowDepth);
  //return(lightPosH.z);

  //return lightPosH.w / 5.f;
  //lightPosH /= lightPosH.w;
  //if(lightPosH.z >= shadowDepth)
  //Why this ridiculous bias? Seems like a bandaid on wrong
  //dont know why i need to calc worldspace distance?? 
  //Not sure why i cant (Correct) matrix version to work atm (just storing .z in light space)
  if(depth >= shadowDepth + 0.01f)
    return 0.25f;
  else
    return 1.0f;
}

float4 main(VS_OUT vOut) : SV_TARGET
{
  float shadowFactor = getShadowFactor(vOut.lightPosH, vOut.posW);
  float nDotL = dot(-vOut.normalW, lightDir) * shadowFactor;
  //return float4(getShadowFactor(vOut.lightPosH), 0.0f, 1.0f);
  return float4(nDotL.xxx, 1.0f);
  //return float4(shadowFactor.xxx, 1.0f);
}