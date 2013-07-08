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
/// \brief  Not sure this file should go in Modules/Render. But it is an 
///         auxiliary file to the ViewScene render module.

#ifndef INTERFACE_MODULES_GLWIDGET_H
#define INTERFACE_MODULES_GLWIDGET_H

#include <QtOpenGL/QGLWidget>
#include <QTimer>

#include "GLContext.h"
#include "Spire/Interface.h"
#include "Spire/AppSpecific/SCIRun/SRInterface.h"
#include "Spire/AppSpecific/SCIRun/SRCommonAttributes.h"
#include "Spire/AppSpecific/SCIRun/SRCommonUniforms.h"
#include "Spire/StuPipe/StuInterface.h"

namespace SCIRun {
namespace Gui {
  
class GLWidget : public QGLWidget
{
  Q_OBJECT

public:
  GLWidget(const QGLFormat& format);
  ~GLWidget();

  std::shared_ptr<Spire::SCIRun::SRInterface> getSpire() const      {return mSpire;}
  std::shared_ptr<GLContext>                  getContext() const    {return mContext;}

protected:
  virtual void closeEvent(QCloseEvent *evt);
  virtual void mousePressEvent(QMouseEvent* event);
  virtual void mouseMoveEvent(QMouseEvent* event);
  virtual void mouseReleaseEvent(QMouseEvent* event);
  virtual void wheelEvent(QWheelEvent * event);
  virtual void initializeGL();
  virtual void resizeGL(int width, int height);

  void buildScene();

protected slots:
  // Only used when not using std threads.
  void updateRenderer();

private:

  /// Retrieve SRInterface mouse button from mouse event.
  Spire::SCIRun::SRInterface::MouseButton getSpireButton(QMouseEvent* event);

  std::shared_ptr<GLContext>                  mContext;       ///< Graphics context.
  std::shared_ptr<Spire::SCIRun::SRInterface> mSpire;         ///< Interface to spire.

#ifndef SPIRE_USE_STD_THREADS
  QTimer*                                     mTimer;
#endif

};

} // end of namespace SCIRun
} // end of namespace Gui

#endif // SPIRE_GLWIDGET_H
