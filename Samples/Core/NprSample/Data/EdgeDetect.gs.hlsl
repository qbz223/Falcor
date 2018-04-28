//Based on Inking the Cube : Edge Detection with Direct3D 10, Doss 2008
__import DefaultVS;
__import ShaderCommon;

static const float kHalfPi = 1.57079632679f;
static const float kPi = 3.14159265359f;
static const float kTwoPi = 6.28318530718f;

struct GsOut
{
  float4 posH : SV_POSITION;
  float4 color : COLOR;
  float3 normalW : NORMAL;
};

cbuffer GsPerFrame
{
  float edgeLength;
  float creaseThreshold;
  float facingBias;
  float zBias;
  float3 ssCenter;
}

float getAngularDistance(float3 ssPos)
{
  float2 p = normalize(ssPos - ssCenter);
  float theta = atan2(p.y, p.x);
  theta += kPi;
  return theta;
}

float3 getFaceNormal(float3 a, float3 b, float3 c)
{
  float3 viewDir = -gCam.position + a;
  float3 edge0 = b - a;
  float3 edge1 = c - a;
  return normalize(cross(edge0, edge1));
}

//Based on House Edge Detection Thesis 2010
void extrudeEdgeCap(inout TriangleStream<GsOut> outStream, float3 v, float3 n, float4 s, float4 s1, float p)
{
  float2 dim = float2(1920, 1080);
  float4 temp = mul(float4(v + n, 1.0f), gCam.viewProjMat);
  temp.xy = (temp.xy / temp.w) * dim;
  float2 m = normalize(temp.xy - (s.xy * dim));
  float4 capVert = float4(((s * dim) + p * sign(dot(m, p))) / dim * s.w, s.zw);
  GsOut output;
  output.posH = s;
  output.normalW = n;
  output.color = float4(0, 0, 0, 1);
  outStream.Append(output);
  output.posH = capVert;
  outStream.Append(output);
  output.posH = s1;//s + 0.01f * float4(m, 0, 0);
  outStream.Append(output);
  outStream.RestartStrip();
}

void extrudeEdge(inout TriangleStream<GsOut> outStream, float3 aPos, float3 aNorm, float3 bPos, float3 bNorm)
{
  float3 aSSNorm = normalize(mul(aNorm, (float3x3)gCam.viewProjMat));
  float3 bSSNorm = normalize(mul(bNorm, (float3x3)gCam.viewProjMat));
  float aSideVec = 0.5f * aNorm * edgeLength;
  float bSideVec = 0.5f * bNorm * edgeLength;
  float4 edgeVerts[4] = {//float4 so can store posH
    float4(aPos - aSideVec, 0), float4(aPos + aSideVec, 0),
    float4(bPos - bSideVec, 0), float4(bPos + bSideVec, 0) };

  [unroll(4)]
  for(uint i = 0; i < 4; ++i)
  {
    GsOut output;
    float4 view = mul(float4(edgeVerts[i].xyz, 1.0f), gCam.viewMat);
    view.z -= zBias;
    output.posH = mul(view, gCam.projMat);
    float distance = getAngularDistance(output.posH.xyz / output.posH.w);
    float u = distance / kTwoPi;
    float v = i % 2 == 0 ? 0 : 1;
    //Save posH into edge verts to use for edge caps
    edgeVerts[i] = output.posH;
    output.normalW = i < 2 ? aNorm : bNorm;
    output.color = float4(0, u, v, 1);
    outStream.Append(output);
  }
  outStream.RestartStrip();

  //Artifacts this is meant to fix aren't even significant on the scenes im using 
  //Having issues with this, try to get it at the end, theres other stuff i want more 
  //float2 p = 2.f * normalize(float2(edgeVerts[0].y - edgeVerts[2].y, edgeVerts[2].x - edgeVerts[0].x));
  
  //extrudeEdgeCap(outStream, aPos, aSSNorm, edgeVerts[0], edgeVerts[1], p);
  //extrudeEdgeCap(outStream, bPos, bSSNorm, edgeVerts[2], edgeVerts[3], p);
}

void checkAdjacentTri(inout TriangleStream<GsOut> outStream, 
  float3 a, float3 b, float3 c, float3 centerNorm, float3 bNorm, float3 cNorm)
{
  float3 faceNorm = getFaceNormal(a, b, c);
  //Todo centroid?
  float3 view = normalize(-gCam.position + a);
  if(dot(faceNorm, view) >= facingBias)
  {
    extrudeEdge(outStream, c, cNorm, b, bNorm);
  }
  else
  {
    float between = dot(faceNorm, centerNorm);
    if(between < creaseThreshold)
    {
      extrudeEdge(outStream, c, cNorm, b, bNorm);
    }
  }
}

//3 for main tri. each edge is 4 verts for main fin and 3 for their corner correction 
[maxvertexcount(24)]
void main(triangleadj VS_OUT input[6], inout TriangleStream<GsOut> outStream)
{
  //Check if front facing tri
  float3 faceNorm = getFaceNormal(input[0].posW, input[2].posW, input[4].posW);
  float3 viewDir = normalize(-gCam.position + input[0].posW);
  //This effectively culls backfacing tris so can disable culling 
  //so dont need to worry about fins being culled
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
      //output.color = float4(1.f, input[triIndices[i]].texC, 0.f);
      output.color = float4(1.f, 1.f, 1.f, 0.f);
      outStream.Append(output);
    }
    outStream.RestartStrip();
  }
}
