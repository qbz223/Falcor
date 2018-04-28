struct GsOut
{
  float4 posH : SV_POSITION;
  float4 color : COLOR;
  float3 normalW : NORMAL;
};

float4 main(GsOut psIn) : SV_TARGET0
{ 
  //Edges have 0 in this
  if(psIn.color.x < 0.1f)
  {
    float u = psIn.color.y;
    float3 color;
    if(u < 0.5)
    {
      //[0, 0.5] -> [0, 1]
      float t = u * 2;
      color = lerp(float3(1, 0, 0), float3(0, 1, 0), t);
    }
    else 
    {
      //[0.5, 1] -> [0, 1]
      float t = (u - 0.5f) * 2;
      color = lerp(float3(0, 1, 0), float3(0, 0, 1), t);
    }

    return float4(color, 1);
  }
  else
  {
    return psIn.color;
  }
}