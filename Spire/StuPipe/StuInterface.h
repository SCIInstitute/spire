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
#include <list>
#include <string>
#include <unordered_map>
#include "../Core/PipeInterface.h"
#include "../Core/ShaderUniformStateManTemplates.h"
#include "../Exceptions.h"

namespace Spire {

class Hub;
class StuObject;
class ShaderProgramAsset;

/// \todo For other pipes. If we have access to OpenGL4.2 features, use image
///       load store to implement Order Independent Transparency using
///       per-pixel linked lists. Requires atomic operations in the GPU.
///       Compositing will become very important in this renderer. We can only
///       guarantee a number of fragments in the shader.

/// \todo Before implementing an OIT renderer, a compositing renderer needs to
///       be built first. We can sort objects in a backt-to-front manner and
///       render them appropriately.

/// First pass, incredibly stupid pipe interface.
/// No frame buffer management or advanced rendering (although this kind of
/// advanced rendering would be more suited towards a programmable module that
/// you hook into spire). This pipe was only created for ease-of-use.
class StuInterface : public PipeInterface
{
public:
  StuInterface(Interface& iface);
  virtual ~StuInterface();
  
  /// Initialization as performed on the renderer thread.
  void initOnRenderThread() override;

  /// The different depths supported by the IBO.
  enum IBO_TYPE
  {
    IBO_8BIT,
    IBO_16BIT,
    IBO_32BIT,
  };

  /// Generally not needed at this stage.
  /// \todo Add supported OpenGL version to spire. This will allow us to 
  ///       determine what shaders we should us.
  enum SHADER_TYPES
  {
    // Programmable pipeline
    VERTEX_SHADER,          // Supported as of OpenGL 2.0
    TESSELATION_CONTROL,    // Supported as of OpenGL 4.0
    TESSELATION_EVALUATION, // Supported as of OpenGL 4.0
    GEOMETRY_SHADER,        // Supported as of OpenGL 3.2
    FRAGMENT_SHADER,        // Supported as of OpenGL 2.0

    // Abstract
    COMPUTE_SHADER,         // Supported as of OpenGL 4.3
  };

  //============================================================================
  // THREAD SAFE - Remember, the same thread should always be calling spire.
  //============================================================================
  // Most of these interfaces are implemented through the use of mutex

  //---------
  // Objects
  //---------

  /// Adds a renderable 'object' to the scene.
  void addObject(const std::string& object);

  /// Adds a VBO. This VBO can be re-used by adding passes to the object.
  /// Throws an std::out_of_range exception if the object is not found in the 
  /// system.
  /// \param  object        Name of the object to add the VBO to. The VBO will
  ///                       be destroyed when the object is removed. If you want
  ///                       to reuse VBO / IBOs add passes to this object.
  /// \param  name          Name of the VBO. See addIBOToObject for a full
  ///                       description of why you are required to name your
  ///                       VBO.
  /// \param  vboData       VBO data. This pointer will NOT be stored inside of
  ///                       spire. Unless there is a reference to it out side
  ///                       of spire, it will be destroyed.
  /// \param  attribNames   List of attribute names. This is used as a sanity
  ///                       check to ensure that the shader program's expected
  ///                       attributes match up with what you have provided in
  ///                       in the VBO. This only checked when a call to
  ///                       addGeomPassToObject is made.
  void addVBOToObject(const std::string& object,
                      const std::string& name,
                      std::shared_ptr<std::vector<uint8_t>> vboData,
                      const std::vector<std::string>& attribNames);

