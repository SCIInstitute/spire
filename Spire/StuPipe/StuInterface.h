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
#include <map>
#include <tuple>
#include <cstdint>
#include "../InterfaceCommon.h"
#include "../Core/PipeInterface.h"
#include "../Core/ShaderUniformStateManTemplates.h"
#include "../Core/GPUStateManager.h"
#include "../Core/LambdaInterface.h"

#include "StuObjectLambda.h"

namespace Spire {

class Hub;
class StuObject;
class ShaderProgramAsset;
class VBOObject;
class IBOObject;

/// \todo The following *really* wants to be a constexpr inside of StuInterface,
/// when we upgrade to VS 2012, we should also upgrade this.
#define SPIRE_DEFAULT_PASS "spire_default"

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
class WIN_DLL StuInterface : public PipeInterface
{
public:
  StuInterface(Interface& iface);
  virtual ~StuInterface();
  
  /// Initialization as performed on the renderer thread.
  void ntsInitOnRenderThread() override;

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

  //============================================================================
  // THREAD SAFE - Remember, the same thread should always be calling spire.
  //============================================================================

  //--------
  // Passes
  //--------

  // The default pass is always present.

  /// Adds a pass to the front of the pass list. Passes at the front of the list
  /// are rendered first.
  void addPassToFront(const std::string& passName);

  /// Adds a pass to the back of the pass list. Passes at the back of the list
  /// are rendered last.
  void addPassToBack(const std::string& passName);

  //---------
  // Objects
  //---------

  /// \todo Add BVH which only stores object names and is used for frustum
  ///       culling. This BVH should be mutex locked so the UI thread can cast
  ///       rays into it.

  /// Adds a renderable 'object' to the scene.
  void addObject(const std::string& object);

  /// Adds a renderable 'object' to the scene, and assigns it 'renderOrder'.
  /// Objects with lower render orders will be rendered first.
  void addObject(const std::string& object, int32_t renderOrder);

  /// Completely removes 'object' from the pipe. This includes removing all of
  /// the object's passes as well.
  /// Throws an std::out_of_range exception if the object is not found in the 
  /// system.
  void removeObject(const std::string& object);

  /// Removes all objects from the system.
  void removeAllObjects();

  /// Assigns a new rendering order to the object
  /// Throws std::range_error if object is not found.
  void assignRenderOrder(const std::string& object, int32_t renderOrder);


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
  void addPassToObject(const std::string& object,
                       const std::string& program,
                       const std::string& vboName,
                       const std::string& iboName,
                       PRIMITIVE_TYPES type,
                       int32_t passOrder,
                       const std::string& pass = SPIRE_DEFAULT_PASS);

  /// Removes a pass from the object.
  /// Throws an std::out_of_range exception if the object or pass is not found 
  /// in the system. 
  /// \param  object        Unique object name.
  /// \param  pass          Pass name.
  void removePassFromObject(const std::string& object,
                            const std::string& pass);


  /// Associates an object -> world transform with the given object / pass
  /// combination. Default is the identity transformation.
  /// This is the only 'special' case variable associated with passes / objects
  /// other than uniforms. We generally want to concatenate the object -> world
  /// transform with the inverse view transform and projection transform.
  void addObjectTransform(const std::string& object,
                          const M44& transform);

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

  /// GPU state that will be applied directly before the object is rendered.
  /// Note: The default GPU state is consists of the default GPUState 
  ///       constructor.
  void addObjectPassGPUState(const std::string& object,
                             const GPUState& state,
                             const std::string& pass = SPIRE_DEFAULT_PASS);

  /// \todo addPassUniform

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


  /// \todo Create method to add uniforms to the UniformManager (NOT the state
  ///       manager -- uniform manager is the type checker).

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
  // The name StuObjectLambdaFunction is a little deceptive.
  // StuObjectLambdaFunctions will be called per-pass.
  typedef std::function<void (LambdaInterface&)> StuPassLambdaFunction;

  // Lambda function that includes an object as context.
  typedef std::function<void (StuObjectLambdaInterface&)> StuObjectLambdaFunction;

  /// These functions help satisfy uniforms that need extra attribute data in
  /// order to process. These can be used to remove load from the GPU by
  /// precomputing any number of things.
  typedef std::function<void (StuObjectLambdaInterface&, std::list<UnsatisfiedUniform>&)> 
      StuObjectUniformLambdaFunction;

