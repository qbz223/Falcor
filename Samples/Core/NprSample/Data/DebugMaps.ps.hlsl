cbuffer PerFrame
{
  float depthMin;
  float depthMax;
  float2 padding;
}

Texture2D gDebugTex;
SamplerState gSamplerState;
float4 main(float2 texC : TEXCOORD) : SV_TARGET

{
  float4 debugSample = gDebugTex.Sample(gSamplerState, texC);
  float3 resultColor = debugSample.xyz;
#ifdef _NORMAL
  resultColor = (debugSample.xyz + 1) * 0.5f;
#elif defined _DEPTH
  float depth = debugSample.x;
  float scaledDepth = (depth - depthMin) / (depthMax - depthMin);
  resultColor = scaledDepth;
#endif

  return float4(resultColor, 1.0f);
}