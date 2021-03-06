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

namespace Falcor
{
class NprSceneRenderer : public SceneRenderer
{
  public:
    using UniquePtr = std::unique_ptr<NprSceneRenderer>;
    static UniquePtr create(Scene::SharedPtr pScene)
    {
      return UniquePtr(new NprSceneRenderer(pScene));
    }

    void renderScene(RenderContext* pCtx) override
    {
      pCtx->getGraphicsState()->setRasterizerState(nullptr);
      mpLastRs = nullptr;
      SceneRenderer::renderScene(pCtx);
    }

    //Its handling RS itself to deal with alpha
    void enableCulling(bool enable) { mShouldCull = enable; }

  private:
    bool mShouldCull = true;

    NprSceneRenderer(Scene::SharedPtr pScene) : SceneRenderer(pScene)
    {
      RasterizerState::Desc desc;
      mpDefaultRs = RasterizerState::create(desc);
      desc.setCullMode(RasterizerState::CullMode::None);
      mpNoCullRS = RasterizerState::create(desc);
    }

    bool setPerMeshInstanceData(const CurrentWorkingData& currentData, const Scene::ModelInstance* pModelInstance, const Model::MeshInstance* pMeshInstance, uint32_t drawInstanceID) override
    {
      auto cb = currentData.pVars->getConstantBuffer("GsPerFrame");
      if(cb)
      {
        auto offset = cb->getVariableOffset("ssCenter");
        auto center = float4(pMeshInstance->getBoundingBox().center, 1.0f);
        auto world = pModelInstance->getTransformMatrix();
        world = world * pMeshInstance->getTransformMatrix();
        auto worldCenter = world * center;
        auto mvp = currentData.pCamera->getViewProjMatrix();
        center = mvp * worldCenter;
        center /= center.w;
        cb->setBlob(&center, offset, sizeof(vec3));
      }

      return SceneRenderer::setPerMeshInstanceData(currentData, pModelInstance, pMeshInstance, drawInstanceID);
    }

    bool setPerMaterialData(const CurrentWorkingData& currentData, const Material* pMaterial) override
    {
      RasterizerState::SharedPtr pRs;
      if(!mShouldCull || pMaterial->getAlphaMap())
      {
        pRs = mpNoCullRS;
      }
      else
      {
        pRs = mpDefaultRs;
      }

      if(pRs != mpLastRs)
      {
        currentData.pContext->getGraphicsState()->setRasterizerState(pRs);
        mpLastRs = pRs;
      }

      return SceneRenderer::setPerMaterialData(currentData, pMaterial);
    }

    RasterizerState::SharedPtr mpLastRs = nullptr;
    RasterizerState::SharedPtr mpNoCullRS;
    RasterizerState::SharedPtr mpDefaultRs;
};
}