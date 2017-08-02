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
#include "Falcor.h"
#include <string>
#include <map>
#include <vector>
#include <set>
#include <random>
#include <algorithm>    // std::random_shuffle
#include <ctime>        // std::time
#include <cstdlib>      // std::rand, std::srand


#include "GLSLDescs.h"

using namespace Falcor;
using namespace GLSLDescs;

class GLSLShaderData
{

public:

    //  Shader Code Data.
    GLSLShaderData();

    //  Shader Code Data with Input and Output Structs.
    GLSLShaderData(std::string inputStructVariableType, std::string outputStructVariableType);

    //  Add Constant Buffer.
    void addUniformBuffer(const UniformBufferData & ubDesc);

    //  Set Textures.
    void setTextures(const std::vector<GLSLResourcesDesc> & textures);

    //  Set SSBOs.
    void setSSBOs(const std::vector<GLSLResourcesDesc> & ssbos);

    //  Set Samplers.
    void setSamplers(const std::vector<GLSLResourcesDesc> & samplers);

    //  Set Images.
    void setImages(const std::vector<GLSLResourcesDesc> & images);

    //  Add an Input Variable.
    void addInputVariable(const GLSLResourcesDesc & shaderResourceDesc);

    //  Add an Output Variable.
    void addOutputVariable(const GLSLResourcesDesc & shaderResourceDesc);

    //  Use an Input Variable.
    void useInputVariable(const std::string & useVar, const std::string & inputVar);

    //  Access a Variable from the Constant Buffer.
    void accessUniformBuffer(const std::string & outVar, const std::string & cbVar, std::vector<uint32_t> dimensions, const std::string & accessVar);

    //  Access a Texture by load.
    void loadTexture(const std::string & outVar, const std::string & textureVar, const std::string & loadTextureAccess);

    //  Access a Texture by sample.
    void sampleTexture(const std::string & outVar, const std::string & textureVar, const std::string & samplerVar, const std::string & sampleTextureAccess);

    //  Access a Texture from an Array by load.
    void loadTextureFromArray(const std::string & outVar, const std::string & arrayOfTexturesVar, const std::string & arrayAccess, const std::string & loadTextureAccess);

    //  Access a Texture from an Array by sample.
    void sampleTextureFromArray(const std::string & outVar, const std::string & arrayOfTexturesVar, const std::string & arrayAccess, const std::string & samplerAccessVariable, const std::string & samplerAccessLocation);

    //  Define a Variable for the local scope.
    void defineVariable(const std::string & newVariableType, const std::string & outVariable);

    //  Run a quick operation.
    void executeOperate(const std::string & outVariable, const std::string & varleft, const std::string & varright, const std::string & op);

    //  Set Output Variable
    void setOutputVariable(const std::string & outVariable, const std::string & inVariable);

    //  Return the Shader Input Struct Type.
    std::string getShaderInputStructType() const;

    //  Return the Shader Output Struct Type.
    std::string getShaderOutputStructType() const;

    //  Get Constant Buffers.
    const std::vector<UniformBufferData> & getUniformBuffers() const;

    //  Get Shader Resources.
    const std::vector<GLSLResourcesDesc> & getShaderResources() const;

    //  Return the Code for the use of the inputs.
    const std::vector<std::string> & getUseInputsCode() const;

    //  Create the Code to access the resources.
    const std::vector<std::string> & getAccessResourcesCode() const;

    //  Return the Code to create the Local Variables.
    const std::vector<std::string> & getCreateLocalVariablesCode() const;

    //  Return the Code to Operate the Variables.
    const std::vector<std::string> & getOperateVariablesCode() const;

    //  Return the Code to Write the Outputs.
    const std::vector<std::string> & getWriteOutputsCode() const;

    //  Get the Input Struct Desc.
    const GLSLStructDesc & getInputStructDesc() const;

    //  Get the Output Struct Desc.
    const GLSLStructDesc & getOutputStructDesc() const;

private:


    //  Input Semantics.
    GLSLStructDesc inSemantics;

    //  Output Semantics.
    GLSLStructDesc outSemantics;


    //  Uniform Buffer Data.
    std::vector<UniformBufferData> mUBs;

    //  Screen Space Buffer Objects.
    std::vector<GLSLResourcesDesc> mSSBOs;

    //  Textures.
    std::vector<GLSLResourcesDesc> mTextures;

    //  Samplers.
    std::vector<GLSLResourcesDesc> mSamplers;

    //  Images.
    std::vector<GLSLResourcesDesc> mImages;



    //  Input Variables.
    std::vector<std::string> inputVariables;

    //  Access Variables.
    std::vector<std::string> accessVariables;

    //  Define Variables.
    std::vector<std::string> defineVariables;

    //  Operate Variables.
    std::vector<std::string> operateVariables;

    //  Output Variables.
    std::vector<std::string> outputVariables;

};