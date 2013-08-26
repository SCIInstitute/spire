/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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
/// \date   December 2012

#include "Common.h"
#include "GPUStateManager.h"

namespace Spire {

// All of these functions can be made into a 2D array (mimicing a 1-1 function).

//------------------------------------------------------------------------------
GLenum CULL_ORDERToGL(CULL_ORDER order)
{
  switch (order)
  {
    case ORDER_CCW:                 return GL_CCW;
    case ORDER_CW:                  return GL_CW;
  }
  return GL_CCW;  // CCW is GL's default
}

//------------------------------------------------------------------------------
CULL_ORDER GLToCULL_ORDER(GLenum order)
{
  switch (order)
  {
    case GL_CCW:                    return ORDER_CCW;
    case GL_CW:                     return ORDER_CW;
  }
  return ORDER_CCW;
}

//------------------------------------------------------------------------------
GLenum BLEND_FUNCToGL(BLEND_FUNC func)
{
  switch (func) 
  {
    case BF_ZERO:                   return GL_ZERO;
    case BF_ONE:                    return GL_ONE;
    case BF_SRC_COLOR:              return GL_SRC_COLOR;
    case BF_ONE_MINUS_SRC_COLOR:    return GL_ONE_MINUS_SRC_COLOR;
    case BF_DST_COLOR:              return GL_DST_COLOR;
    case BF_ONE_MINUS_DST_COLOR:    return GL_ONE_MINUS_DST_COLOR;
    case BF_SRC_ALPHA:              return GL_SRC_ALPHA;
    case BF_ONE_MINUS_SRC_ALPHA:    return GL_ONE_MINUS_SRC_ALPHA;
    case BF_DST_ALPHA:              return GL_DST_ALPHA;
    case BF_ONE_MINUS_DST_ALPHA:    return GL_ONE_MINUS_DST_ALPHA;
    case BF_SRC_ALPHA_SATURATE:     return GL_SRC_ALPHA_SATURATE;
  }
  return GL_ONE;
}

//------------------------------------------------------------------------------
BLEND_FUNC GLToBLEND_FUNC(GLenum func) 
{
  switch (func) 
  {
    case GL_ZERO:                   return BF_ZERO;
    case GL_ONE:                    return BF_ONE;
    case GL_SRC_COLOR:              return BF_SRC_COLOR;
    case GL_ONE_MINUS_SRC_COLOR:    return BF_ONE_MINUS_SRC_COLOR;
    case GL_DST_COLOR:              return BF_DST_COLOR;
    case GL_ONE_MINUS_DST_COLOR:    return BF_ONE_MINUS_DST_COLOR;
    case GL_SRC_ALPHA:              return BF_SRC_ALPHA;
    case GL_ONE_MINUS_SRC_ALPHA:    return BF_ONE_MINUS_SRC_ALPHA;
    case GL_DST_ALPHA:              return BF_DST_ALPHA;
    case GL_ONE_MINUS_DST_ALPHA:    return BF_ONE_MINUS_DST_ALPHA;
    case GL_SRC_ALPHA_SATURATE:     return BF_SRC_ALPHA_SATURATE;
  }
  return BF_ONE;
}

//------------------------------------------------------------------------------
GLenum BLEND_EQToGL(BLEND_EQ func) 
{
  switch (func) 
  {
    case BE_FUNC_ADD:               return GL_FUNC_ADD;
    case BE_FUNC_SUBTRACT:          return GL_FUNC_SUBTRACT;
    case BE_FUNC_REVERSE_SUBTRACT:  return GL_FUNC_REVERSE_SUBTRACT;
#ifdef SPIRE_OPENGL_ES_2
    case BE_MIN:                    Log::error() << "GL_MIN not supported in ES 2.0" << std::endl; break;
    case BE_MAX:                    Log::error() << "GL_MAX not supported in ES 2.0" << std::endl; break;
#else
    case BE_MIN:                    return GL_MIN;
    case BE_MAX:                    return GL_MAX;
#endif
  }
  return GL_FUNC_ADD;
}

//------------------------------------------------------------------------------
BLEND_EQ GLToBLEND_EQ(const GLenum& func) 
{
  switch (func) 
  {
    case GL_FUNC_ADD:               return BE_FUNC_ADD;
    case GL_FUNC_SUBTRACT:          return BE_FUNC_SUBTRACT;
    case GL_FUNC_REVERSE_SUBTRACT:  return BE_FUNC_REVERSE_SUBTRACT;
#ifndef SPIRE_OPENGL_ES_2
    case GL_MIN:                    return BE_MIN;
    case GL_MAX:                    return BE_MAX;
#endif
  }
  return BE_FUNC_ADD;
}

//------------------------------------------------------------------------------
GLenum DEPTH_FUNCToGL(const DEPTH_FUNC& func) 
{
  switch (func) 
  {
    case DF_NEVER:                  return GL_NEVER;
    case DF_LESS:                   return GL_LESS;
    case DF_EQUAL:                  return GL_EQUAL;
    case DF_LEQUAL:                 return GL_LEQUAL;
    case DF_GREATER:                return GL_GREATER;
    case DF_NOTEQUAL:               return GL_NOTEQUAL;
    case DF_GEQUAL:                 return GL_GEQUAL;
    case DF_ALWAYS:                 return GL_ALWAYS;
  }
  return GL_LEQUAL;
}

//------------------------------------------------------------------------------
DEPTH_FUNC GLToDEPTH_FUNC(const GLenum& func) 
{
  switch (func) 
  {
    case GL_NEVER:                  return DF_NEVER;
    case GL_LESS:                   return DF_LESS;
    case GL_EQUAL:                  return DF_EQUAL;
    case GL_LEQUAL:                 return DF_LEQUAL;
    case GL_GREATER:                return DF_GREATER;
    case GL_NOTEQUAL:               return DF_NOTEQUAL;
    case GL_GEQUAL:                 return DF_GEQUAL;
    case GL_ALWAYS:                 return DF_ALWAYS;
  }
  return DF_LEQUAL;
}


//------------------------------------------------------------------------------
GPUStateManager::GPUStateManager()
{
}

//------------------------------------------------------------------------------
GPUStateManager::~GPUStateManager()
{
}

//------------------------------------------------------------------------------
size_t GPUStateManager::getMaxTextureUnits() const
{
  GLint tmp;
#ifdef SPIRE_OPENGL_ES_2
  glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &tmp);
#else
  glGetIntegerv(GL_MAX_TEXTURE_UNITS, &tmp);
#endif
  return static_cast<size_t>(tmp);
}

