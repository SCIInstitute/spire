/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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
/// \date   October 2012

#include <string>
#include <sstream>

#include "../Exceptions.h"

#include "Hub.h"
#include "HubThread.h"

namespace Spire {

//------------------------------------------------------------------------------
HubThread::HubThread(Hub* hub) :
    mHub(hub),
    mThreadKill(false),
    mThreadRunning(false)
{
}

//------------------------------------------------------------------------------
HubThread::~HubThread()
{
}

//------------------------------------------------------------------------------
bool HubThread::isRendererThreadRunning()
{
  return mThreadRunning.load();
}

//------------------------------------------------------------------------------
void HubThread::createRendererThread()
{
  mHub->logMessage() << "Creating renderer thread." << std::endl;
  if (isRendererThreadRunning() == true)
    throw ThreadException("Cannot create new renderer thread; a renderer "
                          "thread is currently running.");

  mThread = std::thread(&HubThread::rendererThread, this);
}

//------------------------------------------------------------------------------
void HubThread::killRendererThread()
{
  mHub->logMessage() << "Killing rendering thread." << std::endl;
  if (isRendererThreadRunning() == false)
    throw ThreadException("Cannot kill renderer thread; the thread is not "
                          "currently running.");

  // The following join statement could possibly throw a system_error if 
  // mThread.get_id() == std::thread::id()
  mThreadKill.store(true);
  mThread.join();
}

//------------------------------------------------------------------------------
void HubThread::rendererThread()
{
  mThreadRunning.store(true);

  mHub->oneTimeGLInit();

  while (mThreadKill.load() == false)
  {
    mHub->doFrame();
    /// @todo Add logic to determine how long we should sleep for...
    ///       This will be highly dependent on how long it took to render the
    ///       last frame, and if we are in the middle of compositing the final
    ///       frame together.
    std::chrono::milliseconds dur(10);
    std::this_thread::sleep_for(dur);
  }

  mThreadRunning.store(false);
}

}
