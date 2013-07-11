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

#ifndef SPIRE_HIGH_INTERFACEIMPLEMENTATION_H
#define SPIRE_HIGH_INTERFACEIMPLEMENTATION_H

#include <memory>
#include <utility>
#include <vector>
#include <list>
#include <string>
#include <unordered_map>
#include <map>
#include <tuple>
#include <cstdint>
#include "../Common.h"
#include "../Interface.h"
#include "../InterfaceCommon.h"

#include "ObjectLambda.h"
#include "ThreadMessage.h"

#ifdef SPIRE_USE_STD_THREADS
#include "CircFIFOSeqCons.hpp"
#endif

namespace Spire
{

class Hub;
class SpireObject;
class ShaderProgramAsset;
class VBOObject;
class IBOObject;

/// Implementation of the functions exposed in Interface.h
/// All functions in this class are not thread safe.
class InterfaceImplementation
{
public:
  InterfaceImplementation(Hub& hub);
  virtual ~InterfaceImplementation()  {}
  
  /// SHOULD ONLY be called by the thread associated with Queue.
  /// Will add 'fun' to the queue associated with 'thread'.
  /// \return false if we failed to add the function to the specified queue.
  ///         queue is likely to be full.
  bool addFunctionToQueue(const Hub::RemoteFunction& fun);

  /// SHOULD ONLY be called by the spire thread!
  /// Will execute all commands the the queue associated with Interface::THREAD.
  void executeQueue();

  //============================================================================
  // STATIC IMPLEMENTATION -- Called from interface
  //============================================================================
  // All of the functions below constitute the implementation of the interface
  // to spire. 
  // NOTE: None of the functions below should not take references or raw
  // pointers with the exception of the Hub* raw poiner (unfortunately
  // unavoidable). We don't want to worry about the lifetime of the objects
  // during cross-thread communication.

  //----------
  // Graphics
  //----------
  /// Called in the event of a resize. This calls glViewport with 0, 0, width, height.
  static void resize(Hub& hub, size_t width, size_t height);

  //============================================================================
  // IMPLEMENTATION
  //============================================================================
  /// Cleans up all GL resources.
  void clearGLResources();

  /// Performs all rendering passes.
  void doAllPasses();

  /// Performs a rendering pass.
  void doPass(const std::string& pass);

  /// Retrieves number of objects.
  size_t getNumObjects()      {return mNameToObject.size();}

  /// Returns true if the pass already exists.
  bool hasPass(const std::string& pass) const;

  /// \note: All of the functions below except all of their parameters by
  ///        *value* not by reference. The only exception is the Hub variable,
  ///        which is passed by the current thread so we are guarenteed that it
  ///        has not gone out of scope. We have no such guarantees about 
  ///        variables on a separate thread.

  //============================================================================
  // CALLBACK IMPLEMENTATION -- Called from interface
  //============================================================================

  //--------
  // Passes
  //--------

  /// Adds a pass to the front of the pass list. Passes at the front of the list
  /// are rendered first.
  static void addPassToFront(InterfaceImplementation& self, std::string passName);

  /// Adds a pass to the back of the pass list. Passes at the back of the list
  /// are rendered last.
  static void addPassToBack(InterfaceImplementation& self, std::string passName);

  //---------
  // Objects
  //---------

  /// Adds a renderable 'object' to the scene.
  void addObject(InterfaceImplementation& self, std::string objectName);

private:

  struct Pass
  {
    Pass(const std::string& name) :
        mName(name)
    {}

    std::string                                                     mName;
    std::unordered_map<std::string, std::shared_ptr<SpireObject>>   mNameToObject;

    std::vector<Interface::PassLambdaFunction>                      mPassBeginLambdas;
    std::vector<Interface::PassLambdaFunction>                      mPassEndLambdas;

    /// \todo Rendering order for the objects?
  };

  /// This unordered map is a 1-1 mapping of object names onto objects.
  std::unordered_map<std::string, std::shared_ptr<SpireObject>>   mNameToObject;

  /// List of shaders that are stored persistently by this pipe (will never
  /// be GC'ed unless this pipe is destroyed).
  std::list<std::shared_ptr<ShaderProgramAsset>>                  mPersistentShaders;

  /// VBO names to our representation of a vertex buffer object.
  std::unordered_map<std::string, std::shared_ptr<VBOObject>>     mVBOMap;

  /// IBO names to our representation of an index buffer object.
  std::unordered_map<std::string, std::shared_ptr<IBOObject>>     mIBOMap;

  /// List of passes in the order they are meant to be rendered.
  std::list<std::shared_ptr<Pass>>                                mPasses;
  std::unordered_map<std::string, std::shared_ptr<Pass>>          mNameToPass;

  /// Global begin/end lambdas.
  /// @{
  std::vector<Interface::PassLambdaFunction>                      mGlobalBeginLambdas;
  std::vector<Interface::PassLambdaFunction>                      mGlobalEndLambdas;
  /// @}

private:

#ifdef SPIRE_USE_STD_THREADS
  /// \todo Change to boost <url:http://www.boost.org/doc/libs/1_53_0/doc/html/lockfree.html> 
  ///       Wouldn't have to deal with the limit to message size...
  typedef CircularFifo<ThreadMessage,256> MessageQueue;
  MessageQueue    mQueue;
#endif

  Hub&            mHub;
};

} // namespace Spire

#endif 