//------------------------------------------------------------------------------
void GPUStateManager::apply(const GPUState& state, bool force)
{
  GL_CHECK();

  setDepthTestEnable(state.mDepthTestEnable, force);
  setDepthFunc(state.mDepthFunc, force);
  setCullFaceEnable(state.mCullFaceEnable, force);
  setCullState(state.mCullState, force);
  setBlendEnable(state.mBlendEnable, force);
  setCullFrontFaceOrder(state.mCullOrder, force);
  setLineWidth(state.mLineWidth, force);
  setLineSmoothingEnable(state.mLineSmoothing, force);

  // Do this by hand to avoid the redundant glActiveTexture calls
  /// \todo Grab the maximum number of texture units and use that instead...
  for (size_t i = 0; i < getMaxTextureUnits();  i++) 
  {
    if (force || state.mTexEnable[i] != mInternalState.mTexEnable[i]) 
    {
      glActiveTexture(GLenum(GL_TEXTURE0+i));
      mInternalState.mTexEnable[i] = state.mTexEnable[i];
      switch (mInternalState.mTexEnable[i])
      {
#ifdef SPIRE_OPENGL_ES_2
        case TEX_1D:      Log::error() << "1D textures not supported in ES 2.0" << std::endl;
                          glDisable(GL_TEXTURE_2D);
                          glDisable(GL_TEXTURE_CUBE_MAP);
                          break;
        case TEX_2D:      glDisable(GL_TEXTURE_CUBE_MAP);
                          glEnable(GL_TEXTURE_2D);
                          break;
        case TEX_3D:      Log::error() << "3D textures not supported in ES 2.0" << std::endl;
                          glDisable(GL_TEXTURE_CUBE_MAP);
                          glDisable(GL_TEXTURE_2D);
                          break;
        case TEX_NONE:    glDisable(GL_TEXTURE_2D);
                          glDisable(GL_TEXTURE_CUBE_MAP);
                          break;
#else
        case TEX_1D:      glDisable(GL_TEXTURE_2D);
                          glDisable(GL_TEXTURE_3D);
                          glDisable(GL_TEXTURE_CUBE_MAP);
                          glEnable(GL_TEXTURE_1D);
                          break;
        case TEX_2D:      glDisable(GL_TEXTURE_3D);
                          glDisable(GL_TEXTURE_CUBE_MAP);
                          glEnable(GL_TEXTURE_2D);
                          break;
        case TEX_3D:      glDisable(GL_TEXTURE_CUBE_MAP);
                          glDisable(GL_TEXTURE_1D);
                          glDisable(GL_TEXTURE_2D);
                          glEnable(GL_TEXTURE_3D);
                          break;
        case TEX_NONE:    glDisable(GL_TEXTURE_1D);
                          glDisable(GL_TEXTURE_2D);
                          glDisable(GL_TEXTURE_3D);
                          glDisable(GL_TEXTURE_CUBE_MAP);
                          break;
#endif
      }
    }
  }
  mInternalState.mTexActiveUnit = state.mTexActiveUnit;
  glActiveTexture(GLenum(GL_TEXTURE0 + mInternalState.mTexActiveUnit));

  setDepthMask(state.mDepthMask, force);
  setColorMask(state.mColorMask, force);
  setBlendEquation(state.mBlendEquation, force);
  setBlendFunction(state.mBlendFuncSrc, state.mBlendFuncDst, force);

  GL_CHECK();
}

