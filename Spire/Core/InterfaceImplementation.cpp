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

#include "Core/Hub.h"
#include "InterfaceImplementation.h"

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
      msg.execute(mHub);
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
void InterfaceImplementation::pipePushBack(Hub& hub,
                                           std::shared_ptr<PipeInterface> pipe)
{
  hub.addPipe(pipe);
}
  
//------------------------------------------------------------------------------
void InterfaceImplementation::pipeRemove(Hub& hub,
                                         std::shared_ptr<PipeInterface> pipe)
{
  hub.removePipe(pipe);
}


//------------------------------------------------------------------------------
void InterfaceImplementation::resize(Hub& hub, size_t width, size_t height)
{
  GL(glViewport(0, 0, width, height));
}

} // end of namespace Spire

