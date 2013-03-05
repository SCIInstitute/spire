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
/// \date   March 2013

#ifndef SPIRE_APPSPECIFIC_SCIRUN_SRCOMMONUNIFORMS_H
#define SPIRE_APPSPECIFIC_SCIRUN_SRCOMMONUNIFORMS_H

#include <tuple>

#include "../../Core/ShaderUniformStateManTemplates.h"

namespace Spire
{

/// Common uniforms used by SCIRun.
class SRCommonUniforms
{
public:
  SRCommonUniforms()          {}
  virtual ~SRCommonUniforms() {}
  
  //----------------------------------------------------------------------------
  // Camera Uniforms
  //----------------------------------------------------------------------------

  /// Inverse world view - projection matrix.
  /// Used by vertex shaders to transform from object coordinates into 
  /// projected coordinates.
  static std::tuple<const char*, UNIFORM_TYPE> getCameraWorldToProjection()   
  {return std::make_pair("uProjIVWorld", UNIFORM_FLOAT_MAT4);}

  /// Inverse view to world matrix.
  static std::tuple<const char*, UNIFORM_TYPE> getCameraWorldToView()
  {return std::make_pair("uProjIV", UNIFORM_FLOAT_MAT4);}

  /// Projection matrix.
  static std::tuple<const char*, UNIFORM_TYPE> getCameraProjection()
  {return std::make_pair("uProj", UNIFORM_FLOAT_MAT4);}

  /// View to world.
  static std::tuple<const char*, UNIFORM_TYPE> getCameraWorld()
  {return std::make_pair("uView", UNIFORM_FLOAT_MAT4);}

  //----------------------------------------------------------------------------
  // Lighting uniforms
  //----------------------------------------------------------------------------

  /// Directional light vector
  static std::tuple<const char*, UNIFORM_TYPE> getLightDir()
  {return std::make_pair("uDirLight", UNIFORM_FLOAT_VEC4);}

  //----------------------------------------------------------------------------
  // Misc. uniforms
  //----------------------------------------------------------------------------

  /// Floating point color vector
  static std::tuple<const char*, UNIFORM_TYPE> getColorFloat()
  {return std::make_pair("uColor", UNIFORM_FLOAT_VEC4);}

};

} // namespace Spire

#endif 
