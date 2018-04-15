//Based on Inking the Cube : Edge Detection with Direct3D 10, Doss 2008
__import DefaultVS;
__import ShaderCommon;

struct GsOut
{
  float4 posH : SV_POSITION;
  float4 color : COLOR;
  float3 normalW : NORMAL;
};

cbuffer GsPerFrame
{
  float edgeLength;
}

void extrudeEdge(inout TriangleStream<GsOut> outStream, float3 aPos, float3 aNorm, float3 bPos, float3 bNorm)
{
  float3 edgeVerts[4];
  edgeVerts[0] = aPos;
  edgeVerts[1] = aPos + aNorm * edgeLength;
  edgeVerts[2] = bPos;
  edgeVerts[3] = bPos + bNorm * edgeLength;
  for(uint i = 0; i < 4; ++i)
  {
    GsOut output;
    output.posH = mul(float4(edgeVerts[i], 1.0f), gCam.viewProjMat);
    output.normalW = i < 2 ? aNorm : bNorm;
    output.color = float4(0, 0, 0, 1);
    outStream.Append(output);
  }
  outStream.RestartStrip();
}

bool triIsFrontFacing(float3 viewDir, float3 a, float3 b, float3 c)
{
  float3 edge0 = b - a;
  float3 edge1 = c - a;
  float3 faceNorm = normalize(cross(edge0, edge1));
  return dot(faceNorm, viewDir) < 0;
}

//3 for main tri. each edge can create up to 4 verts
[maxvertexcount(15)]
void main(triangleadj VS_OUT input[6], inout TriangleStream<GsOut> outStream)
{
  //This is arbitrary. Might be better to test centroid? Easy to try later
  float3 viewDir = -input[0].posW;
  //Check if front facing tri
  if(triIsFrontFacing(viewDir, input[0].posW, input[2].posW, input[4].posW))
  {
    //Check edge tri adjacent to 0, 2
    if (!triIsFrontFacing(viewDir, input[1].posW, input[2].posW, input[0].posW))
    {
      extrudeEdge(outStream, input[0].posW, input[0].normalW, input[2].posW, input[2].normalW);
    }

    //Check edge tri adjacent to 4, 0
    if(!triIsFrontFacing(viewDir, input[5].posW, input[0].posW, input[4].posW))
    {
      extrudeEdge(outStream, input[4].posW, input[4].normalW, input[0].posW, input[0].normalW);
    }
    
    //Check edge tri adjacent to 2, 4
    if (!triIsFrontFacing(viewDir, input[3].posW, input[4].posW, input[2].posW))
    {
      extrudeEdge(outStream, input[2].posW, input[2].normalW, input[4].posW, input[4].normalW);
    }
  }

  //Output the main triangle
  int triIndices[3] = { 0, 2, 4 };
  for (int i = 0; i < 3; ++i)
  {
    GsOut output;
    output.posH = mul(float4(input[triIndices[i]].posW, 1.0f), gCam.viewProjMat);
    output.normalW = input[triIndices[i]].normalW;
    output.color = float4(1.f, 1.f, 1.f, 1.f);
    outStream.Append(output);
  }
  outStream.RestartStrip();
}