//------------------------------------------------------------------------------
GPUState GPUStateManager::getStateFromOpenGL() const
{
  GL_CHECK();

  GPUState state;

  GLint e;
  glGetIntegerv(GL_DEPTH_FUNC, &e);
  state.mDepthTestEnable              = glIsEnabled(GL_DEPTH_TEST) != 0;
  state.mDepthFunc                    = GLToDEPTH_FUNC(static_cast<GLenum>(e));

  state.mCullFaceEnable               = glIsEnabled(GL_CULL_FACE) != 0;

  glGetIntegerv(GL_CULL_FACE_MODE, &e);
  state.mCullState                    = (e == GL_FRONT) ? CULL_FRONT : CULL_BACK;

  state.mBlendEnable                  = glIsEnabled(GL_BLEND) != 0;

  glGetIntegerv(GL_FRONT_FACE, &e);
  state.mCullOrder                    = GLToCULL_ORDER(static_cast<GLenum>(e));

  for(size_t i=0; i < getMaxTextureUnits(); ++i)
  {
    glActiveTexture(GL_TEXTURE0 + GLenum(i));
#ifdef SPIRE_OPENGL_ES_2
    if (glIsEnabled(GL_TEXTURE_2D))
    {
      state.mTexEnable[i] = TEX_2D;
    }
    else
    {
      state.mTexEnable[i] = TEX_NONE;
    }
#else
    if (glIsEnabled(GL_TEXTURE_3D))
    {
      state.mTexEnable[i] = TEX_3D;
    }
    else if (glIsEnabled(GL_TEXTURE_2D))
    {
      state.mTexEnable[i] = TEX_2D;
    }
    else if (glIsEnabled(GL_TEXTURE_1D))
    {
      state.mTexEnable[i] = TEX_1D;
    }
    else 
    {
      state.mTexEnable[i] = TEX_NONE;
    }
#endif
  }
  GLboolean	 b;
  glGetBooleanv(GL_DEPTH_WRITEMASK, &b);
  state.mDepthMask = b != 0;

  GLboolean	 col[4];
  glGetBooleanv(GL_COLOR_WRITEMASK, col);
  state.mColorMask = col[0] != 0;  

  GLint src, dest;
#ifdef SPIRE_OPENGL_ES_2
  //GL_BLEND_DST_RGB and GL_BLEND_DST_ALPHA
  glGetIntegerv(GL_BLEND_SRC_RGB, &src);
  glGetIntegerv(GL_BLEND_DST_RGB, &dest);
#else
  glGetIntegerv(GL_BLEND_SRC, &src);
  glGetIntegerv(GL_BLEND_DST, &dest);
#endif
  state.mBlendFuncSrc = GLToBLEND_FUNC(static_cast<GLenum>(src));
  state.mBlendFuncDst = GLToBLEND_FUNC(static_cast<GLenum>(dest));

  GLint equation;
  GL(glGetIntegerv(GL_BLEND_EQUATION_RGB, &equation));
  state.mBlendEquation = GLToBLEND_EQ(static_cast<GLenum>(equation));

  return state;
}

//------------------------------------------------------------------------------
void GPUStateManager::setBlendEnable(bool value, bool force)
{
  if (force || value != mInternalState.mBlendEnable)
  {
    mInternalState.mBlendEnable = value;
    if (mInternalState.mBlendEnable)
    {
      GL(glEnable(GL_BLEND));
    }
    else
    {
      GL(glDisable(GL_BLEND));
    }
  }
}

