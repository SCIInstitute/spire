/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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
/// \date   December 2012

#include "Common.h"
#include "Driver.h"

namespace Spire { 
namespace StuPipe {

//------------------------------------------------------------------------------
Driver::Driver(Hub& hub) :
    PipeDriver(hub)
    //mUniformColorTest(hub)
{
  mInitialState.mDepthTestEnable = true;
  mInitialState.mCullFaceEnable = false;  // Todo: Set to true for geometry.
                                          // Should not be true for volumes.
  mInitialState.mBlendEnable = true;
  glClearColor(0.3f, 0.0f, 0.3f, 1.0f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
}

//------------------------------------------------------------------------------
Driver::~Driver()
{
}

//------------------------------------------------------------------------------
void Driver::doFrame()
{
  // Clear the screen
  glClearColor(0.3f, 0.0f, 0.3f, 1.0f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  //mView = mView * M44::rotationY(0.1f);
  //mHub.getCamera()->setViewTransform(mView);

  // Force a known GPU state
  mHub.getGPUStateManager().apply(mInitialState, true);
  mHub.getHackedRenderer()->doFrame();

  // Render a latvolume...

  // Render a volume...

}

} } // end of namespace Spire::StuPipe
