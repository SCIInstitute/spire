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

#include <utility>

#include "../Common.h"
#include "StuObject.h"
#include "Exceptions.h"
#include "Core/CommonUniforms.h"
#include "Core/Hub.h"
#include "Core/ShaderUniformStateMan.h"


namespace Spire {

//------------------------------------------------------------------------------
// StuPass
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
StuPass::StuPass(
    Hub& hub,
    const std::string& passName, const std::string& programName, int32_t passOrder,
    std::shared_ptr<VBOObject> vbo, std::shared_ptr<IBOObject> ibo, GLenum primitiveType) :

    mVBO(vbo),
    mIBO(ibo),
    mName(passName),
    mPassOrder(passOrder),
    mPrimitiveType(primitiveType),
    mHub(hub)
{
  // findProgram will throw an exception of type std::out_of_range if shader is
  // not found.
  ShaderProgramMan& man = mHub.getShaderProgramManager();
  mShader = man.findProgram(programName);

  // Ensure there is at least enough space in the mUniforms vector. 
  size_t numUniforms = mShader->getUniforms().getNumUniforms();
  mUniforms.reserve(numUniforms);
  mUnsatisfiedUniforms.reserve(numUniforms);

  // Add uniforms present in the shader to the unsatisfied uniforms vector.
  // Not constructing an iterator interface as it's just easier to index.
  for (size_t i = 0; i < numUniforms; i++)
  {
    const ShaderUniformCollection::UniformSpecificData& uniformData = 
        mShader->getUniforms().getUniformAtIndex(i);

    // Check for transformations related to the object transform. If we find
    // them, we won't add them to the unsastisfied uniforms -- instead, they
    // will go into a special hard-coded list that we will update when we are
    // rendered.
    //std::string uniformCodeName = uniformData.uniform->codeName;
    //if (std::get<0>(CommonUniforms::getObject()) == uniformCodeName)
    //{
    //  mObjectTransformUniforms.push_back(
    //      ObjectTransformUniform(ObjectTransformUniform::TRANSFORM_OBJECT, uniformData.glUniformLoc));
    //}
    //else if (std::get<0>(CommonUniforms::getObjectToView()) == uniformCodeName)
    //{
    //  mObjectTransformUniforms.push_back(
    //      ObjectTransformUniform(ObjectTransformUniform::TRANSFORM_OBJECT_TO_CAMERA, uniformData.glUniformLoc));
    //}
    //else if (std::get<0>(CommonUniforms::getObjectToCameraToProjection()) == uniformCodeName)
    //{
    //  mObjectTransformUniforms.push_back(
    //      ObjectTransformUniform(ObjectTransformUniform::TRANSFORM_OBJECT_TO_CAMERA_TO_PROJECTION, uniformData.glUniformLoc));
    //}
    //else
    //{
      mUnsatisfiedUniforms.push_back(
          UnsastisfiedUniformItem(uniformData.uniform->codeName, 
                                  uniformData.glUniformLoc));
    //}

  }
}

//------------------------------------------------------------------------------
StuPass::~StuPass()
{
}

//------------------------------------------------------------------------------
void StuPass::renderPass()
{
  /// \todo Should route through the shader man so we don't re-apply programs
  ///       that are already active.
  GL(glUseProgram(mShader->getProgramID()));

  GL(glBindBuffer(GL_ARRAY_BUFFER, mVBO->getGLIndex()));
  GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO->getGLIndex()));

  /// \todo Ensure attributes are always sorted in ascending order...
  // We have already verified that the attributes contained in the shader
  // are consistent with the attributes we have in the VBO. Therefore, it's
  // okay to calculate the attribute stride based on the shader's stride, and
  // bind all of the shader's attributes.
  const ShaderAttributeCollection& attribs = mVBO->getAttributeCollection();
  attribs.bindAttributes(mShader);

  GPUState priorGPUState = mHub.getGPUStateManager().getState(); // Do NOT store a reference to the state...
  if (mGPUState != nullptr)
    mHub.getGPUStateManager().apply(*mGPUState);

#ifdef SPIRE_DEBUG
  // Gather all uniforms from shader and build a list. Ensure that all uniforms
  // processed by spire match up.
  std::list<std::string> allUniforms;
  for (int i = 0; i < mShader->getUniforms().getNumUniforms(); i++)
  {
    allUniforms.push_back(mShader->getUniforms().getUniformAtIndex(i).uniform->codeName);
  }