  /// The following functions add hooks into the rendering infrastructure.
  /// @{
  void addLambdaBeginAllPasses(const StuPassLambdaFunction& fp);
  void addLambdaEndAllPasses(const StuPassLambdaFunction& fp);
  void addLambdaPrePass(const StuPassLambdaFunction& fp, const std::string& pass = SPIRE_DEFAULT_PASS);
  void addLambdaPostPass(const StuPassLambdaFunction& fp, const std::string& pass = SPIRE_DEFAULT_PASS);
  /// @}
  
  /// Adds per-object hooks for calculating uniforms and performing rendering.
  /// Both are optional, and you can add as many lambdas as you need.
  /// @{

  /// If an object rendering lambda is found, then normal rendering does not
  /// proceed.
  void addLambdaObjectRender(const StuObjectLambdaFunction& fp, const std::string& object, const std::string& pass = SPIRE_DEFAULT_PASS);

  /// Lambda object uniforms are optional and they will not be called if there
  /// are no unsatisfied uniforms found.
  void addLambdaObjectUniforms(const StuObjectUniformLambdaFunction& fp, const std::string& object, const std::string& pass = SPIRE_DEFAULT_PASS);

  /// @}

  //============================================================================
  // NOT THREAD SAFE
  //============================================================================
  // Be sure that you are calling these functions from the thread upon which
  // spire is executing. All non-thread-safe functions are prefixed with 'nts'.

  /// Renders all passes, in order.
  void ntsDoAllPasses() override;

  /// Perform the entire pass.
  /// \todo Add timing and other semantics here.
  void ntsDoPass(const std::string& pass) override;

  /// Obtain the current number of objects.
  size_t ntsGetNumObjects() const         {return mNameToObject.size();}

  /// Obtain the current rendering order.
  /// (technically, this is thread safe as the rendering thread never accesses
  ///  this value and is only used on the client thread).
  int32_t ntsGetRenderOrder() const       {return mCurrentRenderOrder;}

  /// Obtain the object associated with 'name'.
  /// throws std::range_error if the object is not found.
  std::shared_ptr<const StuObject> ntsGetObjectWithName(const std::string& name) const;
  
  /// Returns true if the system would render the list of object names in the
  /// specified order.
  bool ntsHasRenderingOrder(const std::vector<std::string>& renderOrder) const;

  /// Cleans up all GL resources.
  /// Should ONLY be called from the rendering thread.
  /// In our case, this amounts to disposing of all of our objects and VBO/IBOs
  /// and persistent shader objects.
  virtual void clearGLResources();

  /// Returns true if the specified object is in the pass.
  bool ntsIsObjectInPass(const std::string& object, const std::string& pass) const;

  /// Returns true if the pass already exists.
  bool ntsHasPass(const std::string& pass) const;

private:

  struct Pass
  {
    Pass(const std::string& name) :
        mName(name)
    {}

    std::string                                                   mName;
    std::unordered_map<std::string, std::shared_ptr<StuObject>>   mNameToObject;

    std::vector<StuPassLambdaFunction>                            mPassBeginLambdas;
    std::vector<StuPassLambdaFunction>                            mPassEndLambdas;

    /// \todo Rendering order for the objects?
  };

  /// Remove the specified object from the order list.
  void removeObjectFromOrderList(const std::string& objectName, int32_t objectOrder);

  /// This unordered map is a 1-1 mapping of object names onto objects.
  std::unordered_map<std::string, std::shared_ptr<StuObject>>   mNameToObject;

  /// Rendering order of objects. This map is not a well-defined function: one
  /// value in the domain possibly maps to multiple values in the range.
  std::multimap<int32_t, std::shared_ptr<StuObject>>            mRenderOrderToObjects;

  /// List of shaders that are stored persistently by this pipe (will never
  /// be GC'ed unless this pipe is destroyed).
  std::list<std::shared_ptr<ShaderProgramAsset>>                mPersistentShaders;

  /// VBO names to our representation of a vertex buffer object.
  std::unordered_map<std::string, std::shared_ptr<VBOObject>>   mVBOMap;

  /// IBO names to our representation of an index buffer object.
  std::unordered_map<std::string, std::shared_ptr<IBOObject>>   mIBOMap;

  /// List of passes in the order they are meant to be rendered.
  std::list<std::shared_ptr<Pass>>                              mPasses;
  std::unordered_map<std::string, std::shared_ptr<Pass>>        mNameToPass;

  /// Global begin/end lambdas.
  /// @{
  std::vector<StuPassLambdaFunction>                            mGlobalBeginLambdas;
  std::vector<StuPassLambdaFunction>                            mGlobalEndLambdas;
  /// @}


