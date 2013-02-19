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

namespace Spire {

/// First pass, incredibly stupid pipe interface.
/// No frame buffer management or advanced rendering (although this kind of
/// advanced rendering would be more suited towards a programmable module that
/// you hook into spire). This pipe was only created for ease-of-use.
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

  /// Adds a VBO. This VBO can be re-used by changing
  /// the uniforms or the index buffer. This is essentially the vertex buffer
  /// object.
  /// \param  vboData       VBO data.
  /// \param  attribNames   List of attribute names. This is used as a sanity
  ///                       check to ensure that the shader program's expected
  ///                       attributes match up with what you have provided in
  ///                       in the VBO. This only checked when a call to
  ///                       addGeomPassToObject is made.
  /// \return The VBO identifier if the VBO was successfully created. Otherwise
  ///         an exception is thrown.
  size_t addVBO(std::shared_ptr<std::vector<uint8_t>> vboData,
                const std::vector<std::string>& attribNames);

  /// Adds an IBO. This IBO can be re-used.
  /// \return The IBO identifier if the IBO was successfully created. Otherwise
  ///         an exception is thrown.
  size_t addIBO(std::shared_ptr<std::vector<uint8_t>> iboData);

  /// \todo Include attribute specification along with the pass so we can match
  ///       it up with what the shader is expecting. These should all be based
  ///       on names.
  /// Adds a geometry pass to an object given by the identifier 'object'.
  /// \param  object        Unique object name.
  /// \param  vboID         VBO to use.
  /// \param  iboID         IBO to use.
  /// \param  program       Complete shader program to use when rendering.
  ///                       See the oveloaded addPersistentShader functions.
  void addGeomPassToObject(const std::string& object,
                           const std::string& program,
                           size_t vboID,
                           size_t iboID);

  /// Associates a uniform value to the specified object's pass.
  /// The uniform value will be returned to its default value once this pass
  /// has been completed.
  /// \NOTE This needs to be a templated function, see UniformStateMan.
  void addObjectUniform(const std::string& object,
                        const std::string& uniformName);

  //----------
  // Uniforms
  //----------
  
  
  //-----------------
  // Shader Programs
  //-----------------

  /// NOTE: It is possible to have shader programs implemented inside of the
  ///       pipe itself. That way, it's easier to detect OpenGL version problems
  ///       and other things.

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
