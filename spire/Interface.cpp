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
/// \date   September 2012

#include <sstream>
#include "Interface.h"
#include "Exceptions.h"
#include "High/Hub.h"
#include "High/HubThread.h"

namespace Spire {

//------------------------------------------------------------------------------
Interface::Interface(Context* context, bool createThread) :
    mHub(new Hub(context, Hub::LogFunction())),
    mHubThread(new HubThread(mHub))
{
  if (createThread)
  {
    // Unknown whether there is support for this on other compilers...
    mHubThread->createRendererThread();
  }
  else
  {
    mHub->oneTimeGLInit();
  }
}

//------------------------------------------------------------------------------
Interface::~Interface()
{
  mHub->logMessage() << "Shutting down interface." << std::endl;

  // If the renderer thread is running, attempt to shut it down and join it.
  if (mHubThread->isRendererThreadRunning())
  {
    mHub->logMessage() << "Terminating rendering thread." << std::endl; 
    mHubThread->killRendererThread();
  }

  // Manualy destroy these classes for now (this will be changed to shared 
  // pointers).
  mHub->logMessage() << "Destroying rendering hub." << std::endl; 
  delete mHub;
  delete mHubThread;
}

//------------------------------------------------------------------------------
void Interface::doFrame()
{
  if (mHubThread->isRendererThreadRunning())
    throw ThreadException("You cannot call doFrame when the renderer is "
                          "running in a separate thread.");

  mHub->doFrame();
}

//------------------------------------------------------------------------------
void Interface::killRendererThread()
{
  mHubThread->killRendererThread();
}

//------------------------------------------------------------------------------
void Interface::createRendererThread()
{
  mHubThread->createRendererThread();
}

//------------------------------------------------------------------------------
bool Interface::isRendererThreadRunning()
{
  return mHubThread->isRendererThreadRunning();
}


} // end of namespace Renderer
