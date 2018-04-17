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

float3 getFaceNormal(float3 a, float3 b, float3 c)
{
  float3 viewDir = -gCam.position + a;
  float3 edge0 = b - a;
  float3 edge1 = c - a;
  return normalize(cross(edge0, edge1));
}

void extrudeEdge(inout TriangleStream<GsOut> outStream, float3 aPos, float3 aNorm, float3 bPos, float3 bNorm)
{
  float3 edgeVerts[4];
  edgeVerts[0] = aPos;
  float realEdgeLength = 0.0001f;
  edgeVerts[1] = aPos + aNorm * realEdgeLength;
  edgeVerts[2] = bPos;
  edgeVerts[3] = bPos + bNorm * realEdgeLength;
  for(uint i = 0; i < 4; ++i)
  {
    GsOut output;
    float4 view = mul(float4(edgeVerts[i], 1.0f), gCam.viewMat);
    view.z -= 0.01f;
    output.posH = mul(view, gCam.projMat);
    output.normalW = i < 2 ? aNorm : bNorm;
    output.color = float4(0, 0, 0, 1);
    outStream.Append(output);
  }
  outStream.RestartStrip();
}

void checkAdjacentTri(inout TriangleStream<GsOut> outStream, 
  float3 a, float3 b, float3 c, float3 centerNorm, float3 bNorm, float3 cNorm)
{
  const float bias = 0.0001f;
  float3 faceNorm = getFaceNormal(a, b, c);
  //Todo centroid?
  float3 view = normalize(-gCam.position + a);
  if(dot(faceNorm, view) >= bias)
  {
    extrudeEdge(outStream, c, cNorm, b, bNorm);
  }
  else
  {
    float between = dot(faceNorm, centerNorm);
    if(between < 0.85f)
    {
      extrudeEdge(outStream, c, cNorm, b, bNorm);
    }
  }
}

//3 for main tri. each edge can create up to 4 verts
[maxvertexcount(15)]
void main(triangleadj VS_OUT input[6], inout TriangleStream<GsOut> outStream)
{
  //Check if front facing tri
  float3 faceNorm = getFaceNormal(input[0].posW, input[2].posW, input[4].posW);
  float3 viewDir = normalize(-gCam.position + input[0].posW);
  if(dot(faceNorm, viewDir) < 0.f)
  {
    //Check edge tri adjacent to 0, 2
    checkAdjacentTri(outStream, input[1].posW, input[2].posW, input[0].posW, 
      faceNorm, input[2].normalW, input[0].normalW);

    //Check edge tri adjacent to 2, 4
    checkAdjacentTri(outStream, input[3].posW, input[4].posW, input[2].posW,
      faceNorm, input[4].normalW, input[2].normalW);

    //Check edge tri adjacent to 4, 0
    checkAdjacentTri(outStream, input[5].posW, input[0].posW, input[4].posW,
      faceNorm, input[0].normalW, input[4].normalW);


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
}
