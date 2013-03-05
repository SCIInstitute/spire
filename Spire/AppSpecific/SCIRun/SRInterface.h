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

#ifndef SPIRE_APPSPECIFIC_SCIRUN_SCIRUNINTERFACE_H
#define SPIRE_APPSPECIFIC_SCIRUN_SCIRUNINTERFACE_H

#include "../../Interface.h"
#include "../../StuPipe/StuInterface.h"

namespace Spire {
namespace SCIRun {

class ArcBall;

/// A wrapper around spire that provides higher level functionality required
/// to operate SCIRun.
/// \todo Think about how we are going to break apart this class.
///       There will be a lot of functionality behind it.
class SRInterface : public Interface
{
public:
  SRInterface(std::shared_ptr<Context> context,
              const std::vector<std::string>& shaderDirs,
              bool createThread, LogFunction logFP = LogFunction());
  virtual ~SRInterface();

  void eventResize(int32_t width, int32_t height);

  /// \todo Specify what buttons are pressed.
  /// @{
  void inputMouseDown(const Vector2<int32_t>& pos);
  void inputMouseMove(const Vector2<int32_t>& pos);
  void inputMouseUp(const Vector2<int32_t>& pos);
  /// @}

  void inputMouseWheel(int32_t delta);

  /// Stupipe interface. This is the pipe that SCIRun uses to render its
  /// geometry.
  std::shared_ptr<Spire::StuInterface> getStuPipe() {return mStuInterface;}

  /// \todo Selecting objects...

  /// \todo Obtaining data from mesh objects in order to spatially partition
  ///       them and provide quick object feedback.

private:

  std::shared_ptr<Spire::StuInterface>    mStuInterface;

  Vector2<int>              mWindowDims;    ///< Current window dimensions.

  std::unique_ptr<ArcBall>  mArcBall;       ///< ArcBall delta transform calculator.
  M44                       mCamWorld;      ///< Camera in world space.

  float                     mCamDistance;   ///< Camera's distance from the origin.
};

} // namespace SCIRun
} // namespace Spire

#endif 
