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

#include "Interface.h"
#include "ThreadMessage.h"

namespace Spire
{

/// Implementation of the functions exposed in Interface.h
class InterfaceImplementation
{
public:
  InterfaceImplementation()           {}
  virtual ~InterfaceImplementation()  {}
  
  /// SHOULD ONLY be called by the thread associated with Queue.
  /// Will add 'fun' to the queue associated with 'thread'.
  /// \return false if we failed to add the function to the specified queue.
  ///         queue is likely to be full.
  bool addFunctionToQueue(Interface::THREAD thread,
                          ThreadMessage::RemoteFunction fun);

  /// SHOULD ONLY be called by the spire thread!
  /// Will execute all commands the the queue associated with Interface::THREAD.
  void executeQueue(Interface::THREAD thread, Hub& hub);

  //============================================================================
  // IMPLEMENTATION
  //============================================================================
  // All of the functions below constitute the implementation of the interface
  // to spire.




private:

#ifdef SPIRE_USE_STD_THREADS
  typedef CircularFifo<ThreadMessage,256> MessageQueue;
#else
  typedef std::queue<ThreadMessage>       MessageQueue;
#endif

  MessageQueue                    mUIQueueIn;     ///< UI messaging queue.
  MessageQueue                    mModuleQueueIn; ///< Module messaging queue.
};

} // namespace Spire

#endif 