#endif

  // Assign pass local uniforms.
  for (auto it = mUniforms.begin(); it != mUniforms.end(); ++it)
  {
#ifdef SPIRE_DEBUG
    allUniforms.remove(it->uniformName);
    //Log::debug() << "Uniform " << it->uniformName << ": " << it->item->asString() << std::endl;
#endif
    it->item->applyUniform(it->shaderLocation);
  }

  // Assign global uniforms, searches through 3 levels in an attempt to find the
  // uniform: object global -> pass global -> and global.
  for (auto it = mUnsatisfiedUniforms.begin(); it != mUnsatisfiedUniforms.end(); ++it)
  {
#ifdef SPIRE_DEBUG
    allUniforms.remove(it->uniformName);
    //Log::debug() << "Uniform " << it->uniformName << ": " << std::endl
    //    << mHub.getShaderUniformStateMan().uniformAsString(it->uniformName) << std::endl;
#endif
    bool applied = mHub.getPassUniformStateMan().tryApplyUniform(mName, it->uniformName, it->shaderLocation);
    if (applied == false)
      mHub.getGlobalUniformStateMan().applyUniform(it->uniformName, it->shaderLocation);
  }

//  // Update any uniforms that require the object transformation.
//  for (auto it = mObjectTransformUniforms.begin(); it != mObjectTransformUniforms.end(); ++it)
//  {
//    std::string codeName;
//    M44 transform;
//    /// \todo Add GL( ) preprocessor definition around the GL calls below.
//    switch (it->transformType)
//    {
//      case ObjectTransformUniform::TRANSFORM_OBJECT:
//        codeName = std::get<0>(CommonUniforms::getObject());
//        glUniformMatrix4fv(static_cast<GLint>(it->varLocation), 1, false,
//                           static_cast<const GLfloat*>(glm::value_ptr(objectToWorld)));
//#ifdef SPIRE_DEBUG
//        allUniforms.remove(codeName);
//#endif
//        break;
//
//      case ObjectTransformUniform::TRANSFORM_OBJECT_TO_CAMERA:
//        codeName = std::get<0>(CommonUniforms::getObjectToView());
//        transform = inverseView * objectToWorld;
//        glUniformMatrix4fv(static_cast<GLint>(it->varLocation), 1, false,
//                           static_cast<const GLfloat*>(glm::value_ptr(transform)));
//#ifdef SPIRE_DEBUG
//        allUniforms.remove(codeName);
//#endif
//        break;
//
//      case ObjectTransformUniform::TRANSFORM_OBJECT_TO_CAMERA_TO_PROJECTION:
//        codeName = std::get<0>(CommonUniforms::getObjectToCameraToProjection());
//        transform = inverseViewProjection * objectToWorld;
//        glUniformMatrix4fv(static_cast<GLint>(it->varLocation), 1, false,
//                           static_cast<const GLfloat*>(glm::value_ptr(transform)));
//#ifdef SPIRE_DEBUG
//        allUniforms.remove(codeName);
//#endif
//        break;
//    }
//  }

#ifdef SPIRE_DEBUG
  if (allUniforms.size() != 0)
  {
    assert(0);
    throw std::runtime_error("Spire should have consumed all uniforms!");
  }
#endif

  //Log::debug() << "Rendering with prim type " << mPrimitiveType << " num elements "
  //             << mIBO->getNumElements() << " ibo type " << mIBO->getType() << std::endl;
  GL(glDrawElements(mPrimitiveType, mIBO->getNumElements(), mIBO->getType(), 0));

  if (mGPUState != nullptr)
    mHub.getGPUStateManager().apply(priorGPUState);
}

