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
#include <thread>

#include "Common.h"
#include "High/Hub.h"
#include "High/Log.h"

#include "Pipes/StuPipe/Driver.h"

static int __thread testing = 0;

namespace Spire {

//------------------------------------------------------------------------------
Hub::Hub(Context* context, LogFunction logFn) :
    mContext(context),
    mPipe(new StuPipe::Driver(*this)),
    mLog(new Log(logFn))
{

}

//------------------------------------------------------------------------------
Hub::~Hub()
{
}

//------------------------------------------------------------------------------
void Hub::oneTimeGLInit()
{
  mContext->makeCurrent();

  // Initialize OpenGL
  glClearColor(0.0, 0.0, 0.0, 1.0);

  const GLubyte* vendor     = glGetString(GL_VENDOR);
  const GLubyte* renderer   = glGetString(GL_RENDERER);
  const GLubyte* versionl   = glGetString(GL_VERSION);

  logMessage() << "OpenGL initialization. Running on a " << vendor << " " 
               << renderer << " with OpenGL version " << versionl << std::endl
               << "GL made current on thread " << std::this_thread::get_id()
               << std::endl;

	GLint tmp;
  logDebug() << "Hardware specific attributes" << std::endl;
  logDebug() << "+Programmable:" << std::endl;

	glGetIntegerv(GL_MAX_TEXTURE_UNITS, &tmp);
  logDebug() << "  Texture Units: " << tmp << std::endl;

  logDebug() << "+Fixed function (transient):" << std::endl;
	glGetIntegerv(GL_MAX_MODELVIEW_STACK_DEPTH, &tmp);
  logDebug() << "  Model view stack depth: " << tmp << std::endl;
	
	glGetIntegerv(GL_MAX_PROJECTION_STACK_DEPTH, &tmp);
  logDebug() << "  Projection stack depth: " << tmp << std::endl;
	
	glGetIntegerv(GL_MAX_TEXTURE_STACK_DEPTH, &tmp);
  logDebug() << "  Texture stack depth: " << tmp << std::endl;

  /// TODO: Add GPU memory checks using GL_NVX_gpu_memory_info for NVIDIA
  ///       and GL_ATI_meminfo for ATI.

  // Pulled from Tuvok
  //const bool bOpenGLSO12     = atof((const char*)versionl) >= 1.2;
  //const bool bOpenGLSO20     = atof((const char*)versionl) >= 2.0;
}

//------------------------------------------------------------------------------
void Hub::doFrame()
{
  // When we get here, it has already been determined that a frame needs to be
  // redrawn.

  // Question: How do we split up the pipes? Does everything happen in the
  // stupipe, when we route through the stupipe? It almost appears that it has
  // to. Should we just build the pipe and see where it leads us to?

  // Rudimentary doFrame...
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  mPipe->doFrame();

  mContext->swapBuffers();
}

//------------------------------------------------------------------------------
std::ostream& Hub::logDebug()
{
  return mLog->getDebugStream();
}

//------------------------------------------------------------------------------
std::ostream& Hub::logMessage()
{
  return mLog->getMessageStream();
}

//------------------------------------------------------------------------------
std::ostream& Hub::logWarning()
{
  return mLog->getWarningStream();
}

//------------------------------------------------------------------------------
std::ostream& Hub::logError()
{
  return mLog->getErrorStream();
}

} // end of namespace Spire
