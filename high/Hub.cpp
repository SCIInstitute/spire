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

#include "Hub.h"
#include "Log.h"

namespace Spire {

//------------------------------------------------------------------------------
Hub::Hub(Context* context, LogFunction logFn) :
    mContext(context),
    mLogFP(logFn)
{
  if (mLogFP == nullptr)
  {
#ifndef WIN32
    std::stringstream osFilename;
    osFilename << "/tmp/SpireLog";//_" << std::this_thread::get_id();
    mOutputFile.open(osFilename.str());
    mLogFP = std::bind(&Hub::logFunction, this,
                       std::placeholders::_1, std::placeholders::_2);
#endif
  }

  mLog = std::unique_ptr<Log>(new Log(mLogFP));
}

//------------------------------------------------------------------------------
Hub::~Hub()
{
  logMessage() << "Terminating spire output." << std::endl;
  if (mOutputFile.is_open())
    mOutputFile.close();
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
  // Rudimentary doFrame...
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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


//------------------------------------------------------------------------------
void Hub::logFunction(const std::string& msg, Interface::LOG_LEVEL level)
{
  switch (level)
  {
    case Interface::LOG_DEBUG:
      mOutputFile << "Debug:   " << msg;
      break;

    case Interface::LOG_MESSAGE:
      mOutputFile << "General: " << msg;
      break;

    case Interface::LOG_WARNING:
      mOutputFile << "Warning: " << msg;
      break;

    case Interface::LOG_ERROR:
      mOutputFile << "Error:   " << msg;
      break;
  }
  mOutputFile.flush();
}

} // end of namespace Spire
