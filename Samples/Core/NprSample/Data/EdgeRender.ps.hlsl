struct GsOut
{
  float4 posH : SV_POSITION;
  float4 color : COLOR;
  float3 normalW : NORMAL;
};

float4 main(GsOut psIn) : SV_TARGET0
{
  return psIn.color;
}