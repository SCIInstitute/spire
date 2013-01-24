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

#include <fstream>

#include "Common.h"
#include "ShaderMan.h"
#include "Exceptions.h"
#include "High/Log.h"
#include "High/FileUtil.h"

namespace Spire {

//------------------------------------------------------------------------------
std::shared_ptr<ShaderAsset> ShaderMan::loadShader(const std::string& shaderFile,
                                                   GLenum shaderType)
{
  std::shared_ptr<BaseAsset> asset = findAsset(shaderFile);
  if (asset == nullptr)
  {
    // Load a new shader.
    std::shared_ptr<ShaderAsset> shaderAsset(
        new ShaderAsset(mHub, shaderFile, shaderType));

    // Add the asset to BaseAssetMan's internal weak_ptr list.
    asset = std::dynamic_pointer_cast<BaseAsset>(shaderAsset);
    addAsset(asset);
    holdAsset(asset, getDefaultHoldTime());

    return shaderAsset;
  }
  else
  {
    return std::dynamic_pointer_cast<ShaderAsset>(asset);
  }
}

//------------------------------------------------------------------------------
ShaderAsset::ShaderAsset(Hub& hub, const std::string& filename, 
                         GLenum shaderType) :
    BaseAsset(filename),
    mHasValidShader(false),
    mHub(hub)
{
  std::string targetFilename = findFileInDirs(filename, hub.getShaderDirs(),
                                              false);
  const std::vector<std::string> dirs(hub.getShaderDirs());
  std::ifstream file(targetFilename, std::ios_base::in);
  if (file.is_open() == false)
  {
    Log::message() << "Failed to open shader " << filename << std::endl;
    throw NotFound("Failed to find shader.");
  }

  // Size std::string appropriately before reading file.
  std::string fileContents;
  file.seekg(0, std::ios::end);
  fileContents.resize(static_cast<unsigned int>(file.tellg()));
  file.seekg(0, std::ios::beg);

  // Extra parenthesis are essential to avoid the most vexing parse.
  fileContents.assign( (std::istreambuf_iterator<char>(file)), 
                        std::istreambuf_iterator<char>());
  
  // Now compile the shader file.
  GLuint  shader;
  GLint   compiled;

  // Create the shader object.
  shader = glCreateShader(shaderType);
  if (0 == shader)
  {
    Log::message() << "Failed to create shader of type: " << shaderType << "\n";
    throw GLError("Unable to construct shader.");
  }

  const char* cFileContents = fileContents.c_str();
  glShaderSource(shader, 1, &cFileContents, NULL);
  GL_CHECK();

  glCompileShader(shader);

  // Check the compile status
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

  // Check compilation status.
  if (!compiled)
  {
    GLint infoLen = 0;
  
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
    if (infoLen > 1)
    {
      char* infoLog = new char[infoLen];

      glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
      Log::error() << "Error compiling '" << filename << "':" << std::endl << infoLog 
                   << std::endl;

      delete[] infoLog;
    }

    glDeleteShader(shader);

    throw GLError("Failed to compile shader.");
  }

  mHasValidShader = true;
  glID = shader;
}

//------------------------------------------------------------------------------
ShaderAsset::~ShaderAsset()
{
  if (mHasValidShader)  
  {
    glDeleteShader(glID);
    mHasValidShader = false;
  }
}

} 