//------------------------------------------------------------------------------
void GPUStateManager::setBlendEquation(BLEND_EQ value, bool force)
{
  if (force || value != mInternalState.mBlendEquation)
  {
    mInternalState.mBlendEquation = value;
    GL(glBlendEquation(BLEND_EQToGL(mInternalState.mBlendEquation)));
  }
}

//------------------------------------------------------------------------------
void GPUStateManager::setBlendFunction(BLEND_FUNC src, BLEND_FUNC dest, 
                                       bool force)
{
  if (   force 
      || src != mInternalState.mBlendFuncSrc 
      || dest != mInternalState.mBlendFuncDst)
  {
    mInternalState.mBlendFuncSrc = src;
    mInternalState.mBlendFuncDst = dest;
    GL(glBlendFunc( BLEND_FUNCToGL(mInternalState.mBlendFuncSrc), 
                   BLEND_FUNCToGL(mInternalState.mBlendFuncDst) ));
  }
}

//------------------------------------------------------------------------------
void GPUStateManager::setColorMask(bool mask, bool force)
{
  if (force || mask != mInternalState.mColorMask)
  {
    mInternalState.mColorMask = mask;
    GLboolean b = mInternalState.mColorMask ? 1 : 0;
    GL(glColorMask(b,b,b,b));
  }
}

//------------------------------------------------------------------------------
void GPUStateManager::setCullState(STATE_CULL value, bool force)
{
  if (force || value != mInternalState.mCullState)
  {
    mInternalState.mCullState = value;
    GL(glCullFace((mInternalState.mCullState == CULL_FRONT) ? GL_FRONT : GL_BACK));
  }
}

//------------------------------------------------------------------------------
void GPUStateManager::setCullFaceEnable(bool value, bool force)
{
  if (force || value != mInternalState.mCullFaceEnable)
  {
    mInternalState.mCullFaceEnable = value;
    if (mInternalState.mCullFaceEnable)
    {
      GL(glEnable(GL_CULL_FACE));
    }
    else
    {
      GL(glDisable(GL_CULL_FACE));
    }
  }
}

//------------------------------------------------------------------------------
void GPUStateManager::setCullFrontFaceOrder(CULL_ORDER value, bool force)
{
  if (force || value != mInternalState.mCullOrder)
  {
    mInternalState.mCullOrder = value;
    GL(glFrontFace(CULL_ORDERToGL(mInternalState.mCullOrder)));
  }
}

//------------------------------------------------------------------------------
void GPUStateManager::setDepthFunc(DEPTH_FUNC value, bool force)
{
  if (force || value != mInternalState.mDepthFunc)
  {
    mInternalState.mDepthFunc = value;
    GL(glDepthFunc(DEPTH_FUNCToGL(mInternalState.mDepthFunc)));
  }
}

//------------------------------------------------------------------------------
void GPUStateManager::setDepthMask(bool value, bool force)
{
  if (force || value != mInternalState.mDepthMask)
  {
    mInternalState.mDepthMask = value;
    GL(glDepthMask(mInternalState.mDepthMask ? 1 : 0));
  }
}

//------------------------------------------------------------------------------
void GPUStateManager::setDepthTestEnable(bool value, bool force)
{
  if (force || value != mInternalState.mDepthTestEnable)
  {
    mInternalState.mDepthTestEnable = value;
    if (mInternalState.mDepthTestEnable)
    {
      GL(glEnable(GL_DEPTH_TEST));
    }
    else
    {
      GL(glDisable(GL_DEPTH_TEST));
    }
  }
}

//------------------------------------------------------------------------------
void GPUStateManager::setLineWidth(float width, bool force)
{
  if (force || width != mInternalState.mLineWidth)
  {
    // Supported in GLES?
    mInternalState.mLineWidth = width;
    GL(glLineWidth(width));
  }
}

//------------------------------------------------------------------------------
void GPUStateManager::setLineSmoothingEnable(bool value, bool force)
{
  if (force || value != mInternalState.mLineSmoothing)
  {
    mInternalState.mLineSmoothing = value;
    // Line smoothing not supported in OpenGL ES.
    // No warning is given because this is set by default at program
    // initialization to a known value.
#ifndef SPIRE_OPENGL_ES_2
    if (mInternalState.mLineSmoothing)
    {
      GL(glEnable(GL_LINE_SMOOTH));
    }
    else
    {
      GL(glDisable(GL_LINE_SMOOTH));
    }
#endif
  }
}
} // end of namespace Spire
