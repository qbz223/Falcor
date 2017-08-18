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
#include "Graphics/FullScreenPass.h"
#include "API/ConstantBuffer.h"
#include "API/FBO.h"
#include "API/Sampler.h"
#include "Utils/Gui.h"
#include "Effects/Utils/GaussianBlur.h"

namespace Falcor
{
	/** Bloom effect
	*/
	class Bloom
	{
	public:
		using UniquePtr = std::unique_ptr<Bloom>;

		/** Destructor
		*/
		~Bloom();

		/** Create a new object
		*/
		static UniquePtr create();

		/** Set UI elements into a give GUI and UI group
		\param pGui The Gui to Render To.
		\param uiGroup The Gui Group.
		*/
		void renderUI(Gui* pGui, const char* uiGroup = nullptr);

		/** Run the Bloom program
		\param pRenderContext Render-context to use
		\param pSrc The source FBO
		\param pDst The destination FBO
		*/
		void execute(RenderContext* pRenderContext, Fbo::SharedPtr pSrc, Fbo::SharedPtr pDst);

		/** Get the Bloom Threshold.
		*/
		float getBloomThreshold();

		/**	Set the Bloom Threshold.
		\param newBloomThreshold The Bloom Threshold to use.
		*/
		void setBloomThreshold(float newBloomThreshold);
		
		/** Get the Bloom Strength.
		*/
		float getBloomStrength();

		/** Set the Bloom Strength.
		*/
		void setBloomStrength(float newStrength);

		/**	Get the Gaussian Blur Kernel.
		*/
		GaussianBlur * getGaussianBlurKernel();

	private:

		Bloom();

		/** Update the Bloom Extraction Fbo.
		*/
		void updateExtractionFbo(Fbo::SharedPtr pSrcFbo);

		/**	Update the Gaussian Blur Fbo.
		*/
		void updateGaussianBlurFbo(Fbo::SharedPtr pSrcFbo);

		//	Bloom Extraction pass.
		FullScreenPass::UniquePtr mpBloomExtractionPass;

		//	Bloom Extraction Fbo.
		Fbo::SharedPtr mpBloomExtractionFbo;
		
		//	Bloom Extraction Texture.
		Texture::SharedPtr mpBloomExtractionTexture;
		
		//	Bloom Extraction Vars.
		GraphicsVars::SharedPtr mpBloomExtractionVars;

		//	Gaussian Blur Pass for the extracted values.
		GaussianBlur::UniquePtr mpGaussianBlurPass;

		//	Gaussian Blur Output Fbo.
		Fbo::SharedPtr mpGaussianBlurOutputFbo;
		
		//	Sampler.
		Sampler::SharedPtr mpSampler;

		//	Blend the Bloom.
		FullScreenPass::UniquePtr mpBloomBlendPass;

		//	Bloom Blend.
		GraphicsVars::SharedPtr mpBloomBlendVars;

		//	The Bloom Threshold for the Bloom Extraction Pass.
		float mBloomThreshold;

		//	The Bloom Strength for the Bloom Blend Pass.
		float mBloomStrength;
	};
}