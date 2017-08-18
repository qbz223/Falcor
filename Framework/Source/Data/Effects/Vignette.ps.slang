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

//	Vignette Data.
cbuffer VignetteData
{
	float4 vignetteCenterAndAspectRatio;
	float4 vignetteColorAndStrength;
	float4 vignetteRadiusAndSmoothstep;
};

//	Source Texture.
Texture2D<float4> gSourceTexture;

//	Sampler.
SamplerState gSourceSampler;

//	
float4 main(float2 texC  : TEXCOORD) : SV_TARGET0
{
	//	Get the Sample source.
	float4 source = gSourceTexture.Sample(gSourceSampler, texC);

	//	Compute the length.	
	float len = length((texC - vignetteCenterAndAspectRatio.xy) / vignetteCenterAndAspectRatio.zw);

	//	Compute the Vignette Value.
	float vignetteValue = smoothstep(vignetteRadiusAndSmoothstep.x, vignetteRadiusAndSmoothstep.x - vignetteRadiusAndSmoothstep.y, len);
	
	//	Compute the Base Vignette Color.
	float3 vignetteBaseColor = lerp(vignetteColorAndStrength.rgb, source.rgb, vignetteValue);

	//	Compute the Output Vignette Color.
	float3 vignetteOutputColor = lerp(vignetteBaseColor, source.rgb, vignetteColorAndStrength.w);

	//	Return the color.
	return float4(vignetteOutputColor, source.w);
}
