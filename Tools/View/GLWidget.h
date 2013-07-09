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

#ifndef SPIRE_GLWIDGET_H
#define SPIRE_GLWIDGET_H

#define NOMINMAX

#include <QtOpenGL/QGLWidget>
#include <QTimer>

#include "GLContext.h"

// From spire
#include "Spire/Interface.h"

class GLWidget : public QGLWidget
{
  Q_OBJECT

public:
  GLWidget(const QGLFormat& format);

protected:
  void resizeEvent(QResizeEvent *evt);
  void closeEvent(QCloseEvent *evt);
  virtual void mousePressEvent(QMouseEvent* event);
  virtual void mouseMoveEvent(QMouseEvent* event);
  virtual void mouseReleaseEvent(QMouseEvent* event);

  void buildScene();

protected slots:
  void updateRenderer();

private:

  std::shared_ptr<GLContext>            mContext;
  std::shared_ptr<Spire::Interface>     mSpire;
  std::shared_ptr<Spire::StuInterface>  mStuInterface;
  Spire::Vector2<int>                   mLastMousePos;
  Spire::M44                            mCamWorld;

#ifndef SPIRE_USE_STD_THREADS
  QTimer*                           mTimer;
#endif

};


#endif // SPIRE_GLWIDGET_H
