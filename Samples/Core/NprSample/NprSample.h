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
    Scene::SharedPtr mpScene;
    SceneRenderer::SharedPtr mpSceneRenderer;
    vec2 mCameraDepthRange = vec2(0.001f, 100.0f);

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
      float normalThreshold = 25.0f;
      float depthThreshold = 0.001f;
    } mImagePassData;

    enum DebugMode { None = 0, Depth = 1, Normal = 2, DebugModeCount = 3};
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
