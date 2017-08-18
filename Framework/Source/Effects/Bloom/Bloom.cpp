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
#include "Bloom.h"
#include "API/RenderContext.h"
#include "Graphics/FboHelper.h"

namespace Falcor
{
	static const char* kBloomExtractionShaderFilename = "Effects/BloomExtraction.ps.slang";
	static const char* kBloomBlendShaderFilename = "Effects/BloomBlend.ps.slang";

	Bloom::~Bloom() = default;

	//	Create the Bloom pass.
	Bloom::UniquePtr Bloom::create()
	{
		Bloom * bloom = new Bloom();
		return Bloom::UniquePtr(bloom);
	}

	//	Render the UI.
	void Bloom::renderUI(Gui * pGui, const char * uiGroup)
	{
		if ((!uiGroup) || pGui->beginGroup(uiGroup))
		{
			//	
			pGui->addFloatVar("Bloom Threshold Value", mBloomThreshold, 0.0);

			//	
			pGui->addFloatVar("Bloom Strength Value", mBloomStrength, 0.0);

			//
			mpGaussianBlurPass->renderUI(pGui, "Bloom Gaussian Blur");

			//	
			if (uiGroup)
			{
				pGui->endGroup();
			}

		}


	}

	//	Execute the Bloom pass.
	void Bloom::execute(RenderContext * pRenderContext, Fbo::SharedPtr pSrc, Fbo::SharedPtr pDst)
	{
		updateExtractionFbo(pSrc);
		updateGaussianBlurFbo(pSrc);

		// Execute the Extraction Pass.
		mpBloomExtractionVars->setTexture("gSourceTexture", pSrc->getColorTexture(0));
		mpBloomExtractionVars->setSampler("gSourceSampler", mpSampler);
		mpBloomExtractionVars->getConstantBuffer("BloomThresholdData")->setVariable("bloomThresholdValue", mBloomThreshold);

		//	Set the Fbo for the Bloom Extraction Pass.
		pRenderContext->setGraphicsVars(mpBloomExtractionVars);
		pRenderContext->getGraphicsState()->setFbo(mpBloomExtractionFbo);
		mpBloomExtractionPass->execute(pRenderContext);

		//	Execute the Gaussian Blur Pass.
		pRenderContext->getGraphicsState()->setFbo(pDst);
		mpGaussianBlurPass->execute(pRenderContext, mpBloomExtractionFbo->getColorTexture(0), mpGaussianBlurOutputFbo);

		//	Execute the Bloom Blend Pass.
		mpBloomBlendVars->setTexture("gBaseTexture", pSrc->getColorTexture(0));
		mpBloomBlendVars->setTexture("gBloomTexture", mpGaussianBlurOutputFbo->getColorTexture(0));
		mpBloomBlendVars->setSampler("gSourceSampler", mpSampler);
		mpBloomBlendVars->getConstantBuffer("BloomStrengthData")->setVariable("bloomStrengthValue", mBloomStrength);

		//	Set the Fbo for the Bloom Blend Pass.
		pRenderContext->setGraphicsVars(mpBloomBlendVars);
		pRenderContext->getGraphicsState()->setFbo(pDst);
		mpBloomBlendPass->execute(pRenderContext);

	}

	//	Return the Bloom Threshold.
	float Bloom::getBloomThreshold()
	{
		return mBloomThreshold;
	}
	
	//	Set the Bloom Threshold.
	void Bloom::setBloomThreshold(float newBloomThreshold)
	{
		mBloomThreshold = newBloomThreshold;
	}

	//	Return the Bloom Strength.
	float Bloom::getBloomStrength()
	{
		return mBloomStrength;
	}

	//	Set the Bloom Strength.
	void Bloom::setBloomStrength(float newStrength)
	{
		mBloomStrength = newStrength;
	}
	
	//	Return the Gaussian Blur Kernel.
	GaussianBlur * Bloom::getGaussianBlurKernel()
	{
		return mpGaussianBlurPass.get();
	}

