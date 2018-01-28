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
#include "ClassShadows.h"

Gui::DropdownList ClassShadows::kShadowModes =
{
  { Basic, "Basic" },
  { Variance, "Variance" },
  { Moment, "Moment" },
};

void ClassShadows::onGuiRender()
{
  //Scene Loading
  if (mpGui->addButton("Load Scene"))
  {
    std::string filename;
    if (openFileDialog("Scene files\0 * .fscene\0\0", filename))
    {
      mpScene = Scene::loadFromFile(filename);
      mpSceneRenderer = SceneRenderer::create(mpScene);
    }
  }

  mpGui->addSeparator();

  if(mpSceneRenderer)
  {
    //Light Controls
    if(mpGui->beginGroup("LightData"))
    {
      mpGui->addFloatVar("Dir Light distance", mShadowPass.dirLightDistance);
      mpScene->getLight(0)->renderUI(mpGui.get());
      mpGui->endGroup();
    }
    mpGui->addSeparator();

    if(mpGui->beginGroup("Shadows"))
    {
      //Current Shadow mode
      if(mpGui->addDropdown("Mode", kShadowModes, mShadowMode))
      {
        //Update shader defines to reflect changed mode
        auto& prog = mpState->getProgram();
        auto& passProg = mShadowPass.mpState->getProgram();
        prog->clearDefines();
        passProg->clearDefines();

        if(mShadowMode == Variance)
        {
          prog->addDefine("VARIANCE");
          passProg->addDefine("VARIANCE");
        }
        else if(mShadowMode == Moment)
        {
          prog->addDefine("MOMENT");
          passProg->addDefine("MOMENT");
        }
      }

      //culling
      if(mpGui->addCheckBox("Front face culling", mbFrontFaceCulling))
      {
        if(mbFrontFaceCulling)
          mShadowPass.mpState->setRasterizerState(mpFrontFaceCull);
        else
          mShadowPass.mpState->setRasterizerState(mpBackFaceCull);
      }

      mpGui->addFloatVar("Depth Bias", mPsPerFrame.depthBias, 0);
      mpGui->endGroup();
    }
    mpGui->addSeparator();

    //Debug drawing shadow map controls
    if (mpGui->beginGroup("Debug Maps"))
    {
      mpGui->addCheckBox("Debug Draw Map", mDebugData.bShouldDebugDrawShadowMap);
      if(mDebugData.bShouldDebugDrawShadowMap)
      {
        mpGui->addFloatVar("Debug Coef", mDebugData.debugCoef, 0.0001f);
        mpGui->addFloatVar("Left", mDebugData.position.x, 0, mpDefaultFBO->getWidth() - mDebugData.size.x);
        mpGui->addFloatVar("Top", mDebugData.position.y, 0, mpDefaultFBO->getHeight() - mDebugData.size.y);
        mpGui->addFloatVar("Width", mDebugData.size.x, 1, (float)mpDefaultFBO->getWidth());
        mpGui->addFloatVar("Height", mDebugData.size.y, 1, (float)mpDefaultFBO->getHeight());
      }
    }
  }
}

void ClassShadows::onLoad()
{
  //Main Pass
  mpScene = Scene::create();
  auto cam = Camera::create();
  cam->setDepthRange(0.01f, 100.f);
  mpScene->addCamera(cam);
  mpState = GraphicsState::create();
  auto prog = GraphicsProgram::createFromFile("ShadowVS.slang", "SimpleShadow.ps.hlsl");
  mpState->setProgram(prog);
  mpState->setFbo(mpDefaultFBO);
  Sampler::Desc samplerDesc;
  samplerDesc.setFilterMode(Sampler::Filter::Linear, Sampler::Filter::Linear, Sampler::Filter::Linear);
  samplerDesc.setAddressingMode(Sampler::AddressMode::Border, Sampler::AddressMode::Border, Sampler::AddressMode::Border);
  samplerDesc.setBorderColor(float4(99999, 0, 0, 1));
  mpVars = GraphicsVars::create(prog->getActiveVersion()->getReflector());
  mpVars->setSampler("gTestSampler", Sampler::create(samplerDesc));

  //Shadow Pass
  //Texture
  mShadowPass.mpShadowMap = Texture::create2D(
    mpDefaultFBO->getWidth(),
    mpDefaultFBO->getHeight(),
    ResourceFormat::RGBA32Float,
    1u,
    1u,
    nullptr,
    Resource::BindFlags::ShaderResource | Resource::BindFlags::RenderTarget);
  //debug texture 
  mShadowPass.mpDebugShadowMap = Texture::create2D(
    mpDefaultFBO->getWidth(),
    mpDefaultFBO->getHeight(),
    ResourceFormat::RGBA32Float,
    1u,
    1u,
    nullptr,
    Resource::BindFlags::ShaderResource | Resource::BindFlags::RenderTarget);
  auto shadowDepth = Texture::create2D(
    mpDefaultFBO->getWidth(),
    mpDefaultFBO->getHeight(),
    ResourceFormat::D32Float,
    1u,
    1u,
    nullptr,
    Resource::BindFlags::ShaderResource | Resource::BindFlags::DepthStencil);
  //State
  mShadowPass.mpState = GraphicsState::create();
  auto passProg = GraphicsProgram::createFromFile("", "ShadowPass.ps.hlsl");
  mShadowPass.mpState->setProgram(passProg);
  mShadowPass.mpFbo = Fbo::create();
  mShadowPass.mpFbo->attachColorTarget(mShadowPass.mpShadowMap, 0);
  mShadowPass.mpFbo->attachColorTarget(mShadowPass.mpDebugShadowMap, 1);
  mShadowPass.mpFbo->attachDepthStencilTarget(shadowDepth);
  mShadowPass.mpState->setFbo(mShadowPass.mpFbo);

  //Vars
  mShadowPass.mpVars = GraphicsVars::create(passProg->getActiveVersion()->getReflector());

  //Rs state
  auto rsDesc = RasterizerState::Desc();
  mpBackFaceCull = RasterizerState::create(rsDesc);
  rsDesc.setCullMode(RasterizerState::CullMode::Front);
  mpFrontFaceCull = RasterizerState::create(rsDesc);

  //Initial UI data
  mDebugData.position = vec2(mpDefaultFBO->getWidth() - 600, 0);
  mDebugData.size = vec2(600, 600 * ((float)mpDefaultFBO->getHeight() / mpDefaultFBO->getWidth()));
}

