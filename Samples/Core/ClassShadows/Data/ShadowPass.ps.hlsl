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
  float3 lightPos;  
}

float4 main(VS_OUT vOut) : SV_TARGET
{ 
  //i dont understand why i need to 
  float depth = length(vOut.posW - lightPos);
  //return float4(depth.xxx, 1.0f);
  return float4(vOut.posH.www, 1.0f);
}