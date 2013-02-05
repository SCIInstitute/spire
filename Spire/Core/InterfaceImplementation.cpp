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
#include "Core/HackedUCRenderer.h"
#include "InterfaceImplementation.h"

namespace Spire {


//------------------------------------------------------------------------------
bool InterfaceImplementation::addFunctionToQueue(const Hub::RemoteFunction& fun)
{
#ifdef SPIRE_USE_STD_THREADS
  return mQueue.push(ThreadMessage(fun));
#else
  mQueue.push(ThreadMessage(fun));
  return true;
#endif
}

//------------------------------------------------------------------------------
void InterfaceImplementation::executeQueue(Hub& hub)
{
#ifdef SPIRE_USE_STD_THREADS
  ThreadMessage msg;
  while (mQueue.pop(msg))
  {
    msg.execute(hub);
  }
#else
  while (!mQueue.empty())
  {
    mQueue.front().execute(hub);
    mQueue.pop();
  }
#endif
}

//------------------------------------------------------------------------------
void InterfaceImplementation::cameraSetTransform(Hub& hub, M44 transform)
{
  hub.getCamera()->setViewTransform(transform);
}

//------------------------------------------------------------------------------
void InterfaceImplementation::renderHACKSetUCEdge(Hub& hub,
                                                  uint8_t* vertexBuffer, size_t vboSize,
                                                  uint8_t* indexBuffer, size_t iboSize)
{
  // Destroy any previous vertex / index buffers
  hub.getHackedRenderer()->setEdgeData(vertexBuffer, vboSize, indexBuffer, iboSize);
}

//------------------------------------------------------------------------------
void InterfaceImplementation::renderHACKSetUCEdgeColor(Hub& hub, const V4& color)
{
  hub.getHackedRenderer()->setEdgeColor(color);
}

//------------------------------------------------------------------------------
void InterfaceImplementation::renderHACKSetUCFace(Hub& hub,
                                                  uint8_t* vertexBuffer, size_t vboSize,
                                                  uint8_t* indexBuffer, size_t iboSize)
{
  hub.getHackedRenderer()->setFaceData(vertexBuffer, vboSize, indexBuffer, iboSize);
}

//------------------------------------------------------------------------------
void InterfaceImplementation::renderHACKSetUCFaceColor(Hub& hub, const V4& color)
{
  hub.getHackedRenderer()->setFaceColor(color);
}


} // end of namespace Spire

