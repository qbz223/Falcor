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

float4 main(VS_OUT vOut) : SV_TARGET
{
  return float4(vOut.posH.w, vOut.posH.w, vOut.posH.w, 1.0f);
}