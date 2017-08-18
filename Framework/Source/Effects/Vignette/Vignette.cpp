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
#include "Framework.h"
#include "Vignette.h"
#include "API/RenderContext.h"
#include "Graphics/FboHelper.h"

namespace Falcor 
{
	static const char* kVignetteShaderFilename = "Effects\\Vignette.ps.slang";

	//	Create the Vignette Program.
	Vignette::UniquePtr Vignette::create()
	{
		Vignette * vignette = new Vignette();
		return Vignette::UniquePtr(vignette);
	}
	
	//	Set the Default Destructor.
	Vignette::~Vignette() = default;

	//	Add the Vignette UI.
	void Vignette::renderUI(Gui * pGui, const char * uiGroup)
	{
		if ((uiGroup == nullptr) || pGui->beginGroup(uiGroup))
		{
			pGui->addFloat3Var("Vignette Color", mVignetteColor, 0.0, 1.0);
			pGui->addFloatVar("Vignette Radius", mVignetteRadius, 0.0, 1.0);
			pGui->addFloatVar("Vignette SmoothStep", mVignetteSmoothStep, 0.0, 1.0);
			pGui->addFloat2Var("Vignette Center", mVignetteCenter, 0.0, 1.0);
			pGui->addFloat2Var("Vignette Aspect Ratio", mVignetteAspectRatio, 0.0, 1.0);
			pGui->addFloatVar("Vignette Strength", mVignetteStrength, 0.0, 1.0);
		}
	}

	//	Execute the Vignette Program.
	void Vignette::execute(RenderContext * pRenderContext, Fbo::SharedPtr pSrc, Fbo::SharedPtr pDst)
	{
		//	Set the Vignette Data.
		mpVignetteData->setVariable("vignetteCenterAndAspectRatio", glm::vec4(mVignetteCenter, mVignetteAspectRatio));
		mpVignetteData->setVariable("vignetteRadiusAndSmoothstep", glm::vec4(mVignetteRadius, mVignetteSmoothStep, 0.0f, 0.0f));
		mpVignetteData->setVariable("vignetteColorAndStrength", glm::vec4(mVignetteColor, mVignetteStrength));
		
		//	Set the Source Data.
		mpVignetteVars->setTexture("gSourceTexture", pSrc->getColorTexture(0));
		mpVignetteVars->setSampler("gSourceSampler", mpLinearSampler);

		//	Set the Graphhics Vars and State.
		pRenderContext->setGraphicsVars(mpVignetteVars);
		pRenderContext->getGraphicsState()->setFbo(pDst);
		
		//	Execute the Render Context.
		mpVignettePass->execute(pRenderContext);
	}

	void Vignette::setVignetteCenter(glm::vec2 newVignetteCenter)
	{
		mVignetteCenter = newVignetteCenter;
	}

	glm::vec2 Vignette::getVignetteCenter()
	{
		return mVignetteCenter;
	}

	void Vignette::setVignetteAspectRatio(glm::vec2 newVignetteAspectRatio)
	{
		mVignetteAspectRatio = newVignetteAspectRatio;
	}

	glm::vec2 Vignette::getVignetteAspectRatio()
	{
		return mVignetteAspectRatio;
	}

	void Vignette::setVignetteRadius(float newRadius)
	{
		mVignetteRadius = newRadius;
	}

	float Vignette::getVignetteRadius()
	{
		return mVignetteRadius;
	}

	void Vignette::setVignetteSmoothStep(float newSmoothStepSize)
	{
		mVignetteSmoothStep = newSmoothStepSize;
	}

	float Vignette::getVignetteSmoothStep()
	{
		return mVignetteSmoothStep;
	}

	void Vignette::setVignetteColor(glm::vec3 vignetteColor)
	{
		mVignetteColor = vignetteColor;
	}

	glm::vec3 Vignette::getVignetteColor()
	{
		return mVignetteColor;
	}

	void Vignette::setVignetteStrength(float newStrength)
	{
		mVignetteStrength = newStrength;
	}

	float Vignette::getVignetteStrength()
	{
		return mVignetteStrength;
	}


	//	Vignette.
	Vignette::Vignette()
	{
		//	
		mVignetteCenter = glm::vec2(0.5, 0.5);
		mVignetteAspectRatio = glm::vec2(1.0, 16.0 / 9.0);
		mVignetteColor = glm::vec3(0.6, 0.1, 0.1);
		mVignetteStrength = 0.25f;
		mVignetteRadius = 0.40f;
		mVignetteSmoothStep = 0.15f;

		//
		Sampler::Desc samplerDesc;
		samplerDesc.setFilterMode(Sampler::Filter::Linear, Sampler::Filter::Linear, Sampler::Filter::Point);
		mpLinearSampler = Sampler::create(samplerDesc);

		//	Create the Vignette Pass.
		mpVignettePass = FullScreenPass::create(kVignetteShaderFilename);
		mpVignetteVars = GraphicsVars::create(mpVignettePass->getProgram()->getActiveVersion()->getReflector());
		mpVignetteData = mpVignetteVars["VignetteData"];
	}

}
