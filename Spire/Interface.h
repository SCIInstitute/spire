/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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
/// \date   September 2012

#ifndef SPIRE_INTERFACE_H
#define SPIRE_INTERFACE_H

#include <string>
#include <vector>
#include <list>
#include <functional>
#include <memory>

#include "Context.h"

#include "Core/WinDLLExport.h"
#include "Core/Math.h"  // Necessary in order to communicate vector types.
#include "Core/ShaderUniformStateManTemplates.h"
#include "Core/GPUStateManager.h"

/// \todo The following *really* wants to be a constexpr inside of StuInterface,
/// when we upgrade to VS 2012, we should also upgrade this.
#define SPIRE_DEFAULT_PASS "spire_default"

namespace Spire {

class Hub;
class HubThread;
class LambdaInterface;
class ObjectLambdaInterface;
class SpireObject;

/// Interface to the renderer.
/// A new interface will need to be created per-context.
/// Spire expects that only one thread will be communicating with it at any
/// given time.
class WIN_DLL Interface
{
public:
  /// All possible log outputs from the renderer. Used via the logging function.
  enum LOG_LEVEL
  {
    LOG_DEBUG,    ///< Debug / verbose.
    LOG_MESSAGE,  ///< General message.
    LOG_WARNING,  ///< Warning.
    LOG_ERROR,    ///< Error.
  };

  typedef std::function<void (const std::string&, Interface::LOG_LEVEL level)> 
      LogFunction;

  /// Constructs an interface to the renderer.
  /// \param  shaderDirs    A list of directories to search for shader files.
  /// \param  createThread  If true, then a thread will be created in which the
  ///                       renderer will run. The renderer will do it's best
  ///                       to manage time allocation and only use cycles
  ///                       when there is something to do.
  /// \param  logFP         The logging function to use when logging rendering
  ///                       messages. Remember, this function will be called
  ///                       from the rendering thread; so it will need to 
  ///                       communicate the messages in a thread-safe manner.
  /// @todo Re-enabled shared_ptr context when SCIRun5 adopts C++11.
  //Interface(std::shared_ptr<Context> context, bool createThread, 
  //          LogFunction logFP = LogFunction());
  Interface(std::shared_ptr<Context> context, 
            const std::vector<std::string>& shaderDirs,
            bool createThread, LogFunction logFP = LogFunction());
  virtual ~Interface();


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
  /// \todo See glProvokingVertex for possible flat shading implementations
  ///       instead of using a geometry shader.
  /// \xxx  Place in GPUMan?
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

  /// See: http://www.opengl.org/wiki/Primitive
  /// \todo Add patch tesselation primitives.
  /// \xxx  Place in GPUMan?
  enum PRIMITIVE_TYPES
  {
    POINTS,
    LINES,
    LINE_LOOP,
    LINE_STRIP,
    TRIANGLES,
    TRIANGLE_STRIP,
    TRIANGLE_FAN,
    LINES_ADJACENCY,
    LINE_STRIP_ADJACENCY,
    TRIANGLES_ADJACENCY,
    TRIANGLE_STRIP_ADJACENCY,
  };

  /// Used for setting up incoming shader attributes.
  enum DATA_TYPES
  {
    DATATYPE_BYTE,      ///< GLbyte   - 8-bit signed integer,   C-Type (signed char),   Suffix (b)
    DATATYPE_SHORT,     ///< GLshort  - 16-bit integer,         C-Type (short),         Suffix (s)
    DATATYPE_INT,       ///< GLint    - 32-bit integer          C-Type (long),          Suffix (I)
    DATATYPE_SIZEI,     ///< GLsizei  - 32-bit integer          C-Type (long),          Suffix (I)
    DATATYPE_FLOAT,     ///< GLfloat  - 32-bit floating,        C-Type (float),         Suffix (f)
    DATATYPE_HALFFLOAT, ///< GLfloat  - 16-bit floating,        C-Type (?),             Suffix (?)
    DATATYPE_CLAMPF,    ///< GLclampf - 32-bit floating,        C-Type (float),         Suffix (f)
    DATATYPE_DOUBLE,    ///< GLdouble - 64-bit floating,        C-Type (double),        Suffix (d)
    DATATYPE_CLAMPD,    ///< GLclampd - 64-bit floating,        C-Type (double),        Suffix (d)
    DATATYPE_UBYTE,     ///< GLubyte  - 8-bit unsigned integer, C-Type (unsigned char), Suffix (ub)
    DATATYPE_BOOLEAN,   ///< GLboolean- 8-bit unsigned integer, C-Type (unsigned char), Suffix (ub)
    DATATYPE_USHORT,    ///< GLushort - 16-bit unsigned integer,C-Type (unsigned short),Suffix (us)
    DATATYPE_UINT,      ///< GLuint   - 32-bit unsigned integer,C-Type (unsigned long), Suffix (ui)
    DATATYPE_ENUM,      ///< GLenum   - 32-bit unsigned integer,C-Type (unsigned long), Suffix (ui)
    DATATYPE_BITFIELD,  ///< GLbitfield- 32-bit unsigned integer,C-Type (unsigned long), Suffix (ui)
  };


