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

const Gui::DropdownList NprSample::skEdgeModeList = 
{
  { (int32_t)EdgeMode::Image, "Image" },
  { (int32_t)EdgeMode::Geometry, "Geometry" }
};

const Gui::DropdownList NprSample::skShadingModeList =
{
  { (int32_t)ShadingMode::EdgeOnly, "EdgeOnly" },
  { (int32_t)ShadingMode::Albedo, "Albedo" },
  { (int32_t)ShadingMode::NDotL0, "N dot L0" },
  { (int32_t)ShadingMode::Toon, "Toon" },
  { (int32_t)ShadingMode::Gooch, "Gooch" }
};

const Gui::DropdownList NprSample::skDebugModeList = 
{
  { (int32_t)DebugMode::None, "None" },
  { (int32_t)DebugMode::Depth, "Depth" },
  { (int32_t)DebugMode::Normal, "Normal" },
  { (int32_t)DebugMode::EdgeUv, "EdgeUv" },
  { (int32_t)DebugMode::EdgeU, "EdgeU" },
  { (int32_t)DebugMode::EdgeV, "EdgeV" },
};

const Gui::DropdownList NprSample::skImageOperatorList =
{
  { (int32_t)ImageOperator::Sobel, "Sobel" },
  { (int32_t)ImageOperator::Prewitt, "Prewitt" },
  { (int32_t)ImageOperator::Scharr, "Scharr" }
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

  if(mpGui->beginGroup("Edge Pass"))
  {
    uint32_t uEdgeMode = (uint32_t)mEdgeMode;
    if(mpGui->addDropdown("Mode", skEdgeModeList, uEdgeMode))
    {
      mEdgeMode = (EdgeMode)uEdgeMode;
    }

    if(mEdgeMode == EdgeMode::Image)
    {
      uint32_t uImageOp = (uint32_t)mImagePass.imageOp;
      if(mpGui->addDropdown("Operator", skImageOperatorList, uImageOp))
      {
        mImagePass.imageOp = (ImageOperator)uImageOp;
        auto pProg = mImagePass.pPass->getProgram();
        pProg->clearDefines();
        switch(mImagePass.imageOp)
        {
          case Sobel:
            break;
          case Prewitt:
            pProg->addDefine("_PREWITT");
            break;
          case Scharr:
            pProg->addDefine("_SCHARR");
            break;
          default:
            should_not_get_here();
        }
      }
      mpGui->addFloatVar("Normal Threshold", mImagePassData.normalThreshold, 0);
      float shownDepthThreshold = mImagePassData.depthThreshold / mImagePass.depthScale;
      mpGui->addFloatVar("Depth Threshold", shownDepthThreshold, 0);
      mImagePassData.depthThreshold = shownDepthThreshold * mImagePass.depthScale;
    }
    else
    {
      mpGui->addFloatVar("Edge Length", mGeoEdgePassData.edgeLength);
      mpGui->addFloatVar("Crease Threshold", mGeoEdgePassData.creaseThreshold);
      mpGui->addFloatVar("Facing Bias", mGeoEdgePassData.facingBias);
      mpGui->addFloatVar("Z Bias", mGeoEdgePassData.zBias);
      if(mpGui->addCheckBox("Draw Edge Caps (Broken)", mGeoEdgePass.useEdgeCaps))
      {
        auto pProg = mGeoEdgePass.pState->getProgram();
        if(mGeoEdgePass.useEdgeCaps)
        {
          pProg->addDefine("_USE_EDGE_CAPS");
        }
        else
        {
          pProg->removeDefine("_USE_EDGE_CAPS");
        }
      }
    }

    mpGui->endGroup();
  }

  if(mpGui->beginGroup("Shading"))
  {
    uint32_t iShadingMode = (uint32_t)mShadingMode;
    if(mpGui->addDropdown("Shading Mode", skShadingModeList, iShadingMode))
    {
      mShadingMode = (ShadingMode)iShadingMode;
      auto gBufProg = mGBuffer.pState->getProgram();
      auto geoProg = mGeoEdgePass.pState->getProgram();
      gBufProg->removeDefine("_TOON");
      geoProg->removeDefine("_TOON");
      gBufProg->removeDefine("_DRAW_ALBEDO");
      geoProg->removeDefine("_DRAW_ALBEDO");
      gBufProg->removeDefine("_DRAW_NDOTL");
      geoProg->removeDefine("_DRAW_NDOTL");
      gBufProg->removeDefine("_GOOCH");
      geoProg->removeDefine("_GOOCH");

      switch(mShadingMode)
      {
      case ShadingMode::EdgeOnly:
        break;
      case ShadingMode::Albedo:
        geoProg->addDefine("_DRAW_ALBEDO");
        gBufProg->addDefine("_DRAW_ALBEDO");
        break;
      case ShadingMode::NDotL0:
        geoProg->addDefine("_DRAW_NDOTL");
        gBufProg->addDefine("_DRAW_NDOTL");
        break;
      case ShadingMode::Toon:
        gBufProg->addDefine("_TOON");
        geoProg->addDefine("_TOON");
        break;
      case ShadingMode::Gooch:
        gBufProg->addDefine("_GOOCH");
        geoProg->addDefine("_GOOCH");
      default:
        should_not_get_here();
      }
    }

    switch(mShadingMode)
    {
      case Toon:
      {
        for(uint32_t i = 0; i < skNumThresholds; ++i)
        {
          std::string label ="Toon Threshold " + std::to_string(i);
          float minVal = i == 0 ? 0 : mShadingData.toonThresholds[i - 1];
          float maxVal = i == skNumThresholds - 1 ? 1 : mShadingData.toonThresholds[i + 1];
          mpGui->addFloatVar(label.c_str(), mShadingData.toonThresholds[i], minVal, maxVal);
        }
        uint32_t numScalars = skNumThresholds + 1;
        for(uint32_t i = 0; i < numScalars; ++i)
        {
          std::string label = "Toon Scalar " + std::to_string(i);
          mpGui->addFloatVar(label.c_str(), mShadingData.toonScalars[i], 0, 1);
        }
        break;
      }
      case Gooch:
      {
        mpGui->addRgbColor("Warm Color", mShadingData.warmColor);
        mpGui->addFloatVar("Warm Albedo Mix", mShadingData.warmAlbedoMix, 0, 1);
        mpGui->addRgbColor("Cool Color", mShadingData.coolColor);
        mpGui->addFloatVar("Cool Albedo Mix", mShadingData.coolAlbedoMix, 0, 1);
        break;
      }
    }

    mpGui->endGroup();
  }

  if(mpGui->beginGroup("Debug"))
  {
    uint32_t uDebugMode = (uint32_t)mDebugControls.mode;
    if(mpGui->addDropdown("Debug Mode", skDebugModeList, uDebugMode))
    {
      mDebugControls.mode = (DebugMode)uDebugMode;
      auto pImageProg = mDebugControls.pDebugPass->getProgram();
      auto pGeoProg = mGeoEdgePass.pState->getProgram();
      auto pGbufProg = mGBuffer.pState->getProgram();
      pImageProg->clearDefines();
      pGeoProg->removeDefine("_EDGE_UV");
      pGeoProg->removeDefine("_EDGE_U");
      pGeoProg->removeDefine("_EDGE_V");

      switch(mDebugControls.mode)
      {
        case None:
          break;
        case Depth:
          pImageProg->addDefine("_DEPTH");
          break;
        case Normal:
          pImageProg->addDefine("_NORMAL");
          break;
        case EdgeUv:
          pGeoProg->addDefine("_EDGE_UV");
          break;
        case EdgeU:
          pGeoProg->addDefine("_EDGE_U");
          break;
        case EdgeV:
          pGeoProg->addDefine("_EDGE_V");
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
  auto gBufPass = GraphicsProgram::createFromFile("", "GBufferPass.ps.hlsl");
  gBufState->setProgram(gBufPass);
  mGBuffer.pState = gBufState; 
  createAndSetGBufferFbo();
  mGBuffer.pVars = GraphicsVars::create(gBufPass->getActiveVersion()->getReflector());
  
  //Image Operator
  mImagePass.pPass = FullScreenPass::create("ImageOperatorPass.ps.hlsl");
  mImagePass.pVars = GraphicsVars::create(mImagePass.pPass->getProgram()->getActiveVersion()->getReflector());

  //Geometry Based Edge Detection
  mGeoEdgePass.pState = GraphicsState::create();
  auto geoEdgeProg = GraphicsProgram::createFromFile("", "EdgeRender.ps.hlsl", "EdgeDetect.gs.hlsl", "", "");
  mGeoEdgePass.pState->setProgram(geoEdgeProg);
  mGeoEdgePass.pState->setFbo(mpDefaultFBO);
  mGeoEdgePass.pVars = GraphicsVars::create(geoEdgeProg->getActiveVersion()->getReflector());

  //Debug
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

    if(mDebugControls.mode == Depth || mDebugControls.mode == Normal)
    {
      renderGBuffer();

      if (mDebugControls.mode == Depth)
      {
        mDebugControls.pVars->getConstantBuffer("PerFrame")->setBlob(&mDebugControls.depthMin, 0, 2 * sizeof(float));
        mDebugControls.pVars->setTexture("gDebugTex", mGBuffer.pState->getFbo()->getDepthStencilTexture());
      }
      else
      {
        mDebugControls.pVars->setTexture("gDebugTex", mGBuffer.pState->getFbo()->getColorTexture(0));
      }

      mpRenderContext->pushGraphicsVars(mDebugControls.pVars);
      mDebugControls.pDebugPass->execute(mpRenderContext.get());
      mpRenderContext->popGraphicsVars();
    }
    else if(mDebugControls.mode == EdgeUv || mDebugControls.mode ==  EdgeU ||
            mDebugControls.mode == EdgeV)
    {
      renderGeoEdges();
    }
    //Either normal operation, abledo, or ndotl
    //defines will take care of it 
    else
    {
      if (mEdgeMode == Image)
      {
        renderGBuffer();
        renderImageEdges();
      }
      //Geometry
      else
      {
        renderGeoEdges();
      }
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

    //Recreate gbuffer with proper window dimensions
    createAndSetGBufferFbo();
  }
}

void NprSample::loadScene(std::string filename)
{
  mpScene = Scene::loadFromFile(filename, Model::LoadFlags::GenerateAdjacency | Model::LoadFlags::DontGenerateTangentSpace);
  mpScene->getActiveCamera()->setDepthRange(0.01f, 100.0f);
  mpSceneRenderer = NprSceneRenderer::create(mpScene);
  std::string lightCountStr = std::to_string(mpSceneRenderer->getScene()->getLightCount());
  mGeoEdgePass.pState->getProgram()->addDefine("_LIGHT_COUNT", lightCountStr);
  mGBuffer.pState->getProgram()->addDefine("_LIGHT_COUNT", lightCountStr);
  onResizeSwapChain();
}

void NprSample::createAndSetGBufferFbo()
{
  Fbo::Desc gBufDesc;
  gBufDesc.setColorTarget(0, ResourceFormat::RGBA32Float);
  gBufDesc.setColorTarget(1, ResourceFormat::RGBA32Float);
  gBufDesc.setDepthStencilTarget(ResourceFormat::D32Float);
  auto pFbo = FboHelper::create2D(mpDefaultFBO->getWidth(), mpDefaultFBO->getHeight(), gBufDesc);
  mGBuffer.pState->setFbo(pFbo);
}

void NprSample::renderGBuffer()
{
  mGBuffer.pVars->getConstantBuffer("NprShadingData")->setBlob(&mShadingData, 0, sizeof(NprShadingData));

  mpRenderContext->pushGraphicsState(mGBuffer.pState);
  mpRenderContext->pushGraphicsVars(mGBuffer.pVars);
  mpSceneRenderer->renderScene(mpRenderContext.get());
  mpRenderContext->popGraphicsVars();
  mpRenderContext->popGraphicsState();
}

void NprSample::renderImageEdges()
{
  auto gBuffer = mGBuffer.pState->getFbo();
  auto normalTex = gBuffer->getColorTexture(0);
  auto colorTex = gBuffer->getColorTexture(1);
  auto depthTex = gBuffer->getDepthStencilTexture();

  mImagePassData.textureDimensions.x = mpDefaultFBO->getWidth();
  mImagePassData.textureDimensions.y = mpDefaultFBO->getHeight();
  mImagePass.pVars->getConstantBuffer("PerFrame")->setBlob(&mImagePassData, 0, sizeof(ImageOperatorPassData));
  mImagePass.pVars->setTexture("gDepth", depthTex);
  mImagePass.pVars->setTexture("gNormal", normalTex);
  mImagePass.pVars->setTexture("gColorTex", colorTex);
  mpRenderContext->pushGraphicsVars(mImagePass.pVars);
  mImagePass.pPass->execute(mpRenderContext.get());
  mpRenderContext->popGraphicsVars();
}

void NprSample::renderGeoEdges()
{
  mGeoEdgePass.pVars->getConstantBuffer("GsPerFrame")->setBlob(&mGeoEdgePassData, 0, sizeof(GeometryEdgePass));
  mGeoEdgePass.pVars->getConstantBuffer("NprShadingData")->setBlob(&mShadingData, 0, sizeof(NprShadingData));

  mpSceneRenderer->enableCulling(false);
  mGeoEdgePass.pState->setFbo(mpDefaultFBO);
  mpRenderContext->pushGraphicsState(mGeoEdgePass.pState);
  mpRenderContext->pushGraphicsVars(mGeoEdgePass.pVars);
  mpSceneRenderer->renderScene(mpRenderContext.get());
  mpRenderContext->popGraphicsVars();
  mpRenderContext->popGraphicsState();
  mpSceneRenderer->enableCulling(true);
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
