/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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
/// \date   December 2012

#ifndef SPIRE_HIGH_SHADERMAN_H
#define SPIRE_HIGH_SHADERMAN_H

#include "Common.h"
#include "BaseAssetMan.h"

namespace Spire {

/// Shader asset.
class ShaderAsset : public BaseAsset
{
public:
  ShaderAsset(const std::string& name, GLenum shaderType);
  virtual ~ShaderAsset();

  bool isValid() const          {return mHasValidShader;}
  GLuint getShaderID() const    {return glID;}

protected:

  GLuint            glID;		          ///< Shader ID.
  bool              mHasValidShader;  ///< True if we have a valid shader ID.
};

/// Shader manager.
class ShaderMan : public BaseAssetMan
{
public:
  ShaderMan()           {}
  virtual ~ShaderMan()  {}

  /// Loads and returns a shader asset. If the shader is already loaded,
  /// a reference to that shader is returned instead of reloading it.
  std::shared_ptr<ShaderAsset> loadShader(const std::string& shaderFile,
                                          GLenum shaderType);

  /// This class implements a *default* hold time for all assets.
  /// Typically when compiling / linking a shader program, the shaders are
  /// no longer needed after the compile / link process. As such, 
  /// ShaderProgramMan does not keep shared_ptr references to each of the
  /// shader assets hanging around. This default time will prevent the system
  /// from constantly freeing and reloading the same shaders.
  static const std::chrono::milliseconds getDefaultHoldTime() 
  {
    return std::chrono::milliseconds(50);
  }

private:
  
};

} // namespace Spire

#endif 