//------------------------------------------------------------------------------
bool StuPass::addPassUniform(const std::string uniformName,
                             std::shared_ptr<AbstractUniformStateItem> item,
                             bool isObjectGlobalUniform)
{
  GLenum uniformGlType;
  GLint uniformLoc;

  try
  {
    // Attempt to find uniform in bound shader.
    const ShaderUniformCollection::UniformSpecificData& uniformData = 
        mShader->getUniforms().getUniformData(uniformName);
    uniformGlType = uniformData.glType;
    uniformLoc = uniformData.glUniformLoc;
  }
  catch (std::out_of_range& e)
  {
    return false;  
  }

  // Check uniform type (see UniformStateMan).
  if (uniformGlType != ShaderUniformMan::uniformTypeToGL(item->getGLType()))
    throw ShaderUniformTypeError("Uniform must be the same type as that found in the shader.");

  // Find the uniform in our vector. If it is not already present, then that
  // means we will have to also remove it from our unsatisfied uniforms vector.
  bool foundUniform = false;
  for (auto it = mUniforms.begin(); it != mUniforms.end(); ++it)
  {
    if (it->uniformName == uniformName)
    {
      foundUniform = true;
      if (!(isObjectGlobalUniform == true && it->passSpecific == true))
      {
        // Replace the uniform's contents.
        it->item = item;

        // Ensure we set the pass specific flag if we are setting a specific
        // uniform.
        /// \todo Add a warning to inform the user that shadowing is occuring?
        if (isObjectGlobalUniform == false)
          it->passSpecific = true;
      }
      else
      {
        /// \todo Add a warning to inform the user that shadowing is occuring?
      }
      break;
    }
  }

  // If we did not find the uniform, then ensure that it is present in the
  // unsatisfied uniforms vector. If it is not, then the shader is not expecting
  // this uniform.
  if (foundUniform == false)
  {
    // Update unsatisfied uniforms list. We know that the vector MUST contain
    // the uniform item because we did not find it while looping through our
    // pre-existing uniforms. It has also passed an existence check against
    // the shader and a type check.
    bool foundUnsatisfiedUniform = false;
    for (auto it = mUnsatisfiedUniforms.begin(); it != mUnsatisfiedUniforms.end(); ++it)
    {
      if (it->uniformName == uniformName)
      {
        mUnsatisfiedUniforms.erase(it);
        foundUnsatisfiedUniform = true;
        break;
      }
    }

    if (foundUnsatisfiedUniform == false)
    {
      return false;
    }

    mUniforms.emplace_back(UniformItem(uniformName, item, uniformLoc,
                                       !isObjectGlobalUniform));
  }

  return true;
}

//------------------------------------------------------------------------------
void StuPass::addGPUState(const GPUState& state)
{
  // This will destroy any prior gpu state.
  mGPUState = std::unique_ptr<GPUState>(new GPUState(state));
}

//------------------------------------------------------------------------------
bool StuPass::hasPassSpecificUniform(const std::string& uniformName) const
{
  for (auto it = mUniforms.begin(); it != mUniforms.end(); ++it)
  {
    if (it->uniformName == uniformName)
    {
      if (it->passSpecific)
        return true;
      else
        return false;
    }
  }

  return false;
}

//------------------------------------------------------------------------------
bool StuPass::hasUniform(const std::string& uniformName) const
{
  for (auto it = mUniforms.begin(); it != mUniforms.end(); ++it)
  {
    if (it->uniformName == uniformName)
      return true;
  }
  return false;
}


/// \note If we ever implement a remove pass uniform function, be *sure* to
///       update the unsatisfied uniforms vector!

//------------------------------------------------------------------------------
// StuObject
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
StuObject::StuObject(Hub& hub, const std::string& name, int32_t renderOrder) :
    mName(name),
    mRenderOrder(renderOrder),
    mHub(hub)
{
  // Reserve at least one slot for the object transformation, and one more for
  // good measure.
  mSpireAttributes.reserve(2);
}


//------------------------------------------------------------------------------
void StuObject::addPass(
    const std::string& passName,
    const std::string& program,
    std::shared_ptr<VBOObject> vbo,
    std::shared_ptr<IBOObject> ibo,
    GLenum type,
    int32_t passOrder)
{
  // Check to see if there already is a pass by that name...
  if (mPasses.find(passName) != mPasses.end())
    throw Duplicate("There already exists a pass with the specified pass name.");

  // Build the pass.
  std::shared_ptr<StuPass> pass(new StuPass(mHub, passName, program, passOrder,
                                            vbo, ibo, type));
  
  mPasses.insert(std::make_pair(passName, pass));
  mPassRenderOrder.insert(std::make_pair(passOrder, pass));

  // Copy any global uniforms that may be relevant to the pass' shader.
  for (auto it = mObjectGlobalUniforms.begin(); it != mObjectGlobalUniforms.end(); ++it)
  {
    pass->addPassUniform(it->uniformName, it->item, true);
  }
}

//------------------------------------------------------------------------------
std::shared_ptr<const StuPass> StuObject::getObjectPassParams(const std::string& passName) const
{
  return getPassByName(passName);
}

//------------------------------------------------------------------------------
void StuObject::removePass(const std::string& passName)
{
  // This call will throw std::out_of_range error if passName doesn't exist in
  // the pass' unordered_map.
  std::shared_ptr<StuPass> pass = getPassByName(passName);

  mPasses.erase(passName);
  removePassFromOrderList(pass->getName(), pass->getPassOrder());
}

