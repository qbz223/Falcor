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

namespace Falcor
{
	/** Vignette Effect.
	*/
	class Vignette
	{

	public:
		
		using UniquePtr = std::unique_ptr<Vignette>;

		/** Create a new Vignette effect.
		*/
		static UniquePtr create();

		/** Default Vignette Destructor.
		*/
		~Vignette();

		/** Set UI elements into a give GUI and UI group
		*/
		void renderUI(Gui* pGui, const char* uiGroup = nullptr);

		/** Run the Vignette program
		\param pRenderContext Render-context to use
		\param pSrc The source FBO
		\param pDst The destination FBO
		*/
		void execute(RenderContext* pRenderContext, Fbo::SharedPtr pSrc, Fbo::SharedPtr pDst);


		/**
			Set the Vignette Center.
		*/
		void setVignetteCenter(glm::vec2 newVignetteCenter);

		/**
			Get the Vignette Center.
		*/
		glm::vec2 getVignetteCenter();

		/**
			Set the Vignette Aspect Ratio.
		*/
		void setVignetteAspectRatio(glm::vec2 newVignetteAspectRatio);

		/**
			Get the Vignette Aspect Ratio.
		*/
		glm::vec2 getVignetteAspectRatio();

		/**
			Set the Vignette Radius.
		*/
		void setVignetteRadius(float newRadius);

		/**
			Get the Vignette Radius.
		*/
		float getVignetteRadius();

		/**
			Set the Vignette Smooth Step.		
		*/
		void setVignetteSmoothStep(float newSmoothStepSize);

		/**
			Get the Vignette Smooth Step.
		*/
		float getVignetteSmoothStep();
		

		/**
			Set the Vignette Color.
		*/
		void setVignetteColor(glm::vec3 vignetteColor);

		/**
			Get the Vignette Color.
		*/
		glm::vec3 getVignetteColor();


		/**
			Set the Vignette Strength.
		*/
		void setVignetteStrength(float newStrength);

		/**
			Get the Vignette Strength.
		*/
		float getVignetteStrength();


	private:

		/** Default Vignette Constructor.
		*/
		Vignette();


		/** The Fullscreen Pass for Vignette.
		*/
		FullScreenPass::UniquePtr mpVignettePass;
		
		/** The Constant Buffer for the Vignette data.
		*/
		ConstantBuffer::SharedPtr mpVignetteData;

		/** The Graphics Vars.
		*/
		GraphicsVars::SharedPtr mpVignetteVars;

		/** The Sampler.
		*/
		Sampler::SharedPtr mpLinearSampler;

		/** Vignette Center.
		*/
		glm::vec2 mVignetteCenter;

		/** Vignette Aspect Ratio.
		*/
		glm::vec2 mVignetteAspectRatio;


		/** Vignette Radius.
		*/
		float mVignetteRadius;

		/** Vignette SmoothStep.
		*/
		float mVignetteSmoothStep;

		/** Vignette Color.
		*/
		glm::vec3 mVignetteColor;

		/** Vignette Strength
		*/
		float mVignetteStrength;

	};

}
