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

#ifndef SPIRE_HIGH_HUB_H
#define SPIRE_HIGH_HUB_H

#include <iostream>
#include <fstream>
#include <functional>
#include <list>

#include "../Interface.h"
#include "ShaderUniformStateMan.h"  // We want the interface to see this (to retrieve global uniforms).
#include "PassUniformStateMan.h"

namespace CPM_SPIRE_NS {

class Log;
class PipeDriver;
class InterfaceImplementation;
class ShaderMan;
class ShaderAttributeMan;
class ShaderUniformMan;
class ShaderProgramMan;

/// Central hub for the renderer.
/// Most managers will reference this class in some way.
class Hub
{
public:

  /// @todo Make context a shared_ptr
  Hub(std::shared_ptr<Context> context,
      const std::vector<std::string>& shaderDirs, 
      Interface::LogFunction logFn);
  virtual ~Hub();

  /// Definition of what a remote function should accept.
  typedef std::function<void (InterfaceImplementation& impl)> RemoteFunction;

  /// One-time initialization of the renderer.
  void oneTimeInit();

  /// Retrieves shader manager.
  ShaderMan& getShaderManager()                   {return *mShaderMan;}

  /// Retrieves shader attribute manager.
  ShaderAttributeMan& getShaderAttributeManager() {return *mShaderAttributes;}

  /// Retrieves shader uniform manager.
  ShaderUniformMan& getShaderUniformManager()     {return *mShaderUniforms;}

  /// Retrieves global shader uniform *state* manager.
  ShaderUniformStateMan& getGlobalUniformStateMan() {return *mShaderUniformStateMan;}

  /// Retrieves pass shader uniform *state* manager.
  PassUniformStateMan& getPassUniformStateMan()   {return *mPassUniformStateMan;}

  /// Retrieves the shader program manager.
  ShaderProgramMan& getShaderProgramManager()     {return *mShaderProgramMan;}

  /// Retrieves the actual screen width in pixels.
  size_t getActualScreenWidth() const             {return mPixScreenWidth;}

  /// Retrieves the actual screen width in pixels.
  size_t getActualScreenHeight() const            {return mPixScreenHeight;}

  /// Retrieve list of directories in which to search for shaders.
  const std::vector<std::string>& getShaderDirs() const {return mShaderDirs;}

  /// Retrieves the interface implementation.
  std::shared_ptr<InterfaceImplementation> getInterfaceImpl() {return mInterfaceImpl;}

  //----------------------------------------------------------------------------
  // Concurrent helper functions
  //----------------------------------------------------------------------------
  // Some of these functions may need to be moved into interface implementation.
  // At least those that don't require access to the context (both begin and
  // end frame require access to the graphics context).
  bool beginFrame(bool makeContextCurrent);
  void endFrame();

  /// Make context current.
  void makeCurrent();

private:

  Interface::LogFunction              mLogFun;          ///< Log function.
  std::unique_ptr<Log>                mLog;             ///< Spire logging class.
  std::shared_ptr<Context>            mContext;         ///< Rendering context.
  std::unique_ptr<ShaderMan>          mShaderMan;       ///< Shader manager.
  std::unique_ptr<ShaderAttributeMan> mShaderAttributes;///< Shader attribute manager.
  std::unique_ptr<ShaderProgramMan>   mShaderProgramMan;///< Shader program manager.
  std::unique_ptr<ShaderUniformMan>   mShaderUniforms;  ///< Shader attribute manager.
  std::unique_ptr<ShaderUniformStateMan> mShaderUniformStateMan; ///< Uniform state manager.
  std::unique_ptr<PassUniformStateMan>mPassUniformStateMan;///< Shader manager for pass'.
  std::vector<std::string>            mShaderDirs;      ///< Shader directories to search.

  std::shared_ptr<InterfaceImplementation>  mInterfaceImpl; ///< Interface implementation.

  size_t                  mPixScreenWidth;  ///< Actual screen width in pxels.
  size_t                  mPixScreenHeight; ///< Actual screen height in pixels.
  
};

} // namespace CPM_SPIRE_NS

#endif // SPIRE_HIGH_HUB_H

