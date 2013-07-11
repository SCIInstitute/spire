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
#include "SpireObject.h"
#include "Exceptions.h"
#include "Hub.h"
#include "ShaderUniformStateMan.h"


namespace Spire {

//------------------------------------------------------------------------------
// ObjectPass
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
ObjectPass::ObjectPass(
    Hub& hub,
    const std::string& passName, const std::string& programName,
    std::shared_ptr<VBOObject> vbo, std::shared_ptr<IBOObject> ibo, GLenum primitiveType) :

    mVBO(vbo),
    mIBO(ibo),
    mName(passName),
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

    mUnsatisfiedUniforms.push_back(
        UnsastisfiedUniformItem(uniformData.uniform->codeName, 
                                uniformData.glUniformLoc,
                                uniformData.glType));
  }
}

//------------------------------------------------------------------------------
ObjectPass::~ObjectPass()
{
}

//------------------------------------------------------------------------------
void ObjectPass::renderPass(ObjectLambdaInterface& lambdaInterface)
{
  GL(glUseProgram(mShader->getProgramID()));

  GL(glBindBuffer(GL_ARRAY_BUFFER, mVBO->getGLIndex()));
  GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO->getGLIndex()));

  // We have already verified that the attributes contained in the shader
  // are consistent with the attributes we have in the VBO. Therefore, it's
  // okay to calculate the attribute stride based on the shader's stride, and
  // bind all of the shader's attributes.
  const ShaderAttributeCollection& attribs = mVBO->getAttributeCollection();
  attribs.bindAttributes(mShader);

  GPUState priorGPUState = mHub.getGPUStateManager().getState(); // Do NOT store a reference to the state...
  if (mGPUState != nullptr)
    mHub.getGPUStateManager().apply(*mGPUState);

  // Assign pass local uniforms.
  for (auto it = mUniforms.begin(); it != mUniforms.end(); ++it)
  {
    it->item->applyUniform(it->shaderLocation);
  }

  // Assign global uniforms, searches through 3 levels in an attempt to find the
  // uniform: object global -> pass global -> and global.
  std::list<Interface::UnsatisfiedUniform> unsatisfiedGlobalUniforms;
  for (auto it = mUnsatisfiedUniforms.begin(); it != mUnsatisfiedUniforms.end(); ++it)
  {
    bool applied = mHub.getPassUniformStateMan().tryApplyUniform(mName, it->uniformName, it->shaderLocation);
    if (applied == false)
    {
      if (mHub.getGlobalUniformStateMan().applyUniform(it->uniformName, it->shaderLocation) == false)
        unsatisfiedGlobalUniforms.push_back(
            Interface::UnsatisfiedUniform(it->uniformName, it->shaderLocation, it->uniformType));
    }
  }

  // If we have an unsatisfied uniforms callback, ensure that it is called..
  if (unsatisfiedGlobalUniforms.size() > 0)
  {
    for (auto it = mUniformLambdas.begin(); it != mUniformLambdas.end(); ++it)
    {
      (*it)(lambdaInterface, unsatisfiedGlobalUniforms);
    }

    if (unsatisfiedGlobalUniforms.size() > 0)
      throw ShaderUniformNotFound("Could not initialize uniform: " + unsatisfiedGlobalUniforms.front().uniformName);
  }
  

  if (mRenderLambdas.size() > 0)
  {
    // Execute custom rendering callbacks.
    for (auto it = mRenderLambdas.begin(); it != mRenderLambdas.end(); ++it)
    {
      (*it)(lambdaInterface);
    }
  }
  else
  {
    GL(glDrawElements(mPrimitiveType, mIBO->getNumElements(), mIBO->getType(), 0));
  }

  if (mGPUState != nullptr)
    mHub.getGPUStateManager().apply(priorGPUState);
}

