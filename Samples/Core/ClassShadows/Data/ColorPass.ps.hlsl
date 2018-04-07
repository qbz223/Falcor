__import DefaultVS;

float4 main(VS_OUT vOut) : SV_TARGET
{
  float nDotL = dot(vOut.normalW, float3(0.25f, 0.5f, 0.75f));
  return float4(nDotL, nDotL, nDotL, 1.0f);
}