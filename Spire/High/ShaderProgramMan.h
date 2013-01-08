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

#ifndef SPIRE_HIGH_SHADERPROGRAMMAN_H
#define SPIRE_HIGH_SHADERPROGRAMMAN_H

#include "BaseAssetMan.h"
#include "High/ShaderAttributeMan.h"
#include "High/ShaderUniformMan.h"

namespace Spire
{

class ShaderProgramAsset : public BaseAsset
{
public:
  ShaderProgramAsset(Hub& hub,
                     const std::string& name,
                     const std::list<std::tuple<std::string, GLenum>>& shaders);
  virtual ~ShaderProgramAsset();

  GLuint getProgramID() const   {return glProgramID;}

protected:

  bool                      mHasValidProgram; ///< True if glProgramID is valid.
  GLuint                    glProgramID;      ///< GL program ID.

  Hub&                      mHub;             ///< Reference to render hub.

  ShaderAttributeCollection mAttributes;      ///< All program attributes.
  ShaderUniformCollection   mUniforms;        ///< All program uniforms.
};

/// Management of fully linked GL shader programs.
class ShaderProgramMan : public BaseAssetMan
{
public:
  ShaderProgramMan(Hub& hub) : mHub(hub)  {}
  virtual ~ShaderProgramMan()             {}
  
  /// Loads a shader program. Accepts a list of couples 
  // (shader name, shader type) to compile and link together.
  std::shared_ptr<ShaderProgramAsset> loadProgram(
      const std::string& programName,
      const std::list<std::tuple<std::string, GLenum>>& shaders);

private:

  Hub&      mHub;
};

} // namespace Spire

#endif 