//------------------------------------------------------------------------------
bool ObjectPass::addPassUniform(const std::string uniformName,
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
void ObjectPass::addGPUState(const GPUState& state)
{
  // This will destroy any prior gpu state.
  mGPUState = std::unique_ptr<GPUState>(new GPUState(state));
}

//------------------------------------------------------------------------------
bool ObjectPass::hasPassSpecificUniform(const std::string& uniformName) const
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
bool ObjectPass::hasUniform(const std::string& uniformName) const
{
  for (auto it = mUniforms.begin(); it != mUniforms.end(); ++it)
  {
    if (it->uniformName == uniformName)
      return true;
  }
  return false;
}

//------------------------------------------------------------------------------
void ObjectPass::addSpireAttribute(const std::string& attributeName,
                                std::shared_ptr<AbstractUniformStateItem> item)
{
  mSpireAttributes[attributeName] = item;
}

//------------------------------------------------------------------------------
std::shared_ptr<const AbstractUniformStateItem> ObjectPass::getSpireAttribute(
    const std::string& attribName) const
{
  auto it = mSpireAttributes.find(attribName);
  if (it != mSpireAttributes.end())
  {
    return it->second;
  }
  else
  {
    return std::shared_ptr<AbstractUniformStateItem>(); 
  }
}

//------------------------------------------------------------------------------
void ObjectPass::addRenderLambda(const Interface::ObjectLambdaFunction& fp)
{
  mRenderLambdas.push_back(fp);
}

//------------------------------------------------------------------------------
void ObjectPass::addUniformLambda(const Interface::ObjectUniformLambdaFunction& fp)
{
  mUniformLambdas.push_back(fp);
}

/// \note If we ever implement a remove pass uniform function, be *sure* to
///       update the unsatisfied uniforms vector!

//------------------------------------------------------------------------------
// SpireObject
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
SpireObject::SpireObject(Hub& hub, const std::string& name) :
    mName(name),
    mHub(hub)
{
  // Reserve at least one slot for the object transformation, and one more for
  // good measure.
  mSpireAttributes.reserve(2);
}


//------------------------------------------------------------------------------
void SpireObject::addPass(
    const std::string& passName,
    const std::string& program,
    std::shared_ptr<VBOObject> vbo,
    std::shared_ptr<IBOObject> ibo,
    GLenum type)
{
  // Check to see if there already is a pass by that name...
  if (mPasses.find(passName) != mPasses.end())
    throw Duplicate("There already exists a pass with the specified pass name.");

  // Build the pass.
  std::shared_ptr<ObjectPass> pass(new ObjectPass(mHub, passName, program, vbo, ibo, type));
  mPasses.insert(std::make_pair(passName, pass));

  // Copy any global uniforms that may be relevant to the pass' shader.
  for (auto it = mObjectGlobalUniforms.begin(); it != mObjectGlobalUniforms.end(); ++it)
  {
    pass->addPassUniform(it->uniformName, it->item, true);
  }
}

//------------------------------------------------------------------------------
std::shared_ptr<const ObjectPass> SpireObject::getObjectPassParams(const std::string& passName) const
{
  return getPassByName(passName);
}

//------------------------------------------------------------------------------
void SpireObject::removePass(const std::string& passName)
{
  // This call will throw std::out_of_range error if passName doesn't exist in
  // the pass' unordered_map.
  std::shared_ptr<ObjectPass> pass = getPassByName(passName);

  mPasses.erase(passName);
}

//------------------------------------------------------------------------------
void SpireObject::addObjectGlobalSpireAttribute(const std::string& attributeName,
                                              std::shared_ptr<AbstractUniformStateItem> item)
{
  mSpireAttributes[attributeName] = item;
}

//------------------------------------------------------------------------------
std::shared_ptr<const AbstractUniformStateItem> SpireObject::getObjectGlobalSpireAttribute(
    const std::string& attribName) const
{
  auto it = mSpireAttributes.find(attribName);
  if (it != mSpireAttributes.end())
  {
    return it->second;
  }
  else
  {
    // This is the highest we can go looking for attributes. The buck stops here.
    throw std::runtime_error("Unable to find object global attribute.");
  }
}

//------------------------------------------------------------------------------
void SpireObject::addObjectPassSpireAttribute(const std::string& passName,
                                            const std::string& attributeName,
                                            std::shared_ptr<AbstractUniformStateItem> item)
{
  std::shared_ptr<ObjectPass> pass = getPassByName(passName);
  pass->addSpireAttribute(attributeName, item);
}

//------------------------------------------------------------------------------
std::shared_ptr<const AbstractUniformStateItem> SpireObject::getObjectPassSpireAttribute(
    const std::string& passName,
    const std::string& attribName) const
{
  std::shared_ptr<ObjectPass> pass = getPassByName(passName);
  return pass->getSpireAttribute(attribName);
}

//------------------------------------------------------------------------------
void SpireObject::addPassUniform(const std::string& passName,
                               const std::string uniformName,
                               std::shared_ptr<AbstractUniformStateItem> item)
{
  // We are going to have a facility similar to UniformStateMan, but we are
  // going to use a more cache-coherent vector. It's unlikely that we ever need
  // to grow the vector beyond the number of uniforms already present in the
  // shader.
  std::shared_ptr<ObjectPass> pass = getPassByName(passName);
  if (pass->addPassUniform(uniformName, item, false) == false)
  {
    std::stringstream stream;
    stream << "This uniform (" << uniformName << ") is not recognized by the shader.";
    throw std::invalid_argument(stream.str());
  }
}

//------------------------------------------------------------------------------
void SpireObject::addGlobalUniform(const std::string& uniformName,
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
void SpireObject::addPassGPUState(const std::string& passName, const GPUState& state)
{
  std::shared_ptr<ObjectPass> pass = getPassByName(passName);
  pass->addGPUState(state);
}

//------------------------------------------------------------------------------
std::shared_ptr<ObjectPass> SpireObject::getPassByName(const std::string& name) const
{
  return mPasses.at(name);
}

//------------------------------------------------------------------------------
bool SpireObject::hasGlobalUniform(const std::string& uniformName) const
{
  for (auto it = mObjectGlobalUniforms.begin(); it != mObjectGlobalUniforms.end(); ++it)
  {
    if (it->uniformName == uniformName)
      return true;
  }
  return false;
}

//------------------------------------------------------------------------------
void SpireObject::renderPass(const std::string& passName)
{
  ObjectLambdaInterface lambdaInterface(mHub, passName, *this);
  std::shared_ptr<ObjectPass> pass = mPasses[passName];
  pass->renderPass(lambdaInterface);
}

//------------------------------------------------------------------------------
void SpireObject::addPassRenderLambda(const std::string& pass, const Interface::ObjectLambdaFunction& fp)
{
  getPassByName(pass)->addRenderLambda(fp);
}

//------------------------------------------------------------------------------
void SpireObject::addPassUniformLambda(const std::string& pass, const Interface::ObjectUniformLambdaFunction& fp)
{
  getPassByName(pass)->addUniformLambda(fp);
}

} // end of namespace Spire
