struct TonalArtMapGenData
{
    float lineThickness;
    float lineWidth;
    float lineY;
    float lineX;
};

StructuredBuffer<uint> gLineCounts;
StructuredBuffer<TonalArtMapGenData> gLineData;

RWTexture2D gTonalArtMap;


[numthreads(16, 1, 1)]
void main(int3 groupID : SV_GroupID, int3 threadID : SV_GroupThreadID)
{
    float w;
    float h;
    gTonalArtMap.GetDimensions(w, h);
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
        uint lineY = (uint)(pixPerThreadY * lineYt) + yOffset;
        float emptySpace = (1.0f - lineW) * w;
        uint lineX = (uint)(emptySpace * lineXt);

        uint lineStartY = (uint)(lineY - 0.5f * lineThickness);
        uint lineEndY = (uint)(lineY + 0.5f * lineThickness);

        for (uint j = lineStartY; j < lineEndY; ++j)
        {
            float dist = abs(lineY - j);
            float color = dist / (lineEndY - lineStartY);

            for (uint k = lineX; k <= (uint)(w - lineX); ++k)
            {
                gTonalArtMap[float2(k, j)] = float4(color, color, color, 1.0f);
            }
        }
    }

    //
    //for (uint k = 0; k < numLinesPerThread; ++k)
    //{
    //    float indexT = (float)index / 16.0f;
    //    if (k == 1)
    //    {
    //        indexT = (float)(16 - index) / 16.0f;
    //    }
    //    
    //    if (k == 2)
    //    {
    //        indexT += (1.0f - indexT) * 0.5f;
    //    }
    //    
    //    uint lineH = (uint)(pixPerThreadH * index + pixPerThreadH * indexT);
    //    uint amntBlank = (uint)(0.0375f * k * w);

    //    uint hStart = (uint)pixPerThreadH * index;
    //    uint hEnd = (uint)pixPerThreadH * (index + 1);
    //    const int lineDist = 3 * (3 - k + 1);
    //    for (uint j = hStart; j < hEnd; ++j)
    //    {
    //        for (uint i = 0; i < (uint)w; ++i)
    //        {
    //            int distance = abs((int)j - (int)lineH);
    //            if (distance <= lineDist && (i >= amntBlank && i <=(uint)(w - amntBlank)))
    //            {
    //                float color = (float)distance / lineDist;
    //                gTonalArtMap[float2(i, j)] = float4(color, color, color, 1);
    //            }
    //        }
    //    }
    //}
}
