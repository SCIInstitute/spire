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
#include "StateManager.h"

namespace Spire {

//------------------------------------------------------------------------------
GLenum BLEND_FUNCToGL(BLEND_FUNC func)
{
  switch (func) 
  {
    case BF_ZERO:                   return GL_ZERO;                   break;
    case BF_ONE:                    return GL_ONE;                    break;
    case BF_SRC_COLOR:              return GL_SRC_COLOR;              break;
    case BF_ONE_MINUS_SRC_COLOR:    return GL_ONE_MINUS_SRC_COLOR;    break;
    case BF_DST_COLOR:              return GL_DST_COLOR;              break;
    case BF_ONE_MINUS_DST_COLOR:    return GL_ONE_MINUS_DST_COLOR;    break;
    case BF_SRC_ALPHA:              return GL_SRC_ALPHA;              break;
    case BF_ONE_MINUS_SRC_ALPHA:    return GL_ONE_MINUS_SRC_ALPHA;    break;
    case BF_DST_ALPHA:              return GL_DST_ALPHA;              break;
    case BF_ONE_MINUS_DST_ALPHA:    return GL_ONE_MINUS_DST_ALPHA;    break;
    case BF_SRC_ALPHA_SATURATE:     return GL_SRC_ALPHA_SATURATE;     break;
  }
  return GL_ONE;
}

//------------------------------------------------------------------------------
BLEND_FUNC GLToBLEND_FUNC(GLenum func) 
{
  switch (func) 
  {
    case GL_ZERO:                   return BF_ZERO;                   break;
    case GL_ONE:                    return BF_ONE;                    break;
    case GL_SRC_COLOR:              return BF_SRC_COLOR;              break;
    case GL_ONE_MINUS_SRC_COLOR:    return BF_ONE_MINUS_SRC_COLOR;    break;
    case GL_DST_COLOR:              return BF_DST_COLOR;              break;
    case GL_ONE_MINUS_DST_COLOR:    return BF_ONE_MINUS_DST_COLOR;    break;
    case GL_SRC_ALPHA:              return BF_SRC_ALPHA;              break;
    case GL_ONE_MINUS_SRC_ALPHA:    return BF_ONE_MINUS_SRC_ALPHA;    break;
    case GL_DST_ALPHA:              return BF_DST_ALPHA;              break;
    case GL_ONE_MINUS_DST_ALPHA:    return BF_ONE_MINUS_DST_ALPHA;    break;
    case GL_SRC_ALPHA_SATURATE:     return BF_SRC_ALPHA_SATURATE;     break;
  }
  return BF_ONE;
}

//------------------------------------------------------------------------------
GLenum BLEND_EQToGL(BLEND_EQ func) 
{
  switch (func) 
  {
    case BE_FUNC_ADD:               return GL_FUNC_ADD;               break;
    case BE_FUNC_SUBTRACT:          return GL_FUNC_SUBTRACT;          break;
    case BE_FUNC_REVERSE_SUBTRACT:  return GL_FUNC_REVERSE_SUBTRACT;  break;
    case BE_MIN:                    return GL_MIN;                    break;
    case BE_MAX:                    return GL_MAX;                    break;
  }
  return GL_FUNC_ADD;
}

//------------------------------------------------------------------------------
BLEND_EQ GLToBLEND_EQ(const GLenum& func) 
{
  switch (func) 
  {
    case GL_FUNC_ADD:               return BE_FUNC_ADD;               break;
    case GL_FUNC_SUBTRACT:          return BE_FUNC_SUBTRACT;          break;
    case GL_FUNC_REVERSE_SUBTRACT:  return BE_FUNC_REVERSE_SUBTRACT;  break;
    case GL_MIN:                    return BE_MIN;                    break;
    case GL_MAX:                    return BE_MAX;                    break;
  }
  return BE_FUNC_ADD;
}

//------------------------------------------------------------------------------
GLenum DEPTH_FUNCToGL(const DEPTH_FUNC& func) 
{
  switch (func) 
  {
    case DF_NEVER:                  return GL_NEVER;                  break;
    case DF_LESS:                   return GL_LESS;                   break;
    case DF_EQUAL:                  return GL_EQUAL;                  break;
    case DF_LEQUAL:                 return GL_LEQUAL;                 break;
    case DF_GREATER:                return GL_GREATER;                break;
    case DF_NOTEQUAL:               return GL_NOTEQUAL;               break;
    case DF_GEQUAL:                 return GL_GEQUAL;                 break;
    case DF_ALWAYS:                 return GL_ALWAYS;                 break;
  }
  return GL_LEQUAL;
}

//------------------------------------------------------------------------------
DEPTH_FUNC GLToDEPTH_FUNC(const GLenum& func) 
{
  switch (func) 
  {
    case GL_NEVER:                  return DF_NEVER;                  break;
    case GL_LESS:                   return DF_LESS;                   break;
    case GL_EQUAL:                  return DF_EQUAL;                  break;
    case GL_LEQUAL:                 return DF_LEQUAL;                 break;
    case GL_GREATER:                return DF_GREATER;                break;
    case GL_NOTEQUAL:               return DF_NOTEQUAL;               break;
    case GL_GEQUAL:                 return DF_GEQUAL;                 break;
    case GL_ALWAYS:                 return DF_ALWAYS;                 break;
  }
  return DF_LEQUAL;
}


//------------------------------------------------------------------------------
StateManager::StateManager()
{
}

//------------------------------------------------------------------------------
StateManager::~StateManager()
{
}

//------------------------------------------------------------------------------
int StateManager::getMaxTextureUnits() const
{
  GLint tmp;
  glGetIntegerv(GL_MAX_TEXTURE_UNITS, &tmp);
  return tmp;
}

//------------------------------------------------------------------------------
void StateManager::apply(const GPUState& state, bool force)
{
  GL_CHECK();

  setDepthTestEnable(state.mDepthTestEnable, force);
  setDepthFunc(state.mDepthFunc, force);
  setCullFaceEnable(state.mCullFaceEnable, force);
  setCullState(state.mCullState, force);
  setBlendEnable(state.mBlendEnable, force);

  // Do this by hand to avoid the redundant glActiveTexture calls
  /// \todo Grab the maximum number of texture units and use that instead...
  for (size_t i = 0; i < getMaxTextureUnits();  i++) 
  {
    if (force || state.mTexEnable[i] != mInternalState.mTexEnable[i]) 
    {
      glActiveTexture(GLenum(GL_TEXTURE0+i));
      mInternalState.mTexEnable[i] = state.mTexEnable[i];
      switch (mInternalState.mTexEnable[i]) {
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
                          glEnable(GL_TEXTURE_3D);
                          break;
        case TEX_NONE:    glDisable(GL_TEXTURE_1D);
                          glDisable(GL_TEXTURE_2D);
                          glDisable(GL_TEXTURE_3D);
                          glDisable(GL_TEXTURE_CUBE_MAP);
                          break;
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
GPUState StateManager::getStateFromOpenGL() const
{
  GL_CHECK();

  GPUState state;

  GLint e;
  glGetIntegerv(GL_DEPTH_FUNC, &e);
  state.mDepthTestEnable              = glIsEnabled(GL_DEPTH_TEST) != 0;
  state.mDepthFunc                    = GLToDEPTH_FUNC(e);

  state.mCullFaceEnable               = glIsEnabled(GL_CULL_FACE) != 0;

  glGetIntegerv(GL_CULL_FACE_MODE, &e);
  state.mCullState                    = (e == GL_FRONT) ? CULL_FRONT : CULL_BACK;

  state.mBlendEnable                  = glIsEnabled(GL_BLEND) != 0;

  for(size_t i=0; i < getMaxTextureUnits(); ++i)
  {
    glActiveTexture(GL_TEXTURE0 + GLenum(i));
    if(glIsEnabled(GL_TEXTURE_3D)) 
    {
      state.mTexEnable[i] = TEX_3D;
    }
    else if(glIsEnabled(GL_TEXTURE_2D))
    {
      state.mTexEnable[i] = TEX_2D;
    }
    else if(glIsEnabled(GL_TEXTURE_1D)) 
    {
      state.mTexEnable[i] = TEX_1D;
    }
    else 
    {
      state.mTexEnable[i] = TEX_NONE;
    }
  }
  GLboolean	 b;
  glGetBooleanv(GL_DEPTH_WRITEMASK, &b);
  state.mDepthMask = b != 0;

  GLboolean	 col[4];
  glGetBooleanv(GL_COLOR_WRITEMASK, col);
  state.mColorMask = col[0] != 0;  

  GLint src, dest;
  glGetIntegerv(GL_BLEND_SRC, &src);
  glGetIntegerv(GL_BLEND_DST, &dest);
  state.mBlendFuncSrc = GLToBLEND_FUNC(src);
  state.mBlendFuncDst = GLToBLEND_FUNC(dest);

  GLint equation;
  glGetIntegerv(GL_BLEND_EQUATION_RGB, &equation); 
  state.mBlendEquation = GLToBLEND_EQ(equation);

  GL_CHECK();
  
  return state;
}

//------------------------------------------------------------------------------
void StateManager::setBlendEnable(bool value, bool force)
{
  if (force || value != mInternalState.mBlendEnable)
  {
    mInternalState.mBlendEnable = value;
    if (mInternalState.mBlendEnable)
    {
      glEnable(GL_BLEND);
    }
    else
    {
      glDisable(GL_BLEND);
    }
  }
}

//------------------------------------------------------------------------------
void StateManager::setBlendEquation(BLEND_EQ value, bool force)
{
  if (force || value != mInternalState.mBlendEquation)
  {
    mInternalState.mBlendEquation = value;
    glBlendEquation(BLEND_EQToGL(mInternalState.mBlendEquation));
  }
}

//------------------------------------------------------------------------------
void StateManager::setBlendFunction(BLEND_FUNC src, BLEND_FUNC dest, bool force)
{
  if (   force 
      || src != mInternalState.mBlendFuncSrc 
      || dest != mInternalState.mBlendFuncDst)
  {
    mInternalState.mBlendFuncSrc = src;
    mInternalState.mBlendFuncDst = dest;
    glBlendFunc( BLEND_FUNCToGL(mInternalState.mBlendFuncSrc), 
                 BLEND_FUNCToGL(mInternalState.mBlendFuncDst) );
  }
}

//------------------------------------------------------------------------------
void StateManager::setColorMask(bool mask, bool force)
{
  if (force || mask != mInternalState.mColorMask)
  {
    mInternalState.mColorMask = mask;
    GLboolean b = mInternalState.mColorMask ? 1 : 0;
    glColorMask(b,b,b,b);
  }
}

//------------------------------------------------------------------------------
void StateManager::setCullState(STATE_CULL value, bool force)
{
  if (force || value != mInternalState.mCullState)
  {
    mInternalState.mCullState = value;
    glCullFace((mInternalState.mCullState == CULL_FRONT) ? GL_FRONT : GL_BACK);
  }
}

//------------------------------------------------------------------------------
void StateManager::setCullFaceEnable(bool value, bool force)
{
  if (force || value != mInternalState.mCullFaceEnable)
  {
    mInternalState.mCullFaceEnable = value;
    if (mInternalState.mCullFaceEnable )
    {
      glEnable(GL_CULL_FACE);
    }
    else
    {
      glDisable(GL_CULL_FACE);
    }
  }
}

//------------------------------------------------------------------------------
void StateManager::setDepthFunc(DEPTH_FUNC value, bool force)
{
  if (force || value != mInternalState.mDepthFunc)
  {
    mInternalState.mDepthFunc = value;
    glDepthFunc( DEPTH_FUNCToGL(mInternalState.mDepthFunc ) );
  }
}

//------------------------------------------------------------------------------
void StateManager::setDepthMask(bool value, bool force)
{
  if (force || value != mInternalState.mDepthMask)
  {
    mInternalState.mDepthMask = value;
    glDepthMask(mInternalState.mDepthMask ? 1 : 0);
  }
}

//------------------------------------------------------------------------------
void StateManager::setDepthTestEnable(bool value, bool force)
{
  if (force || value != mInternalState.mDepthTestEnable)
  {
    mInternalState.mDepthTestEnable = value;
    if (mInternalState.mDepthTestEnable )
    {
      glEnable(GL_DEPTH_TEST);
    }
    else
    {
      glDisable(GL_DEPTH_TEST);
    }
  }
}


} // end of namespace Spire