  //============================================================================
  // THREAD SAFE - Remember, the same thread should always be calling spire.
  //============================================================================

  /// Terminates spire. If running 'threaded' then this will join with the 
  /// spire thread before returning. This should be called before the OpenGL
  /// context is destroyed.
  /// There is no mutex lock in this function, it should only be called by one
  /// thread, but it doesn't matter what thread.
  void terminate();

  //--------
  // Passes
  //--------

  // The default pass (SPIRE_DEFAULT_PASS) is always present.

  /// Adds a pass to the front of the pass list. Passes at the front of the list
  /// are rendered first.
  void addPassToFront(const std::string& passName);

  /// Adds a pass to the back of the pass list. Passes at the back of the list
  /// are rendered last.
  void addPassToBack(const std::string& passName);

  //---------
  // Objects
  //---------

  /// Adds a renderable 'object' to the scene.
  void addObject(const std::string& object);

  /// Completely removes 'object' from the pipe. This includes removing all of
  /// the object's passes as well.
  /// Throws an std::out_of_range exception if the object is not found in the 
  /// system.
  void removeObject(const std::string& object);

  /// Removes all objects from the system.
  void removeAllObjects();


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
  ///                       addPassToObject is made.
  void addVBO(const std::string& name,
              std::shared_ptr<std::vector<uint8_t>> vboData,
              const std::vector<std::string>& attribNames);

  /// Removes specified vbo from the object. It is safe to issue this call even
  /// though some of your passes may still be referencing the VBOs/IBOs. When
  /// the passes are destroyed, their associated VBOs/IBOs will be destroyed.
  void removeVBO(const std::string& vboName);

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
  void addIBO(const std::string& name,
                      std::shared_ptr<std::vector<uint8_t>> iboData,
                      IBO_TYPE type);

  /// Removes specified ibo from the object. It is safe to issue this call even
  /// though some of your passes may still be referencing the VBOs/IBOs. When
  /// the passes are destroyed, their associated VBOs/IBOs will be destroyed.
  void removeIBO(const std::string& iboName);

  /// Loads an asset file and populates the given vectors with vbo and ibo
  /// data. In the future, we should expand this to include other asset types.
  /// Always uses 16bit IBOs and 32bit per component position / normal in the
  /// vbo.
  /// \return Returns the number of triangles read into ibo.
  /// \todo This should be moved into SCI-Run appspecific. It doesn't belong in
  ///       spire.
  static size_t loadProprietarySR5AssetFile(std::istream& stream,
                                            std::vector<uint8_t>& vbo,
                                            std::vector<uint8_t>& ibo);

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
  void addPassToObject(const std::string& object,
                       const std::string& program,
                       const std::string& vboName,
                       const std::string& iboName,
                       PRIMITIVE_TYPES type,
                       const std::string& pass = SPIRE_DEFAULT_PASS);

  /// Removes a pass from the object.
  /// Throws an std::out_of_range exception if the object or pass is not found 
  /// in the system. 
  /// \param  object        Unique object name.
  /// \param  pass          Pass name.
  void removePassFromObject(const std::string& object,
                            const std::string& pass);


  //----------
  // Uniforms
  //----------
  
  /// Associates a uniform value to the specified object's pass. If the uniform
  /// already exists, then its value will be updated if it passes a type check.
  /// During rendering the uniform value will be returned to its default value 
  /// once this pass has been completed.
  /// Throws an std::out_of_range exception if the object or pass is not found 
  /// in the system.
  /// Throws ShaderUniformTypeError if the types do not match what is stored
  /// in the shader.
  template <typename T>
  void addObjectPassUniform(const std::string& object,
                            const std::string& uniformName,
                            T uniformData,
                            const std::string& pass = SPIRE_DEFAULT_PASS)
  {
    addObjectPassUniformConcrete(object, uniformName, 
                                 std::shared_ptr<AbstractUniformStateItem>(
                                     new UniformStateItem<T>(uniformData)), pass);
  }