//------------------------------------------------------------------------------
void StuObject::removePassFromOrderList(const std::string& passName,
                                        int32_t passOrder)
{
  auto it = mPassRenderOrder.find(passOrder);
  /// \xxx Should we be only iterating to mRenderOrderToObjects.count(objRenderOrder)?
  for (; it != mPassRenderOrder.end(); ++it)
  {
    if (it->second->getName() == passName)
    {
      mPassRenderOrder.erase(it);
      return;
    }
  }
  throw std::range_error("Unable to find object to remove in render order map.");
}

//------------------------------------------------------------------------------
void StuObject::addObjectSpireAttribute(const std::string& attributeName,
                                        std::shared_ptr<AbstractUniformStateItem> item)
{
  mSpireAttributes[attributeName] = item;
}

//------------------------------------------------------------------------------
std::shared_ptr<AbstractUniformStateItem> StuObject::getObjectSpireAttribute(const std::string& attribName) const
{
  return mSpireAttributes.at(attribName);
}

//------------------------------------------------------------------------------
void StuObject::addObjectTransform(const M44& transform)
{
  mObjectTransform = transform;
}

//------------------------------------------------------------------------------
void StuObject::addPassUniform(const std::string& passName,
                               const std::string uniformName,
                               std::shared_ptr<AbstractUniformStateItem> item)
{
  // We are going to have a facility similar to UniformStateMan, but we are
  // going to use a more cache-coherent vector. It's unlikely that we ever need
  // to grow the vector beyond the number of uniforms already present in the
  // shader.
  std::shared_ptr<StuPass> pass = getPassByName(passName);
  if (pass->addPassUniform(uniformName, item, false) == false)
  {
    std::stringstream stream;
    stream << "This uniform (" << uniformName << ") is not recognized by the shader.";
    throw std::invalid_argument(stream.str());
  }
}

//------------------------------------------------------------------------------
void StuObject::addGlobalUniform(const std::string& uniformName,
                                 std::shared_ptr<AbstractUniformStateItem> item)
{
  // Search for an already pre-existing uniform.
  bool foundUniform = false;
  for (auto it = mObjectGlobalUniforms.begin(); it != mObjectGlobalUniforms.end(); ++it)
  {
    if (it->uniformName == uniformName)
    {
      foundUniform = true;

      // Replace the uniform's contents.
      it->item = item;
      break;
    }
  }

  if (foundUniform == false)
  {
    // Add a new entry and update
    ObjectGlobalUniformItem uniformItem(uniformName, item);
    mObjectGlobalUniforms.push_back(uniformItem);
  }

  // Attempt to update any children pass' that contain this uniform.
  for (auto it = mPasses.begin(); it != mPasses.end(); ++it)
  {
    it->second->addPassUniform(uniformName, item, true);
  }
}

//------------------------------------------------------------------------------
void StuObject::addPassGPUState(const std::string& passName, const GPUState& state)
{
  std::shared_ptr<StuPass> pass = getPassByName(passName);
  pass->addGPUState(state);
}

//------------------------------------------------------------------------------
std::shared_ptr<StuPass> StuObject::getPassByName(const std::string& name) const
{
  return mPasses.at(name);
}

//------------------------------------------------------------------------------
void StuObject::renderAllPasses()
{
  for (auto it = mPassRenderOrder.begin(); it != mPassRenderOrder.end(); ++it)
  {
    it->second->renderPass();
  }
}

//------------------------------------------------------------------------------
bool StuObject::hasGlobalUniform(const std::string& uniformName) const
{
  for (auto it = mObjectGlobalUniforms.begin(); it != mObjectGlobalUniforms.end(); ++it)
  {
    if (it->uniformName == uniformName)
      return true;
  }
  return false;
}

//------------------------------------------------------------------------------
void StuObject::renderPass(const std::string& passName)
{
  std::shared_ptr<StuPass> pass = mPasses[passName];
  pass->renderPass();
}

//------------------------------------------------------------------------------
bool StuObject::hasPassRenderingOrder(const std::vector<std::string>& passes) const
{
  // Iterate over the map (we will iterate over the map in ascending order).
  // This is the same loop we will be using in the renderer.
  auto itToTest = passes.begin();
  auto itOrder = mPassRenderOrder.begin();
  for (; itOrder != mPassRenderOrder.end() && itToTest != passes.end();
       ++itOrder, ++itToTest)
  {
    if (itOrder->second->getName() != *itToTest)
      return false;
  }

  if (itToTest == passes.end() && itOrder == mPassRenderOrder.end())
    return true;
  else
    return false;
}

} // end of namespace Spire