	// Bloom Constructor.
	Bloom::Bloom()
	{
		//	Set up the Bloom Extraction Pass.
		mpBloomExtractionPass = FullScreenPass::create(kBloomExtractionShaderFilename);
		mpBloomExtractionVars = GraphicsVars::create(mpBloomExtractionPass->getProgram()->getActiveVersion()->getReflector());
		
		//	Set the Bloom Threshold.
		mBloomThreshold = 0.80f;

		//	Create the Gaussian Blur Pass.
		mpGaussianBlurPass = GaussianBlur::create(5u, 20.0f);

		mBloomStrength = 0.0;
		//	Set up the Bloom Blend Pass.
		mpBloomBlendPass = FullScreenPass::create(kBloomBlendShaderFilename);
		mpBloomBlendVars = GraphicsVars::create(mpBloomBlendPass->getProgram()->getActiveVersion()->getReflector());

		//	Set up the Sampler.
		Sampler::Desc samplerDesc;
		samplerDesc.setFilterMode(Sampler::Filter::Linear, Sampler::Filter::Linear, Sampler::Filter::Linear);
		mpSampler = Sampler::create(samplerDesc);



	}


	//	Update the Extraction Fbo.
	void Bloom::updateExtractionFbo(Fbo::SharedPtr pSrcFbo)
	{
		//	Check if we need to recreate the Extraction Fbo.
		bool createExtractionFbo = mpBloomExtractionFbo == nullptr;

		ResourceFormat srcFormat = pSrcFbo->getColorTexture(0)->getFormat();
		uint32_t bytesPerChannel = getFormatBytesPerBlock(srcFormat) / getFormatChannelCount(srcFormat);

		ResourceFormat newFormat = (bytesPerChannel == 32) ? ResourceFormat::RGBA32Float : ResourceFormat::RGBA16Float;
		uint32_t requiredHeight = getLowerPowerOf2(pSrcFbo->getHeight());
		uint32_t requiredWidth = getLowerPowerOf2(pSrcFbo->getWidth());

		//	
		if (createExtractionFbo == false)
		{
			//	Check for the width and height change.
			bool widthChange = (requiredWidth != mpBloomExtractionFbo->getWidth());
			bool heightChange = (requiredHeight != mpBloomExtractionFbo->getHeight());

			//	Check for the format change.
			bool formatChange = (newFormat != mpBloomExtractionFbo->getColorTexture(0)->getFormat());

			//	Changes.
			createExtractionFbo = widthChange || heightChange || formatChange;
		}

		//	Create the Extraction Fbo.
		if (createExtractionFbo)
		{
			Fbo::Desc desc;
			desc.setColorTarget(0, newFormat);
			mpBloomExtractionFbo = FboHelper::create2D(requiredWidth, requiredHeight, desc);
		}

	}

	//	Update the Gaussian Blur Fbo.
	void Bloom::updateGaussianBlurFbo(Fbo::SharedPtr pSrcFbo)
	{
		//	Create the Gaussian Blur Fbo.
		bool createGaussianBlurFbo = mpGaussianBlurOutputFbo == nullptr;

		ResourceFormat srcFormat = pSrcFbo->getColorTexture(0)->getFormat();
		uint32_t bytesPerChannel = getFormatBytesPerBlock(srcFormat) / getFormatChannelCount(srcFormat);

		ResourceFormat newFormat = (bytesPerChannel == 32) ? ResourceFormat::RGBA32Float : ResourceFormat::RGBA16Float;
		uint32_t requiredHeight = getLowerPowerOf2(pSrcFbo->getHeight());
		uint32_t requiredWidth = getLowerPowerOf2(pSrcFbo->getWidth());

		//	
		if (createGaussianBlurFbo == false)
		{
			//	Check for the width and height change.
			bool widthChange = (requiredWidth != mpBloomExtractionFbo->getWidth());
			bool heightChange = (requiredHeight != mpBloomExtractionFbo->getHeight());
			
			//	Check for the format change.
			bool formatChange = (newFormat != mpBloomExtractionFbo->getColorTexture(0)->getFormat());

			//	Changes.
			createGaussianBlurFbo = widthChange || heightChange || formatChange;
		}

		//	Create the Gaussian Blur Fbo.
		if (createGaussianBlurFbo)
		{
			Fbo::Desc desc;
			desc.setColorTarget(0, newFormat);
			mpGaussianBlurOutputFbo = FboHelper::create2D(requiredWidth, requiredHeight, desc);
		}
	}


	//	
	
};

