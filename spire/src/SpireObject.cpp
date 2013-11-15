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

#include "Common.h"
#include "SpireObject.h"
#include "Exceptions.h"
#include "Hub.h"
#include "ShaderUniformStateMan.h"
#include "LambdaInterface.h"
#include "ObjectLambda.h"

namespace CPM_SPIRE_NS {

//------------------------------------------------------------------------------
// ObjectPass
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
ObjectPass::ObjectPass(
    Hub& hub,
    const std::string& passName, const std::string& programName,
    std::shared_ptr<VBOObject> vbo, std::shared_ptr<IBOObject> ibo, GLenum primitiveType) :

    mName(passName),
    mPrimitiveType(primitiveType),
    mVBO(vbo),
    mIBO(ibo),
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
void ObjectPass::renderPass(ObjectLambdaInterface& lambdaInterface,
                            const Interface::UnsatisfiedUniformCB& cb)
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
    ShaderUniformMan::applyUniformGLState(it->item, it->shaderLocation);
    //std::cout << it->uniformName << ": " << it->item->asString() << std::endl;
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
    {
      if (cb)
      {
        cb(unsatisfiedGlobalUniforms);
      }
    }

    if (unsatisfiedGlobalUniforms.size() > 0)
      throw ShaderUniformNotFound("Could not initialize uniform: " 
                                  + unsatisfiedGlobalUniforms.front().uniformName);
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
    GL(glDrawElements(mPrimitiveType, static_cast<GLsizei>(mIBO->getNumElements()), mIBO->getType(), 0));
  }

  // We can do away with this once we switch to VAOs.
  attribs.unbindAttributes(mShader);

  if (mGPUState != nullptr)
    mHub.getGPUStateManager().apply(priorGPUState);
}

//------------------------------------------------------------------------------
bool ObjectPass::addPassUniform(const std::string& uniformName,
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
  catch (std::out_of_range&)
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
std::shared_ptr<const AbstractUniformStateItem>
ObjectPass::getPassUniform(const std::string& uniformName)
{
  for (auto it = mUniforms.begin(); it != mUniforms.end(); ++it)
  {
    if (it->uniformName == uniformName)
    {
      return it->item;
    }
  }

  return std::shared_ptr<const AbstractUniformStateItem>();
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
}


//------------------------------------------------------------------------------
void SpireObject::addPass(
    const std::string& passName,
    const std::string& program,
    std::shared_ptr<VBOObject> vbo,
    std::shared_ptr<IBOObject> ibo,
    GLenum type, const std::string& parentPass)
{
  // Check to see if there already is a pass by that name...
  auto foundPass = mPasses.find(passName);
  std::shared_ptr<ObjectPass> pass(new ObjectPass(mHub, passName, program, vbo, ibo, type));

  // Check for corner case where subpasses were added to the object before
  // the pass was added itself.
  if (foundPass != mPasses.end())
  {
    // Corner case
    if ((*foundPass).second.objectPass == nullptr)
    {
      (*foundPass).second.objectPass = pass;
    }
    else
    {
      Log::error() << "Attempting to add another object pass by the same name.";
      throw Duplicate("There already exists a pass with the specified pass name.");
    }
  }
  else
  {
    // Build the pass and associate it with any parent pass.
    ObjectPassInternal internalPass(pass);

    // Insert the pass into our array. Even sub-passes are added to the toplevel
    // of our array. This is so that we can use the normal pass functions to
    // manipulate subpasses without any extra logic.
    mPasses.insert(std::make_pair(passName, internalPass));
  }

  // Check to see if we are adding a subpass. If we are, look up the parent
  // pass' object. If the parent pass' object does not exist yet, simply
  // create it. This auto-creation is the cause of the corner case above.
  if (parentPass.size() > 0)
  {
    auto parentPassIt = mPasses.find(parentPass);

    if (parentPassIt == mPasses.end())
    {
      ObjectPassInternal parentInternalPass(nullptr);

      // Construct a dummy pass for the parent.
      mPasses.insert(std::make_pair(parentPass, parentInternalPass));
      parentPassIt = mPasses.find(parentPass);
    }
    
    // Create subpass vector if it doesn't already exist.
    if (parentPassIt->second.objectSubPasses == nullptr)
    {
      parentPassIt->second.objectSubPasses = 
          std::shared_ptr<std::vector<std::shared_ptr<ObjectPass>>>(
              new std::vector<std::shared_ptr<ObjectPass>>);
    }

    // Note that duplicate sub-passes have already been weeded out via the
    // name checking that occurs above (since we add subpasses to the top level
    // passes anyways).

    // Now add the subpass to the parentPass' vector of subPasses.
    std::vector<std::shared_ptr<ObjectPass>>& subPasses = *parentPassIt->second.objectSubPasses;
    subPasses.push_back(pass);
  }

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
std::shared_ptr<const AbstractUniformStateItem>
SpireObject::getPassUniform(const std::string& passName,
                            const std::string& uniformName)
{
  // We are going to have a facility similar to UniformStateMan, but we are
  // going to use a more cache-coherent vector. It's unlikely that we ever need
  // to grow the vector beyond the number of uniforms already present in the
  // shader.
  std::shared_ptr<ObjectPass> pass = getPassByName(passName);
  return pass->getPassUniform(uniformName);
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
    if (it->second.objectPass != nullptr)
      it->second.objectPass->addPassUniform(uniformName, item, true);
  }
}

//------------------------------------------------------------------------------
std::shared_ptr<const AbstractUniformStateItem>
SpireObject::getGlobalUniform(const std::string& uniformName)
{
  for (auto it = mObjectGlobalUniforms.begin(); it != mObjectGlobalUniforms.end(); ++it)
  {
    if (it->uniformName == uniformName)
    {
      return it->item;
    }
  }
  return std::shared_ptr<const AbstractUniformStateItem>();
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
  std::shared_ptr<ObjectPass> pass;
  try
  {
    pass = mPasses.at(name).objectPass;
  }
  catch (std::exception&)
  {
    Log::error() << "Unable to find SpireObject pass: " << name << ". " 
                 << "Make sure it has been added to the system. "
                 << "Generally this means that you should add passes to the object before performin this operation." << std::endl;
  }

  if (pass != nullptr)
    return pass;
  else
    throw NotFound("Pass (" + name + ") was found, but no object provided. Unable to find pass with given name.");
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
  renderPass(passName, nullptr);
}

//------------------------------------------------------------------------------
void SpireObject::renderPass(const std::string& passName,
                             const Interface::UnsatisfiedUniformCB& cb)
{
  /// \todo Possibly get rid of lambda interfaces.
  ObjectLambdaInterface lambdaInterface(mHub, passName, *this);
  ObjectPassInternal& internalObjectPass = mPasses[passName];
  std::shared_ptr<ObjectPass> pass = internalObjectPass.objectPass;

  // Render the pass
  if (pass != nullptr)
    pass->renderPass(lambdaInterface, cb);

  // Now render any associated subpasses.
  if (internalObjectPass.objectSubPasses != nullptr)
  {
    for (auto it = internalObjectPass.objectSubPasses->begin(); 
         it != internalObjectPass.objectSubPasses->end(); ++it)
    {
      ObjectLambdaInterface subLambdaInterface(mHub, (*it)->getName(), *this);
      (*it)->renderPass(subLambdaInterface, cb);
    }
  }
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

} // namespace CPM_SPIRE_NS

