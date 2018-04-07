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
#include "NprSample.h"

const std::string NprSample::skDefaultScene = "Scenes/MultiModel.fscene";

const Gui::DropdownList NprSample::skDebugModeList = 
{
  { (int32_t)DebugMode::None, "None" },
  { (int32_t)DebugMode::Depth, "Depth" },
  { (int32_t)DebugMode::Normal, "Normal" }
};

void NprSample::onGuiRender()
{
  if(mpGui->beginGroup("Scene"))
  {
    if (mpGui->addButton("Load Scene"))
    {
      std::string filename;
      if (openFileDialog("Scene files\0 * .fscene\0\0", filename))
      {
        loadScene(filename);
      }
    }

    if (mpGui->addButton("Load Model"))
    {
      std::string filename;
      if (openFileDialog(Model::kSupportedFileFormatsStr, filename))
      {
        auto model = Model::createFromFile(filename.c_str());
        if (model)
        {
          mpScene->deleteAllModels();
          mpScene->addModelInstance(model, "MainModel");
        }
      }
    }

    if(mpGui->addFloat2Var("Camera Depth Range", mCameraDepthRange, 0.001f, 1000000.0f))
    {
      mpScene->getActiveCamera()->setDepthRange(mCameraDepthRange.x, mCameraDepthRange.y);
    }

    mpGui->endGroup();
  }


  if(mpGui->beginGroup("Debug"))
  {
    uint32_t uMode = (uint32_t)mDebugControls.mode;
    if(mpGui->addDropdown("Mode", skDebugModeList, uMode))
    {
      mDebugControls.mode = (DebugMode)uMode;
      switch(mDebugControls.mode)
      {
        mDebugControls.pDebugPass->getProgram()->clearDefines(); 
        case None:
          break;
        case Depth:
          mDebugControls.pDebugPass->getProgram()->addDefine("_DEPTH");
          break;
        case Normal:
          mDebugControls.pDebugPass->getProgram()->addDefine("_NORMAL");
          break;
        default:
          should_not_get_here();
      }
    }

    if(mDebugControls.mode == Depth)
    {
      mpGui->addFloatVar("Depth Min", mDebugControls.depthMin, 0.000001f);
      mpGui->addFloatVar("Depth Max", mDebugControls.depthMax, 1.0f);
    }
    mpGui->endGroup();
  }
}

void NprSample::onLoad()
{
  //Default Scene
  mpScene = Scene::create();
  auto defaultCam = Camera::create();
  defaultCam->setDepthRange(mCameraDepthRange.x, mCameraDepthRange.y);
  mpScene->addCamera(defaultCam);

  //Color
  mColorPass.pState = GraphicsState::create();
  auto prog = GraphicsProgram::createFromFile("", "ColorPass.ps.hlsl");
  mColorPass.pState->setProgram(prog);
  mColorPass.pState->setFbo(mpDefaultFBO);
  mColorPass.pVars = GraphicsVars::create(prog->getActiveVersion()->getReflector());

  //Gbuffer
  auto gBufState = GraphicsState::create();
  Fbo::Desc gBufDesc;
  gBufDesc.setColorTarget(0, ResourceFormat::RGBA32Float);
  gBufDesc.setDepthStencilTarget(ResourceFormat::D32Float);
  auto gBufFbo = FboHelper::create2D(mpDefaultFBO->getWidth(), mpDefaultFBO->getHeight(), gBufDesc);
  gBufState->setFbo(gBufFbo);
  auto gBufPass = GraphicsProgram::createFromFile("", "GBufferPass.ps.hlsl");
  gBufState->setProgram(gBufPass);
  mGBuffer.pState = gBufState; 
  mGBuffer.pVars = GraphicsVars::create(gBufPass->getActiveVersion()->getReflector());
  
  //Debug
  auto debugState = GraphicsState::create();
  mDebugControls.pDebugPass = FullScreenPass::create("DebugMaps.ps.hlsl");
  mDebugControls.pVars = GraphicsVars::create(mDebugControls.pDebugPass->getProgram()->getActiveVersion()->getReflector());

  loadScene(skDefaultScene);
}

void NprSample::onFrameRender()
{
	const glm::vec4 clearColor(0.38f, 0.52f, 0.10f, 1);
 	mpRenderContext->clearFbo(mpDefaultFBO.get(), clearColor, 1.0f, 0, FboAttachmentType::All);
  mpRenderContext->clearFbo(mGBuffer.pState->getFbo().get(), clearColor, 1.0f, 0, FboAttachmentType::All);

  if(mpSceneRenderer)
  {
    mpSceneRenderer->update(mCurrentTime);

    //GBuf Pass
    mpRenderContext->pushGraphicsState(mGBuffer.pState);
    mpRenderContext->pushGraphicsVars(mGBuffer.pVars);
    mpSceneRenderer->renderScene(mpRenderContext.get());
    mpRenderContext->popGraphicsVars();
    mpRenderContext->popGraphicsState();

    //Color Pass
    if(mDebugControls.mode == None)
    {
      mColorPass.pState->setFbo(mpDefaultFBO);
      mpRenderContext->pushGraphicsState(mColorPass.pState);
      mpRenderContext->pushGraphicsVars(mColorPass.pVars);
      mpSceneRenderer->renderScene(mpRenderContext.get());
      mpRenderContext->popGraphicsVars();
      mpRenderContext->popGraphicsState();
    }
    //Debugging
    else
    {
      if(mDebugControls.mode == Depth)
      {
        mDebugControls.pVars->getConstantBuffer("PerFrame")->setBlob(&mDebugControls.depthMin, 0, 2 * sizeof(float));
        mDebugControls.pVars->setTexture("gDebugTex", mGBuffer.pState->getFbo()->getDepthStencilTexture());
      }
      else if(mDebugControls.mode == Normal)
      {
        mDebugControls.pVars->setTexture("gDebugTex", mGBuffer.pState->getFbo()->getColorTexture(0));
      }

      mpRenderContext->pushGraphicsVars(mDebugControls.pVars);
      mDebugControls.pDebugPass->execute(mpRenderContext.get());
      mpRenderContext->popGraphicsVars();
    }
  }
}

void NprSample::onShutdown()
{

}

bool NprSample::onKeyEvent(const KeyboardEvent& keyEvent)
{
  if(mpSceneRenderer)
    return mpSceneRenderer->onKeyEvent(keyEvent);
  else
    return false;
}

bool NprSample::onMouseEvent(const MouseEvent& mouseEvent)
{
  if(mpSceneRenderer)
    return mpSceneRenderer->onMouseEvent(mouseEvent);
  else 
    return false;
}

void NprSample::onDataReload()
{

}

void NprSample::onResizeSwapChain()
{
  if(mpScene)
  {
    float width = (float)mpDefaultFBO->getWidth();
    float height = (float)mpDefaultFBO->getHeight();
    auto pCam = mpScene->getActiveCamera();
    pCam->setFocalLength(21.0f);
    pCam->setAspectRatio(width / height);
  }
}

void NprSample::loadScene(std::string filename)
{
  mpScene = Scene::loadFromFile(filename);
  mpScene->getActiveCamera()->setDepthRange(0.01f, 100.0f);
  mpSceneRenderer = SceneRenderer::create(mpScene);
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
    NprSample sample;
    SampleConfig config;
    config.windowDesc.title = "Falcor Project Template";
    config.windowDesc.resizableWindow = true;
    sample.run(config);
    return 0;
}
