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

#ifndef SPIRE_HIGH_THREADMESSAGE_H
#define SPIRE_HIGH_THREADMESSAGE_H

#include <queue>

#include "Core/Hub.h"

namespace Spire
{

/// Encapsulates a function that will be executed on a remote thread.
/// Used instead of an inbox along with message ID's and whatnot.
/// std::bind along with std::function are used to execute functions on the 
/// remote thread, allowing for arbitrary parameters and circumventing the 
/// need for message passing.
class ThreadMessage
{
public:
  /// The purposes of using std::function is so that all parameters can be
  /// bound using std::bind.
  ThreadMessage();
  ThreadMessage(const Hub::RemoteFunction& toCall);
  virtual ~ThreadMessage()  {}
  
  /// Sets message function to call (rvalue reference so we can move construct?)
  void setFunction(const Hub::RemoteFunction& toCall);

  /// Execute function stored in message.
  void execute(Hub& hub);

private:

  Hub::RemoteFunction mToCall;
};

} // namespace Spire

#endif 