  /// Concrete implementation of the above templated function.
  void addObjectPassUniformConcrete(const std::string& object,
                                    const std::string& uniformName,
                                    std::shared_ptr<AbstractUniformStateItem> item,
                                    const std::string& pass = SPIRE_DEFAULT_PASS);

  /// Adds a uniform that will be consumed regardless of the pass. Pass uniforms
  /// take precedence over pass global uniforms.
  template <typename T>
  void addObjectGlobalUniform(const std::string& object,
                              const std::string& uniformName,
                              T uniformData)
  {
    addObjectGlobalUniformConcrete(object, uniformName,
                                   std::shared_ptr<AbstractUniformStateItem>(
                                       new UniformStateItem<T>(uniformData)));
  }

  /// Concrete implementation of the above templated function.
  void addObjectGlobalUniformConcrete(const std::string& object,
                                      const std::string& uniformName,
                                      std::shared_ptr<AbstractUniformStateItem> item);

  /// Will add *or* update the global uniform if it already exsits.
  /// A shader of a given name is only allowed to be one type. If you attempt
  /// to bind different values to a uniform, this function will throw a
  /// ShaderUniformTypeError.
  template <typename T>
  void addGlobalUniform(const std::string& uniformName, T uniformData)
  {
    addGlobalUniformConcrete(uniformName, 
                             std::shared_ptr<AbstractUniformStateItem>(
                                 new UniformStateItem<T>(uniformData)));
  }

  /// Concrete implementation of the above templated function
  void addGlobalUniformConcrete(const std::string& uniformName,
                                std::shared_ptr<AbstractUniformStateItem> item);

  /// GPU state that will be applied directly before the object is rendered.
  /// Note: The default GPU state is consists of the default GPUState 
  ///       constructor.
  void addObjectPassGPUState(const std::string& object,
                             const GPUState& state,
                             const std::string& pass = SPIRE_DEFAULT_PASS);


  //------------------
  // Spire Attributes
  //------------------
  template <typename T>
  void addObjectGlobalSpireAttribute(const std::string& object,
                                     const std::string& attributeName,
                                     T uniformData)
  {
    addObjectGlobalSpireAttributeConcrete(object, attributeName, 
                                          std::shared_ptr<AbstractUniformStateItem>(
                                              new UniformStateItem<T>(uniformData)));
  }

  // Concrete implementation of the above templated function.
  void addObjectGlobalSpireAttributeConcrete(const std::string& object,
                                             const std::string& attributeName,
                                             std::shared_ptr<AbstractUniformStateItem> item);

  template <typename T>
  void addObjectPassSpireAttribute(const std::string& object,
                                   const std::string& attributeName,
                                   T uniformData,
                                   const std::string& passName = SPIRE_DEFAULT_PASS)
  {
    addObjectPassSpireAttributeConcrete(object, attributeName, 
                                        std::shared_ptr<AbstractUniformStateItem>(
                                            new UniformStateItem<T>(uniformData)), passName);
  }

  // Concrete implementation of the above templated function.
  void addObjectPassSpireAttributeConcrete(const std::string& object,
                                           const std::string& attributeName,
                                           std::shared_ptr<AbstractUniformStateItem> item,
                                           const std::string& passName = SPIRE_DEFAULT_PASS);
  
  //-----------------
  // Shader Programs
  //-----------------

  /// NOTE: It is possible to have shader programs implemented inside of the
  ///       pipe itself. That way, it's easier to detect OpenGL version problems
  ///       and other things.

  /// Adds a persistent shader under the name 'programName'. Default vertex and
  /// fragment shader.
  ///
  /// Throws an invalid_argument exception if there already exists a program
  /// by the same name, and the program specified by this function's arguments
  /// does not match the program that already exists.
  /// Throws a Duplicate exception if this shader is already in the persistent
  /// shader list.
  void addPersistentShader(const std::string& programName,
                           const std::string& vertexShader,
                           const std::string& fragmentShader);

  /// You can build any shader program you want using this method.
  /// Before you use this function, ensure that the necessary features are
  /// supported by the user's graphics card.
  ///
  /// Throws an invalid_argument exception if there already exists a program
  /// by the same name, and the program specified by this function's arguments
  /// does not match the program that already exists.
  /// Throws a Duplicate exception if this shader is already in the persistent
  /// shader list.
  ///
  /// \param shaders  First tuple argument is the shader program file, the 
  ///                 second is the type of shader.
  void addPersistentShader(const std::string& programName,
                           const std::vector<std::tuple<std::string, SHADER_TYPES>>& shaders);

