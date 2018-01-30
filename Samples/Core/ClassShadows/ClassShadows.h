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

class ClassShadows : public Sample
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
    void runShadowPass();
    void debugDrawShadowMap();

    Scene::SharedPtr mpScene ;
    SceneRenderer::SharedPtr mpSceneRenderer;
    GraphicsState::SharedPtr mpState;
    GraphicsVars::SharedPtr mpVars;
    
    //Data for debug drawing shadow map
    struct DebugMapData
    {
      bool bShouldDebugDrawShadowMap = false;
      vec2 position;
      vec2 size;
      float debugCoef = 10.f; //divide the depth by this to make it more easily viewable 
    } mDebugData;

    //Stuff for shadow pass
    struct ShadowPass
    {
      Texture::SharedPtr mpShadowMap;
      Texture::SharedPtr mpDebugShadowMap;
      Texture::SharedPtr mpBlurTex;
      Texture::SharedPtr mpDebugBlurTex;
      Fbo::SharedPtr mpFbo;
      //Fbo for blur results
      Fbo::SharedPtr mpBlurFbo;
      Fbo::SharedPtr mpDebugFbo;
      GraphicsVars::SharedPtr mpVars;
      GraphicsState::SharedPtr mpState;
      float dirLightDistance = 2.f;
    } mShadowPass;

    mat4 mLightViewProj = glm::mat4();

    //Main pass ps cbuffer
    struct PsPerFrame
    {
      vec3 lightDir;
      float depthBias = 0.19f;
    } mPsPerFrame;

    bool mbFrontFaceCulling = false;
    RasterizerState::SharedPtr mpBackFaceCull;
    RasterizerState::SharedPtr mpFrontFaceCull;
    GraphicsState::SharedPtr mpBlurState;
    GaussianBlur::UniquePtr mpBlur;

    typedef uint32_t ShadowMode; enum {Basic = 0, Variance = 1, Moment = 2};
    ShadowMode mShadowMode = Basic;
    static Gui::DropdownList kShadowModes;
};
