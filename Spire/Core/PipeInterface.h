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

#ifndef SPIRE_CORE_PIPEINTERFACE_H
#define SPIRE_CORE_PIPEINTERFACE_H

#include "../InterfaceCommon.h"
#include "../Interface.h"

namespace Spire
{

class Hub;

/// \todo Figure out the basic interface to have for pipes.
///       After iterating through a few pipe implementations, the details
///       should become more clear.
/// It will be mandatory to pointer cast this pipe to its derived
/// type in order to access the majority of the interface.
class WIN_DLL PipeInterface
{
public:
  // Does NOT store a reference to the interface...
  PipeInterface(Interface& iface) :
      mSubmittedToSpire(false),
      mHub(*iface.mHub)
  {}

  virtual ~PipeInterface()                    {}
  
  //============================================================================
  // THREAD SAFE
  //============================================================================

  /// Enables the 'submitted to spire' flag.
  void setSubmitted(bool submitted)   {mSubmittedToSpire = false;}

  /// Returns whether or not this interface has been submitted to spire.
  bool hasBeenSubmitted()             {return mSubmittedToSpire;}

  //============================================================================
  // NOT THREAD SAFE
  //============================================================================
  // Be sure that you are calling these functions from the thread upon which
  // spire is executing. All non-thread-safe functions are prefixed with 'nts'.

  /// Called first thing when the renderer receives the pipe.
  virtual void ntsInitOnRenderThread() = 0;

  /// \todo Figure out time allocations for renderers. We need a way of 
  ///       compositing frames together if it's clear that a pass is taking
  ///       too long. This may necessitate the pipes working together.
  ///       For an example, think of the Tuvok pipe that we will need to
  ///       build... Tuvok will either be it's own pipe, or integrated into
  ///       another pipe. Ideally, Tuvok would work together to get transparency
  ///       working correctly, but that is not going to happen in the short run.
  /// Perform a rendering pass.
  virtual void ntsDoPass() = 0;

  /// Cleans up all GL resources.
  virtual void clearGLResources() = 0;

protected:
  /// Reference to the hub class. Should only be used when on the renderer
  /// thread.
  Hub&  mHub;

private:

  /// Simple boolean value indicating whether or not this pipe has been 
  /// submitted to Spire. If it has not, then we should error out when making
  /// 'thread safe' calls as the spire thread will not recognize them.
  bool mSubmittedToSpire;

};

} // namespace Spire

#endif 
