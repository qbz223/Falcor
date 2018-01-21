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

void ClassShadows::onGuiRender()
{
  if (mpGui->addButton("Load Scene"))
  {
    std::string filename;
    if (openFileDialog("Scene files\0 * .fscene\0\0", filename))
    {
      mpScene = Scene::loadFromFile(filename);
      mpSceneRenderer = SceneRenderer::create(mpScene);
    }
  }
  //if (mpGui->addButton("Load Model"))
  //{
  //  std::string filename;
  //  if (openFileDialog(Model::kSupportedFileFormatsStr, filename))
  //  {
  //    auto model = Model::createFromFile(filename.c_str());
  //    if (model)
  //    {
  //      mpScene->deleteAllModels();
  //      mpScene->addModelInstance(model, "MainModel");
  //    }
  //  }
  //}
}

void ClassShadows::onLoad()
{
  mpScene = Scene::create();
  mpScene->addCamera(Camera::create());
  mpState = GraphicsState::create();
  auto prog = GraphicsProgram::createFromFile("", "SimpleShadow.ps.hlsl");
  mpState->setProgram(prog);
  mpState->setFbo(mpDefaultFBO);
  mpVars = GraphicsVars::create(prog->getActiveVersion()->getReflector());

  //Stuff for heightmap pass
  mShadowPass.mpShadowMap = Texture::create2D(
    mpDefaultFBO->getWidth(),
    mpDefaultFBO->getHeight(),
    ResourceFormat::RGBA32Float,
    1u,
    UINT32_MAX,
    nullptr,
    Resource::BindFlags::ShaderResource | Resource::BindFlags::RenderTarget);
  mShadowPass.mpState = GraphicsState::create();
  auto passProg = GraphicsProgram::createFromFile("", "ShadowPass.ps.hlsl");
  mShadowPass.mpState->setProgram(passProg);
  mShadowPass.mpVars = GraphicsVars::create(passProg->getActiveVersion()->getReflector());
  mShadowPass.mpFbo = Fbo::create();
  mShadowPass.mpFbo->attachColorTarget(mShadowPass.mpShadowMap, 0);
  mShadowPass.mpState->setFbo(mShadowPass.mpFbo);

}

void ClassShadows::runShadowPass()
{
  auto cam = mpScene->getActiveCamera();
  auto prevCamData = cam->getData();

  auto light = mpScene->getLight(0);
  auto lightData = light->getData();

  cam->setPosition(-5.f * lightData.worldDir);
  cam->setTarget(vec3(0, 0, 0));

  mpRenderContext->pushGraphicsState(mShadowPass.mpState);
  mpRenderContext->pushGraphicsVars(mShadowPass.mpVars);
  mpSceneRenderer->renderScene(mpRenderContext.get());
  mpRenderContext->popGraphicsVars();
  mpRenderContext->popGraphicsState();

  cam->setPosition(prevCamData.position);
  cam->setTarget(prevCamData.target);

  mShadowPass.mpShadowMap = mShadowPass.mpFbo->getColorTexture(0);
}

void ClassShadows::onFrameRender()
{
	const glm::vec4 clearColor(0.38f, 0.52f, 0.10f, 1);
 	mpRenderContext->clearFbo(mpDefaultFBO.get(), clearColor, 1.0f, 0, FboAttachmentType::All);
  if(mpSceneRenderer)
  {
    mpSceneRenderer->update(mCurrentTime);

    runShadowPass();

    //y tho? (hacks around multiple swapchain error I still dont understand)
    mpState->setFbo(mpDefaultFBO);

    mpRenderContext->pushGraphicsState(mpState);
    mpRenderContext->pushGraphicsVars(mpVars);
    mpSceneRenderer->renderScene(mpRenderContext.get());
    mpRenderContext->popGraphicsVars();
    mpRenderContext->popGraphicsState();
  }
}

void ClassShadows::onShutdown()
{

}

bool ClassShadows::onKeyEvent(const KeyboardEvent& keyEvent)
{
  ///mpSceneRenderer->setCameraControllerType
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
