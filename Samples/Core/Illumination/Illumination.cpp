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
#include "Illumination.h"

void Illumination::onGuiRender()
{
  if (mpGui->addButton("Load Scene"))
  {
    std::string filename;
    if (openFileDialog("Scene files\0 * .fscene\0\0", filename))
    {
      mpScene = Scene::loadFromFile(filename);      
      mpScene->getActiveCamera()->setDepthRange(0.001f, 1000.0f);
      mpSceneRenderer = SceneRenderer::create(mpScene);
    }
  }

  if(mpGui->addButton("Load Skybox"))
  {
    std::string filename;
    if (openFileDialog("HDR files\0 * .hdr\0\0", filename))
    {
      mpHdrImage = createTextureFromFile(filename, false, false, Resource::BindFlags::ShaderResource);
      mpSkybox = SkyBox::create(mpHdrImage, mpSampler);

      std::string irrFilename = filename.substr(0, filename.size() - 4);
      irrFilename = irrFilename.append(".irr.hdr");
      mpIrradianceMap = createTextureFromFile(irrFilename, false, false, Resource::BindFlags::ShaderResource);
      mpVars->setSrv(0, 0, 0, mpIrradianceMap->getSRV());;

      mDebugSettings.shouldDrawIrr = false;
    }
  }

  if(mpGui->beginGroup("Tone Mapping"))
  {
    if(mpGui->addCheckBox("Enabled", mDebugSettings.enableToneMapping))
    {
      if(mDebugSettings.enableToneMapping)
        mpState->getProgram()->addDefine("USE_TONE_MAPPING");
      else
        mpState->getProgram()->clearDefines();
    }

    if(mDebugSettings.enableToneMapping)
    {
      mpGui->addFloatVar("Exposure", mPsPerFrame.exposure);
    }

    mpGui->endGroup();
  }

  if(mpGui->beginGroup("Debug"))
  {
    if(mpGui->addCheckBox("Draw Irradiance", mDebugSettings.shouldDrawIrr))
    {
      if(mDebugSettings.shouldDrawIrr)
        mpSkybox = SkyBox::create(mpIrradianceMap, mpSampler);
      else
        mpSkybox = SkyBox::create(mpHdrImage, mpSampler);
    }
  }
}

void Illumination::onLoad()
{
  mpScene = Scene::loadFromFile("Scenes\\DragonPlane.fscene");
  mpScene->getActiveCamera()->setDepthRange(0.001f, 1000.0f);
  mpSceneRenderer = SceneRenderer::create(mpScene);
  mpState = GraphicsState::create();
  auto prog = GraphicsProgram::createFromFile("", "Illumination.ps.hlsl");
  mpState->setProgram(prog);
  mpState->setFbo(mpDefaultFBO);
  mpVars = GraphicsVars::create(prog->getActiveVersion()->getReflector());

  mpSampler = Sampler::create(Sampler::Desc());
}

void Illumination::onFrameRender()
{
	const glm::vec4 clearColor(0.38f, 0.52f, 0.10f, 1);
 	mpRenderContext->clearFbo(mpDefaultFBO.get(), clearColor, 1.0f, 0, FboAttachmentType::All);
  if(mpSceneRenderer)
  {
    mpSceneRenderer->update(mCurrentTime);
    //y tho? (hacks around multiple swapchain error I still dont understand)
    mpState->setFbo(mpDefaultFBO);

    if(mDebugSettings.enableToneMapping)
    {
      mpVars->getConstantBuffer("PsPerFrame")->setBlob(&mPsPerFrame, 0, sizeof(PsPerFrame));
    }

    if(mpSkybox)
    {
      mpSkybox->render(mpRenderContext.get(), mpScene->getActiveCamera().get());
    }

    mpRenderContext->pushGraphicsState(mpState);
    mpRenderContext->pushGraphicsVars(mpVars);
    mpSceneRenderer->renderScene(mpRenderContext.get());
    mpRenderContext->popGraphicsVars();
    mpRenderContext->popGraphicsState();
  }
}

void Illumination::onShutdown()
{

}

bool Illumination::onKeyEvent(const KeyboardEvent& keyEvent)
{
  if(mpSceneRenderer)
    return mpSceneRenderer->onKeyEvent(keyEvent);
  else
    return false;
}

bool Illumination::onMouseEvent(const MouseEvent& mouseEvent)
{
  if(mpSceneRenderer)
    return mpSceneRenderer->onMouseEvent(mouseEvent);
  else 
    return false;
}

void Illumination::onDataReload()
{

}

void Illumination::onResizeSwapChain()
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
    Illumination sample;
    SampleConfig config;
    config.windowDesc.title = "Falcor Project Template";
    config.windowDesc.resizableWindow = true;
    sample.run(config);
    return 0;
}
