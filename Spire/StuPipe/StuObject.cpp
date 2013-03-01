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

#include "StuObject.h"
#include "Core/Hub.h"

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
}

//------------------------------------------------------------------------------
StuPass::~StuPass()
{
}

//------------------------------------------------------------------------------
// StuObject
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
StuObject::StuObject(Hub& hub, const std::string& name, int32_t renderOrder) :
    mName(name),
    mRenderOrder(renderOrder),
    mHub(hub)
{

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
  
  mPasses.insert(make_pair(passName, pass));
  mPassRenderOrder.insert(make_pair(passOrder, pass));
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
void StuObject::addPassUniform(const std::string& pass,
                      const std::string uniformName,
                      std::shared_ptr<AbstractUniformStateItem> item)
{
}


//------------------------------------------------------------------------------
std::shared_ptr<StuPass> StuObject::getPassByName(const std::string& name)
{
  return mPasses.at(name);
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
