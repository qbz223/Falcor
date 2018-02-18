cbuffer VSPerFrame
{
  matrix world;
  matrix viewProj;
};

struct VS_IN
{
  float4 pos         : POSITION;
#ifdef HAS_NORMAL
  float3 normal      : NORMAL;
#endif
#ifdef HAS_BITANGENT
  float3 bitangent   : BITANGENT;
#endif
#ifdef HAS_TEXCRD
  float2 texC        : TEXCOORD;
#endif
#ifdef HAS_LIGHTMAP_UV
  float2 lightmapC   : LIGHTMAP_UV;
#endif
#ifdef HAS_COLORS
  float3 color       : DIFFUSE_COLOR;
#endif
#ifdef _VERTEX_BLENDING
  float4 boneWeights : BONE_WEIGHTS;
  uint4  boneIds     : BONE_IDS;
#endif
  uint instanceID : SV_INSTANCEID;
};

struct VS_OUT
{
  float4 posH : SV_POSITION;
  float3 posW : POSITION;
  float3 norm : NORMAL;
};

VS_OUT main(VS_IN vIn)
{
  VS_OUT vOut;
  vOut.posW = mul(world, vIn.pos).xyz;
  vOut.posH = mul(viewProj, vOut.posW);
#ifdef HAS_NORMAL
  vOut.norm = vIn.normal;
#endif
  return vOut;
}