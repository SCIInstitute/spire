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
/// \date   February 2013

#ifndef SPIRE_STUPIPE_STUINTERFACE_H
#define SPIRE_STUPIPE_STUINTERFACE_H

#include <utility>
#include <vector>
#include <array>
#include "Core/PipeInterface.h"

namespace Spire
{

/// First pass, incredibly stupid pipe interface.
class StuInterface : public PipeInterface
{
public:
  StuInterface();
  virtual ~StuInterface();
  
  //============================================================================
  // THREAD SAFE - Remember, the same thread should always be calling spire.
  //============================================================================

  //---------
  // Objects
  //---------
  /// Removes an object given an identifier.
  void removeObject(const std::string& object);

  /// \todo Include attribute specification along with the pass so we can match
  ///       it up with what the shader is expecting. These should all be based
  ///       on names.
  /// Adds a geometry pass to an object given by the identifier 'object'.
  /// \param  object        Unique object name.
  /// \param  passName      Unique name of the pass.
  /// \param  attribBuffer  rvalue reference to the attribute buffer. You are
  ///                       forced to std::move your vector.
  /// \param  program       Complete shader program to use when rendering.
  ///                       See the oveloaded addPersistentShader functions.
  void addGeomPassToObject(const std::string& object,
                           const std::string& passName,
                           std::vector<uint8_t>&& attribBuffer, 
                           const std::vector<std::string>& attribNames,
                           const std::string& program);

  /// Associates a uniform value to the specified object's pass.
  /// The uniform value will be returned to its default value once this pass
  /// has been completed.
  void addPassUniform(const std::string& object,
                      const std::string& passName,
                      const std::string& uniformName);

  //----------
  // Uniforms
  //----------
  
  
  //-----------------
  // Shader Programs
  //-----------------

  /// Adds a persistent shader under the name 'programName'.
  void addPersistentShader(const std::string& programName,
                           const std::string& vertexShader,
                           const std::string& fragmentShader);

  /// \todo Figure out if the platform supports geometry shaders and expose
  ///       this somehow. There should be a fallback shader if the platform
  ///       does not support geometry shaders.
  void addPersistentShader(const std::string& programName,
                           const std::string& vertexShader,
                           const std::string& geometryShader,
                           const std::string& fragmentShader);

  //============================================================================
  // NOT THREAD SAFE
  //============================================================================
  // Do not call any of these functions, they will be called automatically
  // from within spire.

  // Inherited functions
  virtual void doPass();

};

} // namespace Spire

#endif 