  //---------
  // Lambdas
  //---------

  /// \note All lambdas use the push_back semantic. So if you are adding
  /// rendering lambdas, the first lambda you register will be the first one
  /// called when rendering.

  struct UnsatisfiedUniform
  {
    UnsatisfiedUniform(const std::string& name, int location, unsigned int type) :
        uniformName(name),
        uniformType(type),
        shaderLocation(location)
    {}

    std::string                         uniformName;
    unsigned int                        uniformType;    // Should be: GLenum
    int                                 shaderLocation; // Should be: GLint
  };

  // Two types of lambdas to use. One with objects, and one with passes.
  // The name ObjectLambdaFunction is a little deceptive.
  // ObjectLambdaFunctions will be called per-pass.
  typedef std::function<void (LambdaInterface&)> PassLambdaFunction;

  // Lambda function that includes an object as context.
  typedef std::function<void (ObjectLambdaInterface&)> ObjectLambdaFunction;

  /// These functions help satisfy uniforms that need extra attribute data in
  /// order to process. These can be used to remove load from the GPU by
  /// precomputing any number of things.
  typedef std::function<void (ObjectLambdaInterface&, std::list<UnsatisfiedUniform>&)> 
      ObjectUniformLambdaFunction;

  /// The following functions add hooks into the rendering infrastructure.
  /// @{
  void addLambdaBeginAllPasses(const PassLambdaFunction& fp);
  void addLambdaEndAllPasses(const PassLambdaFunction& fp);
  void addLambdaPrePass(const PassLambdaFunction& fp, const std::string& pass = SPIRE_DEFAULT_PASS);
  void addLambdaPostPass(const PassLambdaFunction& fp, const std::string& pass = SPIRE_DEFAULT_PASS);
  /// @}
  
  /// Adds per-object hooks for calculating uniforms and performing rendering.
  /// Both are optional, and you can add as many lambdas as you need.
  /// @{

  /// If an object rendering lambda is found, then normal rendering does not
  /// proceed.
  void addLambdaObjectRender(const std::string& object, const ObjectLambdaFunction& fp, const std::string& pass = SPIRE_DEFAULT_PASS);

  /// Lambda object uniforms are optional and they will not be called if there
  /// are no unsatisfied uniforms found.
  void addLambdaObjectUniforms(const std::string& object, const ObjectUniformLambdaFunction& fp, const std::string& pass = SPIRE_DEFAULT_PASS);

  /// @}

  //============================================================================
  // NOT THREAD SAFE
  //============================================================================

  /// The following functions are *not* thread safe.
  /// Use these functions only when the renderer is not threaded.
  /// @{
  
  /// If anything in the scene has changed, then calling this will render
  /// a new frame and swap the buffers. If the scene was not modified, then this
  /// function does nothing.
  /// You must call this function every time you want a new frame to be rendered
  /// unless you are using the threaded renderer. The threaded renderer will 
  /// call doFrame automatically.
  /// \note You must call doFrame on the same thread where makeCurrent was issued.
  ///       If this is not the same thread where Interface was created, ensure
  ///       a call to context->makeCurrent() is issued before invoking doFrame
  ///       for the first time.
  void ntsDoFrame();

  /// Obtain the current number of objects.
  /// \todo This function nedes to go to the implementation.
  size_t ntsGetNumObjects() const;

  /// Obtain the object associated with 'name'.
  /// throws std::range_error if the object is not found.
  std::shared_ptr<const SpireObject> ntsGetObjectWithName(const std::string& name) const;

  /// Cleans up all GL resources.
  /// Should ONLY be called from the rendering thread.
  /// In our case, this amounts to disposing of all of our objects and VBO/IBOs
  /// and persistent shader objects.
  void ntsClearGLResources();

  /// Returns true if the specified object is in the pass.
  bool ntsIsObjectInPass(const std::string& object, const std::string& pass) const;

  /// Returns true if the pass already exists.
  bool ntsHasPass(const std::string& pass) const;



  /// @}

protected:

  std::unique_ptr<Hub>      mHub;   ///< Rendering hub.

private:

  friend class PipeInterface;       // Technically, only PipeInterface's constructor
                                    // needs friend status. Needed to extract hub
                                    // reference when the pipe is created.
};

} // namespace spire

#endif // SPIRE_INTERFACE_H
