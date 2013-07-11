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
#include "Core/Hub.h"
#include "Core/Log.h"
#include "Core/InterfaceImplementation.h"

using namespace std::placeholders;

namespace Spire {

//------------------------------------------------------------------------------
Interface::Interface(std::shared_ptr<Context> context,
                     const std::vector<std::string>& shaderDirs,
                     bool createThread, LogFunction logFP) :
    mHub(new Hub(context, shaderDirs, logFP, createThread))
{
}

//------------------------------------------------------------------------------
Interface::~Interface()
{
}

//------------------------------------------------------------------------------
void Interface::terminate()
{
  if (mHub->isRendererThreadRunning())
  {
    mHub->killRendererThread();
  }
}

//------------------------------------------------------------------------------
void Interface::ntsDoFrame()
{
  if (mHub->isRendererThreadRunning())
    throw ThreadException("You cannot call doFrame when the renderer is "
                          "running in a separate thread.");

  mHub->doFrame();
}

//------------------------------------------------------------------------------
void Interface::addPassToFront(const std::string& passName)
{
  Hub::RemoteFunction fun =
      std::bind(InterfaceImplementation::addPassToFront, _1, passName);
  mHub->addFunctionToThreadQueue(fun);
}

//------------------------------------------------------------------------------
void Interface::addPassToBack(const std::string& passName)
{
  Hub::RemoteFunction fun =
      std::bind(InterfaceImplementation::addPassToBack, _1, passName);
  mHub->addFunctionToThreadQueue(fun);
}

//------------------------------------------------------------------------------
void Interface::addObject(const std::string& objectName)
{
  Hub::RemoteFunction fun =
      std::bind(InterfaceImplementation::addObject, _1, objectName);
  mHub->addFunctionToThreadQueue(fun);
}

//------------------------------------------------------------------------------
void Interface::removeObject(const std::string& objectName)
{
  Hub::RemoteFunction fun =
      std::bind(InterfaceImplementation::removeObject, _1, objectName);
  mHub->addFunctionToThreadQueue(fun);
}

//------------------------------------------------------------------------------
void Interface::removeAllObjects()
{
  Hub::RemoteFunction fun =
      std::bind(InterfaceImplementation::removeAllObjects, _1);
  mHub->addFunctionToThreadQueue(fun);
}

//------------------------------------------------------------------------------
void Interface::addVBO(const std::string& name,
                       std::shared_ptr<std::vector<uint8_t>> vboData,
                       const std::vector<std::string>& attribNames)
{
  Hub::RemoteFunction fun =
      std::bind(InterfaceImplementation::addVBO, _1, name, vboData, attribNames);
  mHub->addFunctionToThreadQueue(fun);
}

//------------------------------------------------------------------------------
void Interface::removeVBO(const std::string& vboName)
{
  Hub::RemoteFunction fun =
      std::bind(InterfaceImplementation::removeVBO, _1, vboName);
  mHub->addFunctionToThreadQueue(fun);
}

} // end of namespace Renderer

