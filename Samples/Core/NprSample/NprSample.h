/***************************************************************************
# Copyright (c) 2015, NVIDIA CORPORATION. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#  * Neither the name of NVIDIA CORPORATION nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***************************************************************************/
#pragma once
#include "Falcor.h"
#include "NprSceneRenderer.hpp"

using namespace Falcor;

class NprSample : public Sample
{
  public:
    void onLoad() override;
    void onFrameRender() override;
    void onShutdown() override;
    void onResizeSwapChain() override;
    bool onKeyEvent(const KeyboardEvent& keyEvent) override;
    bool onMouseEvent(const MouseEvent& mouseEvent) override;
    void onDataReload() override;
    void onGuiRender() override;

  private:
    const static std::string skDefaultScene;
    void loadScene(std::string filename);
    void createAndSetGBufferFbo();
    void renderGBuffer();
    void renderImageEdges();
    void renderGeoEdges();
    Scene::SharedPtr mpScene;
    NprSceneRenderer::UniquePtr mpSceneRenderer;
    vec2 mCameraDepthRange = vec2(0.001f, 100.0f);

    struct TonalArtMapGenPass
    {
        ComputeState::SharedPtr pState;
        ComputeVars::SharedPtr pVars;
        Texture::SharedPtr pTex;
    } mTonalArtMapGenData;

    struct TonalArtMapGenParameters
    {
        const uint minNumLinesPerThread = 2;
        const uint maxNumLinesPerThread = 6;
        const float minLineThickness = 2;
        const float maxLineThickness = 10;
        const float minLineWidth = 0.3f;
    } mTonalArtMapGenParameters;

    struct TonalArtMapGenData
    {
        float lineThickness;
        float lineWidth;
        float lineY; //[0,1] to determine what Y coord for line within thread's pixels
        float lineX; //same for x
    };

    struct HatchingData
    {
        //A lot of this is old from when I was loading hatch textures as pngs
        //Could use some changing no longer relevant with the tonal art map 
        //generation approach 
        static const uint32_t numHatchTex = 4;
        Texture::SharedPtr pHatchTexArray;
        Texture::SharedPtr pHatchTex[numHatchTex];
    } mHatchingData;

    struct ColorPass
    {
      GraphicsState::SharedPtr pState;
      GraphicsVars::SharedPtr pVars;
    } mColorPass;

    struct GbufferPass
    {
      GraphicsState::SharedPtr pState;
      GraphicsVars::SharedPtr pVars;
    } mGBuffer;

    enum ImageOperator { Sobel = 0, Prewitt = 1, Scharr = 2, ImageOpCount = 3 };
    const static Gui::DropdownList skImageOperatorList;
    struct ImageOperatorPass
    {
      FullScreenPass::UniquePtr pPass;
      GraphicsVars::SharedPtr pVars;
      ImageOperator imageOp;
      //for convenience. Actual depth is entered depth * scale
      //depth range is really small, lets you work in more reasonable #'s
      const float depthScale = .001f;
    } mImagePass;

    struct ImageOperatorPassData
    {
      glm::int2 textureDimensions;
      float normalThreshold = 175.0f;
      float depthThreshold = 0.001f;
    } mImagePassData;

    struct GeometryEdgePass
    {
      GraphicsState::SharedPtr pState;
      GraphicsVars::SharedPtr pVars;
      float edgeLength = 0.1f;
      bool useEdgeCaps = false;
    } mGeoEdgePass;

    struct GeometryEdgePassData
    {
      float edgeLength = 0.01f;
      float creaseThreshold = 0.85f;
      float facingBias = 0.001f;
      float zBias = 0.01f;
    } mGeoEdgePassData;

    enum EdgeMode { Image = 0, Geometry = 1 };
    const static Gui::DropdownList skEdgeModeList;
    EdgeMode mEdgeMode = EdgeMode::Image;

    //TODO pass in a light index rather than just doing nDotL0
    enum ShadingMode { EdgeOnly = 0, Albedo = 1, NDotL0 = 2, Toon = 3, Gooch = 4, Hatch = 5, ShadingModeCount = 6 };
    const static Gui::DropdownList skShadingModeList;
    ShadingMode mShadingMode = ShadingMode::EdgeOnly;
    static const uint32_t skNumThresholds = 3;
    struct NprShadingData
    {
      float toonThresholds[skNumThresholds] = {0.35f, 0.6f, 0.85f};
      float padding;
      float toonScalars[skNumThresholds + 1] = {0.1f, 0.35f, 0.7f, 1.0f};
      //w is the extent to which albedo color is mixed in
      float3 warmColor = float3(1, .29f, .29f);
      float warmAlbedoMix = 0.5f;
      float3 coolColor = float3(.62f, .58f, 1);
      float coolAlbedoMix = 0.25f;
      int hatchDebugIndex = 0;
      float3 camPos;
    } mShadingData;

    enum DebugMode { None = 0, Depth = 1, Normal = 2, 
                      EdgeUv = 3, EdgeU = 4, EdgeV = 5, 
                     HatchDebug = 6, DebugModeCount = 7};
    const static Gui::DropdownList skDebugModeList;
    struct DebugControls
    {
      DebugMode mode = None;
      float depthMin = 0.996f;
      float depthMax = 1.0f;
      FullScreenPass::UniquePtr pDebugPass;
      GraphicsVars::SharedPtr pVars;
    } mDebugControls;
};
