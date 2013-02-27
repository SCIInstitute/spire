/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2013 Scientific Computing and Imaging Institute,
   University of Utah.


   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/

/// \author James Hughes
/// \date   January 2013

#include "Common.h"
#include "Exceptions.h"

#include "Core/Hub.h"
#include "Core/ShaderProgramMan.h"

namespace Spire {

//------------------------------------------------------------------------------
std::shared_ptr<ShaderProgramAsset>
ShaderProgramMan::loadProgram(
    const std::string& programName,
    const std::list<std::tuple<std::string, GLenum>>& shaders)
{
  std::shared_ptr<BaseAsset> asset = findAsset(programName);
  if (asset == nullptr)
  {
    std::shared_ptr<ShaderProgramAsset> program(
        new ShaderProgramAsset(mHub, programName, shaders));

    // Add the asset
    addAsset(std::dynamic_pointer_cast<BaseAsset>(program));

    return program;
  }
  else
  {
    // Check to ensure that 'shaderAsset' all requested shaders -- and that
    // all of the shader names are the same. There should be no directory
    // paths in the shader names, so they should compare exactly.
    std::shared_ptr<ShaderProgramAsset> shaderAsset = 
        std::dynamic_pointer_cast<ShaderProgramAsset>(asset);
    if (shaderAsset->areProgramSignaturesIdentical(shaders) == false)
    {
      throw std::invalid_argument("Shader signatures (shader files and types) "
                                  "should match pre-existing shader.");
    }
    return shaderAsset;
  }
}

//------------------------------------------------------------------------------
ShaderProgramAsset::ShaderProgramAsset(
      Hub& hub, const std::string& name,
      const std::list<std::tuple<std::string, GLenum>>& shaders) :
    BaseAsset(name),
    mHub(hub),
    mAttributes(mHub.getShaderAttributeManager()),
    mUniforms(mHub.getShaderUniformManager())
{
  GLuint program = glCreateProgram();
  if (0 == program)
  {
    Log::error() << "Unable to create GL program using glCreateProgram.\n";
    throw GLError("Unable to create shader program.");
  }

  // Load and attach all shaders.
  for (auto it = shaders.begin(); it != shaders.end(); ++it)
  {
    // Attempt to find shader.
    std::shared_ptr<ShaderAsset> shader = 
        mHub.getShaderManager().loadShader(std::get<0>(*it), std::get<1>(*it));

    glAttachShader(program, shader->getShaderID());
  }

  // Link the program
  glLinkProgram(program);

	// Check the link status 
	GLint linked;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		GLint infoLen = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);

		if (infoLen > 1)
		{
      char* infoLog = new char[infoLen];

			glGetProgramInfoLog(program, infoLen, NULL, infoLog);
      Log::error() << "Error linking program:" << std::endl;
      Log::error() << infoLog << std::endl;

      delete[] infoLog;
		}

		glDeleteProgram(program);

    throw GLError("Failed to link shader.");
	}

  // ATTRIBUTES
  {
    // Check the active attributes.
    GLint activeAttributes;
    glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &activeAttributes);

    const int maxAttribNameSize = 1024;
    char attributeName[maxAttribNameSize];
    for (int i = 0; i < activeAttributes; i++)
    {
      GLsizei charsWritten = 0;
      GLint attribSize;
      GLenum type;
      glGetActiveAttrib(program, i, maxAttribNameSize, &charsWritten,
          &attribSize, &type, attributeName);

      try
      {
        mAttributes.addAttribute(attributeName);
      }
      catch (ShaderAttributeNotFound&)
      {
        Log::error() << "Unable to find attribute: ''" << attributeName << "'"
                     << " in ShaderAttributeMan.\n";
      }
    }
  }

  // Now sync up program attributes
  //mAttributes.bindAttributes(program);

  // UNIFORMS
  {
    // Check the active uniforms.
    GLint activeUniforms;
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &activeUniforms);

    const int maxUniformNameSize = 1024;
    char uniformName[maxUniformNameSize];
    for (int i = 0; i < activeUniforms; i++)
    {
      GLsizei charsWritten = 0;
      GLint uniformSize;
      GLenum type;
      glGetActiveUniform(program, i, maxUniformNameSize, &charsWritten,
          &uniformSize, &type, uniformName);

      try
      {
        mUniforms.addUniform(uniformName);
      }
      catch (ShaderUniformNotFound&)
      {
        Log::warning() << "Unable to find uniform: '" << uniformName << "'"
                       << " in ShaderUniformMan." << std::endl;
      }
    }
  }

  mLoadedShaders    = shaders;
  mHasValidProgram  = true;

  glProgramID       = program;
}

//------------------------------------------------------------------------------
ShaderProgramAsset::~ShaderProgramAsset()
{
  if (mHasValidProgram)
  {
    glDeleteProgram(glProgramID);
    mHasValidProgram = false;
  }
}

//------------------------------------------------------------------------------
bool ShaderProgramAsset::areProgramSignaturesIdentical(
    const std::list<std::tuple<std::string, GLenum>>& shaders)
{
  for (auto it = shaders.begin(); it != shaders.end(); ++it)
  {
    bool foundCorresponding = false;
    for (auto it2 = mLoadedShaders.begin(); it2 != mLoadedShaders.end(); ++it2)
    {
      if (   std::get<0>(*it) == std::get<0>(*it2) 
          && std::get<1>(*it) == std::get<1>(*it2))
      {
        foundCorresponding = true;
        break;
      }
    }

    if (foundCorresponding == false)
      return false;
  }

  return true;
}


} // end of namespace Spire

