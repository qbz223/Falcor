__import DefaultVS;
__import ShaderCommon;
__import Shading;
__import Helpers;

float4 main(VS_OUT vOut) : SV_TARGET
{
  ShadingAttribs attr;
  attr.lodBias = 0;
  attr.UV = vOut.texC;
  applyAlphaTest(gMaterial, attr, vOut.posW);

  return float4(vOut.normalW, 1.0f);
}