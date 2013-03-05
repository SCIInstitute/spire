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

#include "Interface.h"
#include "Core/ThreadMessage.h"
#include "Core/Hub.h"
#include "PipeInterface.h"

#ifdef SPIRE_USE_STD_THREADS
#include "Core/CircFIFOSeqCons.hpp"
#endif

namespace Spire
{

/// Implementation of the functions exposed in Interface.h
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
  // IMPLEMENTATION
  //============================================================================
  // All of the functions below constitute the implementation of the interface
  // to spire. 
  // NOTE: None of the functions below should not take references or raw
  // pointers with the exception of the Hub* raw poiner (unfortunately
  // unavoidable). We don't want to worry about the lifetime of the objects
  // during cross-thread communication.

  //-------
  // Pipes
  //-------
  /// See corresponding Interface definition.
  static void pipePushBack(Hub& hub, std::shared_ptr<PipeInterface> pipe);
  
  /// See corresponding Interface definition.
  static void pipeRemove(Hub& hub, std::shared_ptr<PipeInterface> pipe);

  //------------------
  // HACKED Interface
  //------------------
  // Everything in this interface will be rendered using the uniform color
  // shader.

  /// HACKED  Common vertex buffer.
  ///         Sets the shared vertex buffer.
  static void renderHACKSetCommonVBO(Hub& hub,
                                     uint8_t* vertexBuffer, size_t vboSize);

  /// HACKED  Uniform color face attributes.
  ///         Once your the buffer is passed into this function, spire assumes
  ///         all ownership of it. It will call delete[] on the buffer.
  static void renderHACKSetUCFace(Hub& hub,
                                  uint8_t* indexBuffer, size_t iboSize);

  /// HACKED  Uniform face color
  static void renderHACKSetUCFaceColor(Hub& hub, const V4& color);

  /// HACKED  Uniform color edge attributes.
  static void renderHACKSetUCEdge(Hub& hub,
                                  uint8_t* indexBuffer, size_t iboSize);

  /// HACKED  Uniform face color
  static void renderHACKSetUCEdgeColor(Hub& hub, const V4& color);

  /// HACKED  Set use ztest
  static void renderHACKSetUseZTest(Hub& hub, bool useZTest);

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
