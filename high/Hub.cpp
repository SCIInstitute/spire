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

#include <sstream>
#include <thread>

#include "Hub.h"

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
}

//------------------------------------------------------------------------------
Hub::~Hub()
{
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
  std::stringstream s;

  s << "OpenGL initialization. Running on a "
    << vendor << " " << renderer << " with OpenGL version " << versionl;
  logMessage(s.str());

  s.str("");
  s << "GL made current on thread " << std::this_thread::get_id();
  logMessage(s.str());
  
  // Pulled from Tuvok.
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
void Hub::logDebug(const std::string& msg)
{
  mLogFP(msg, Interface::LOG_DEBUG);
}
 
//------------------------------------------------------------------------------
void Hub::logMessage(const std::string& msg)
{
  mLogFP(msg, Interface::LOG_MESSAGE);
}

//------------------------------------------------------------------------------
void Hub::logWarning(const std::string& msg)
{
  mLogFP(msg, Interface::LOG_WARNING);
}

//------------------------------------------------------------------------------
void Hub::logError(const std::string& msg)
{
  mLogFP(msg, Interface::LOG_ERROR);
}

//------------------------------------------------------------------------------
void Hub::logFunction(const std::string& msg, Interface::LOG_LEVEL level)
{
  // std::endl will flush the buffer.
  switch (level)
  {
    case Interface::LOG_DEBUG:
      mOutputFile << "Debug: " << msg << std::endl;
      break;

    case Interface::LOG_MESSAGE:
      mOutputFile << "General: " << msg << std::endl;
      break;

    case Interface::LOG_WARNING:
      mOutputFile << "Warning: " << msg << std::endl;
      break;

    case Interface::LOG_ERROR:
      mOutputFile << "Error: " << msg << std::endl;
      break;
  }
}

} // end of namespace Spire
