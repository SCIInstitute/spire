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

#include "Hub.h"
#include "InterfaceImplementation.h"
#include "SpireObject.h"

namespace Spire {


//------------------------------------------------------------------------------
InterfaceImplementation::InterfaceImplementation(Hub& hub) :
    mHub(hub)
{
}

//------------------------------------------------------------------------------
bool InterfaceImplementation::addFunctionToQueue(const Hub::RemoteFunction& fun)
{
#ifdef SPIRE_USE_STD_THREADS
  return mQueue.push(ThreadMessage(fun));
#else
  // Call the function immediately. This case (without std threads) will be 
  // used as our synchronized test harness.
  fun(mHub);
  return true;
#endif
}

//------------------------------------------------------------------------------
void InterfaceImplementation::executeQueue()
{
#ifdef SPIRE_USE_STD_THREADS
  ThreadMessage msg;
  while (mQueue.pop(msg))
  {
    try
    {
      msg.execute(this);
    }
    catch (std::exception& e)
    {
      Log::error() << "Spire exception thrown: " << e.what() << std::endl;
    }
    msg.clear();
  }
#else
  // The functions were already executed.
#endif
}


//------------------------------------------------------------------------------
void InterfaceImplementation::resize(Hub& hub, size_t width, size_t height)
{
  GL(glViewport(0, 0, width, height));
}

//------------------------------------------------------------------------------
void InterfaceImplementation::clearGLResources()
{
  mNameToObject.clear();
  mRenderOrderToObjects.clear();
  mPersistentShaders.clear();
  mVBOMap.clear();
  mIBOMap.clear();

  // Do we want to clear passes? They don't have any associated GL data.
  //mPasses.clear();
  //mNameToPass.clear();
  //mGlobalBeginLambdas.clear();
}

//------------------------------------------------------------------------------
void InterfaceImplementation::doAllPasses()
{
  /// \todo Call all passes begin lambdas. Used primarily to setup global
  /// uniforms.

  /// \todo Move this outside of the interface!
  GL(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
  GL(glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT));

  /// \todo Make line width a part of the GPU state.
  glLineWidth(2.0f);
  //glEnable(GL_LINE_SMOOTH);

  GPUState defaultGPUState;
  mHub.getGPUStateManager().apply(defaultGPUState, true); // true = force application of state.


  // Loop through all of the passes.
  for (auto it = mPasses.begin(); it != mPasses.end(); ++it)
  {
    doPass((*it)->mName);
  }

  /// \todo Call all passes end lambdas. Used primarily to setup global
  /// uniforms.
}

//------------------------------------------------------------------------------
bool InterfaceImplementation::hasPass(const std::string& pass) const
{
  return (mNameToPass.find(pass) != mNameToPass.end());
}

//------------------------------------------------------------------------------
void InterfaceImplementation::doPass(const std::string& passName)
{
  std::shared_ptr<Pass> pass = mNameToPass.at(passName);

  ///\todo Call pass begin lambdas. Setup global pass specific uniforms.

  // Loop over all objects in the pass and render them.
  /// \todo Need to add some way of ordering the rendered objects, whether it be
  /// by another structure built into Spire (not for this at all), or some lambda
  /// callback.
  for (auto it = pass->mNameToObject.begin(); it != pass->mNameToObject.end(); ++it)
  {
    it->second->renderPass(passName);
  }

  ///\todo Call pass end lambda.
}

//------------------------------------------------------------------------------
void InterfaceImplementation::addPassToFront(InterfaceImplementation* self, std::string pass)
{
  // Verify that there is no pass by that name already.
  if (self->hasPass(passName) == true)
    throw std::runtime_error("Pass (" + passName + ") already exists!");

  std::shared_ptr<Pass> pass(new Pass(passName));
  self->mPasses.push_back(pass);
  self->mNameToPass[passName] = pass;
}

} // end of namespace Spire