  // NOTE:  The following variable should only be accessed on the client side.
  //        Never by the renderer. This var just makes it easier when adding
  //        objects and you don't care about their order.
  int32_t mCurrentRenderOrder;    ///< Current rendering order. Used for automatic order assignment.
  int32_t mCurrentPassOrder;      ///< Current pass rendering order.

private:

  /// Implementation functions executed on the renderer thread.
  /// I avoid references unless I know the objects that are being referenced
  /// will still be valid when execution reaches the renderer thread.
  /// (no stack variables allowed).
  /// @{
  static void addPassToBackImpl(Hub& hub, StuInterface* iface, std::string pass);
  static void addPassToFrontImpl(Hub& hub, StuInterface* iface, std::string pass);

  static void addObjectImpl(Hub& hub, StuInterface* iface, std::string object,
                            int32_t renderOrder);

  static void assignRenderOrderImpl(Hub& hub, StuInterface* iface,
                                    std::string object, int32_t renderOrder);

  static void addIBOImpl(Hub& hub, StuInterface* iface,
                         std::string iboName,
                         std::shared_ptr<std::vector<uint8_t>> iboData,
                         StuInterface::IBO_TYPE type);
  static void removeIBOImpl(Hub& hub, StuInterface* iface,
                            std::string iboName);

  static void addVBOImpl(Hub& hub, StuInterface* iface,
                         std::string vboName,
                         std::shared_ptr<std::vector<uint8_t>> vboData,
                         std::vector<std::string> attribNames);
  static void removeVBOImpl(Hub& hub, StuInterface* iface,
                            std::string vboName);

  static void addPassToObjectImpl(Hub& hub, StuInterface* iface,
                                  std::string objectName,
                                  std::string passName,
                                  std::string program,
                                  std::string vboID,
                                  std::string iboID,
                                  PRIMITIVE_TYPES type,
                                  int32_t passOrder);

  static void addObjectTransformImpl(Hub& hub, StuInterface* iface,
                                     std::string objectName,
                                     M44 transform);

  static void addObjectPassUniformInternalImpl(Hub& hub, StuInterface* iface,
                                               std::string object,
                                               std::string pass,
                                               std::string uniformName,
                                               std::shared_ptr<AbstractUniformStateItem> item);

  static void addObjectGlobalUniformInternalImpl(Hub& hub, StuInterface* iface,
                                                 std::string objectName,
                                                 std::string uniformName,
                                                 std::shared_ptr<AbstractUniformStateItem> item);

  static void addObjectGlobalSpireAttributeImpl(Hub& hub, StuInterface* iface,
                                                std::string objectName,
                                                std::string attributeName,
                                                std::shared_ptr<AbstractUniformStateItem> item);

  static void addObjectPassSpireAttributeImpl(Hub& hub, StuInterface* iface,
                                              std::string objectName,
                                              std::string attributeName,
                                              std::shared_ptr<AbstractUniformStateItem> item,
                                              std::string passName);


  static void addObjectPassGPUStateImpl(Hub& hub, StuInterface* iface,
                                        std::string object,
                                        std::string pass,
                                        GPUState state);

  static void addGlobalUniformInternalImpl(Hub& hub, StuInterface* iface,
                                           std::string uniformName,
                                           std::shared_ptr<AbstractUniformStateItem> item);

  static void removeGeomPassFromObjectImpl(Hub& hub, StuInterface* iface,
                                           std::string object,
                                           std::string pass);

  static void removeObjectImpl(Hub& hub, StuInterface* iface,
                               std::string object);
  static void removeAllObjectsImpl(Hub& hub, StuInterface* iface);

  static void addPersistentShaderImpl(Hub& hub, StuInterface* iface,
                                      std::string programName,
                                      std::vector<std::tuple<std::string, SHADER_TYPES>> shaders);


  static void addLambdaBeginAllPassesImpl(Hub& hub, StuInterface* iface, StuPassLambdaFunction fp);
  static void addLambdaEndAllPassesImpl(Hub& hub, StuInterface* iface, StuPassLambdaFunction fp);
  static void addLambdaPrePassImpl(Hub& hub, StuInterface* iface, StuPassLambdaFunction fp, std::string pass);
  static void addLambdaPostPassImpl(Hub& hub, StuInterface* iface, StuPassLambdaFunction fp, std::string pass);
  static void addLambdaObjectUniformsImpl(Hub& hub, StuInterface* iface,
                                          StuObjectUniformLambdaFunction fp, std::string object, std::string pass);
  static void addLambdaObjectRenderImpl(Hub& hub, StuInterface* iface,
                                        StuObjectLambdaFunction fp, std::string object, std::string pass);
  /// @}
};

} // namespace Spire

#endif 