void ClassShadows::runShadowPass()
{
  //Clear shadow map
  mpRenderContext->clearFbo(mShadowPass.mpFbo.get(), vec4(99999.f, 0, 0, 0), 1.f, 0);

  //cache cam data
  auto cam = mpScene->getActiveCamera();
  auto prevCamData = cam->getData();

  //set cam to perspective of light
  auto light = mpScene->getLight(0);
  auto lightData = light->getData();
  auto effectiveLightPos = -mShadowPass.dirLightDistance * lightData.worldDir;
  cam->setPosition(effectiveLightPos);
  cam->setTarget(vec3(0, 0, 0));
  cam->setUpVector(vec3(0, 1, 0));

  //Update debug factor
  auto cb = mShadowPass.mpVars->getConstantBuffer("PsPerFrame");
  cb->setBlob(&mDebugData.debugCoef, 0, sizeof(float));

  //render
  mpRenderContext->pushGraphicsState(mShadowPass.mpState);
  mpRenderContext->pushGraphicsVars(mShadowPass.mpVars);
  mpSceneRenderer->renderScene(mpRenderContext.get());
  mpRenderContext->popGraphicsVars();
  mpRenderContext->popGraphicsState();

  //Set PsPerFrameData while have light anyway
  mLightViewProj = glm::mat4(cam->getViewProjMatrix());
  mPsPerFrame.lightDir = lightData.worldDir;
  auto fboTex = mShadowPass.mpFbo->getColorTexture(0);
  //auto fboTex = mShadowPass.mpFbo->getDepthStencilTexture();

  //Restore previous camera state
  cam->setPosition(prevCamData.position);
  cam->setTarget(prevCamData.target);
  cam->setUpVector(prevCamData.up);

  if(mShadowMode == Variance)
  {
    //Do blur here
  }

  //Save resulting shadow maps
  mShadowPass.mpShadowMap = fboTex;
  mShadowPass.mpDebugShadowMap = mShadowPass.mpFbo->getColorTexture(1);
}

void ClassShadows::debugDrawShadowMap()
{
  //Blit shadow map onto main pass fbo
  vec4 srcRect(0, 0, mShadowPass.mpFbo->getWidth(), mShadowPass.mpFbo->getHeight());
  vec4 dstRect(mDebugData.position.x, mDebugData.position.y, 
              mDebugData.position.x + mDebugData.size.x, mDebugData.position.y + mDebugData.size.y);
  mpRenderContext->blit(mShadowPass.mpDebugShadowMap->getSRV(), mpDefaultFBO->getColorTexture(0)->getRTV(),
    srcRect, dstRect);
}

void ClassShadows::onFrameRender()
{
	const glm::vec4 clearColor(0.38f, 0.52f, 0.10f, 1);
 	mpRenderContext->clearFbo(mpDefaultFBO.get(), clearColor, 1.0f, 0, FboAttachmentType::All);
  if(mpSceneRenderer)
  {
    mpSceneRenderer->update(mCurrentTime);

    runShadowPass();

    auto vsCb = mpVars->getConstantBuffer("LightMatrixBuffer");
    vsCb->setBlob(&mLightViewProj, 0, sizeof(mat4));

    auto psCb = mpVars->getConstantBuffer("PsPerFrame");
    psCb->setBlob(&mPsPerFrame, 0, sizeof(PsPerFrame));
    mpVars->setSrv(0, 0, 0, mShadowPass.mpShadowMap->getSRV());

    //y tho? (hacks around multiple swapchain error I still dont understand)
    mpState->setFbo(mpDefaultFBO);

    mpRenderContext->pushGraphicsState(mpState);
    mpRenderContext->pushGraphicsVars(mpVars);
    mpSceneRenderer->renderScene(mpRenderContext.get());
    mpRenderContext->popGraphicsVars();
    mpRenderContext->popGraphicsState();

    if(mDebugData.bShouldDebugDrawShadowMap)
      debugDrawShadowMap();
  }
}

void ClassShadows::onShutdown()
{

}

bool ClassShadows::onKeyEvent(const KeyboardEvent& keyEvent)
{
  if(mpSceneRenderer)
    return mpSceneRenderer->onKeyEvent(keyEvent);
  else
    return false;
}

bool ClassShadows::onMouseEvent(const MouseEvent& mouseEvent)
{
  if(mpSceneRenderer)
    return mpSceneRenderer->onMouseEvent(mouseEvent);
  else 
    return false;
}

void ClassShadows::onDataReload()
{

}

void ClassShadows::onResizeSwapChain()
{
  if(mpScene)
  {
    auto pFbo = mpState->getFbo();
    float width = (float)pFbo->getWidth();
    float height = (float)pFbo->getHeight();
    auto pCam = mpScene->getActiveCamera();
    pCam->setFocalLength(21.0f);
    pCam->setAspectRatio(width / height);
  }
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
    ClassShadows sample;
    SampleConfig config;
    config.windowDesc.title = "Falcor Project Template";
    config.windowDesc.resizableWindow = true;
    sample.run(config);
    return 0;
}
