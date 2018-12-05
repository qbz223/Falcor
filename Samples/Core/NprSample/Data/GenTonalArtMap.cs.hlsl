struct TonalArtMapGenData
{
    float lineThickness;
    float lineWidth;
    float lineY;
    float lineX;
};

cbuffer DataPerRun
{
    uint texIndex;
};

StructuredBuffer<uint> gLineCounts;
StructuredBuffer<TonalArtMapGenData> gLineData;
RWTexture2DArray gTonalArtMap;

#ifdef _VERTICAL
SamplerState gSampler;
Texture2D gPrevTex;
#endif

[numthreads(16, 1, 1)]
void main(int3 groupID : SV_GroupID, int3 threadID : SV_GroupThreadID)
{
    float w;
    float h;
    float elements; 
    gTonalArtMap.GetDimensions(w, h, elements);
    uint index = 16 * groupID.y + threadID.x;

    int lineDataOffset = 0;
    for (uint z = 0; z < index; ++z)
    {
        lineDataOffset += gLineCounts[z];
    }

    uint numLines = gLineCounts[index];
    float pixPerThreadY = h / 16.0f;
    for (uint i = 0; i < numLines; ++i)
    {
        uint dataIndex = lineDataOffset + i;
        float lineYt = gLineData[dataIndex].lineY;
        float lineXt = gLineData[dataIndex].lineX;
        float lineW = gLineData[dataIndex].lineWidth;
        float lineThickness = gLineData[dataIndex].lineThickness;

        uint yOffset = (uint)(pixPerThreadY * index);
        int lineY = (int)(pixPerThreadY * lineYt) + yOffset;
        float emptySpace = (1.0f - lineW) * w;
        uint lineX = (uint)(emptySpace * lineXt);

        uint lineStartY = (uint)(lineY - 0.5f * lineThickness);
        uint lineEndY = (uint)(lineY + 0.5f * lineThickness);

        for (uint j = lineStartY; j < lineEndY; ++j)
        {
            float dist = abs(lineY - j);
            float color = dist / (lineEndY - lineStartY);
            color = 0;

            for (uint k = lineX; k <= (uint)(w - lineX); ++k)
            {
#ifdef _VERTICAL
                float4 existingColor = gPrevTex[float2(j, k)];
                float4 newColor;
                newColor.x = min(existingColor.x, color);
                newColor.y = min(existingColor.y, color);
                newColor.z = min(existingColor.z, color);
                newColor.w = 1;
                gTonalArtMap[float3(j, k, texIndex)] = newColor;
#else
                gTonalArtMap[float3(k, j, texIndex)] = float4(color, color, color, 1.0f);
#endif          
            }
        }
    }
}