  /// Adds an IBO. This IBO can be re-used by adding passes to the object.
  /// Throws an std::out_of_range exception if the object is not found in the 
  /// system.
  /// \param  object        Name of the object to add the VBO to. The VBO will
  ///                       be destroyed when the object is removed. If you want
  ///                       to reuse VBO / IBOs add passes to this object.
  /// \param  name          Name of the IBO. You might find it odd that you are
  ///                       naming an IBO, and in certain terms you are right.
  ///                       IBOs are named here to avoid returning an identifier
  ///                       to the IBO. This would require a mutex lock (at
  ///                       the very least) *and* code would need to be run on
  ///                       the rendering thread since that is where the OpenGL
  ///                       context is current.
  /// \param  iboData       IBO data. This pointer will NOT be stored inside of
  ///                       spire. Unless there is a reference to it out side
  ///                       of spire, it will be destroyed.
  /// \param  type          Specifies what kind of IBO iboData represents.
  void addIBOToObject(const std::string& object,
                      const std::string& name,
                      std::shared_ptr<std::vector<uint8_t>> iboData,
                      IBO_TYPE type);

  /// Completely removes 'object' from the pipe. This includes removing all of
  /// the object's passes as well.
  /// Throws an std::out_of_range exception if the object is not found in the 
  /// system.
  void removeObject(const std::string& object);

  /// Adds a geometry pass to an object given by the identifier 'object'.
  /// Throws an std::out_of_range exception if the object is not found in the 
  /// system. If there already exists a geometry pass, it throws a 'Duplicate' 
  /// exception.
  /// \param  object        Unique object name.
  /// \param  pass          Pass name.
  /// \param  program       Complete shader program to use when rendering.
  ///                       See the oveloaded addPersistentShader functions.
  /// \param  vboID         VBO to use.
  /// \param  iboID         IBO to use.
  /// \return Pass ID. Use this ID to assign uniforms to the pass.
  void addGeomPassToObject(const std::string& object,
                           const std::string& pass,
                           const std::string& program,
                           size_t vboID,
                           size_t iboID);

  /// Removes a geometry pass from the object.
  /// Throws an std::out_of_range exception if the object or pass is not found 
  /// in the system. 
  /// \param  object        Unique object name.
  /// \param  pass          Pass name.
  void removeGeomPassFromObject(const std::string& object,
                                const std::string& pass);

  /// Associates a uniform value to the specified object's pass.
  /// During rendering the uniform value will be returned to its default value 
  /// once this pass has been completed.
  /// Throws an std::out_of_range exception if the object or pass is not found 
  /// in the system.
  template <typename T>
  void addPassUniform(const std::string& object,
                      const std::string& pass,
                      const std::string& uniformName,
                      T uniformData)
  {
    addPassUniformInternal(object, pass, uniformName, 
                           std::unique_ptr<AbstractUniformStateItem>(
                               UniformStateItem<T>(uniformData)));
  }

  //----------
  // Uniforms
  //----------
  
  
  //-----------------
  // Shader Programs
  //-----------------

  /// NOTE: It is possible to have shader programs implemented inside of the
  ///       pipe itself. That way, it's easier to detect OpenGL version problems
  ///       and other things.

  /// Adds a persistent shader under the name 'programName'. Default vertex and
  /// fragment shader.
  void addPersistentShader(const std::string& programName,
                           const std::string& vertexShader,
                           const std::string& fragmentShader);

  /// You can build any shader program you want using this method.
  /// Before you use this function, ensure that the necessary features are
  /// supported by the user's graphics card.
  /// \param shaders  First tuple argument is the shader program file, the 
  ///                 second is the type of shader.
  void addPersistentShader(const std::string& programName,
                           const std::vector<std::tuple<std::string, SHADER_TYPES>>& shaders);

  //============================================================================
  // NOT THREAD SAFE
  //============================================================================
  // Do not call any of these functions, they will be called automatically
  // from within spire.

  void doPass() override;

private:

  void addPassUniformInternal(const std::string& object,
                              const std::string& pass,
                              const std::string& uniformName,
                              std::unique_ptr<AbstractUniformStateItem>&& item);

  /// Object map.
  std::unordered_map<std::string, StuObject>      mObjects;
  std::list<std::shared_ptr<ShaderProgramAsset>>  mPersistentShaders;
};

} // namespace Spire

#endif 
