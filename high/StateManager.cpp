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
GLenum BLEND_FUNCToGL(const BLEND_FUNC& func)
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
BLEND_FUNC GLToBLEND_FUNC(const GLenum& func) 
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
GLenum BLEND_EQToGL(const BLEND_EQUATION& func) 
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
int StateManager::getMaxTextureUnits()
{
  GLint tmp;
  glGetIntegerv(GL_MAX_TEXTURE_UNITS, &tmp);
  return tmp;
}

//------------------------------------------------------------------------------
void StateManager::apply(const GPUState& state, bool force)
{
  GL_CHECK();

  setEnableDepthTest(state.enableDepthTest, bForce);
  setDepthFunc(state.depthFunc, bForce);
  setEnableCullFace(state.enableCullFace, bForce);
  setCullState(state.cullState, bForce);
  setEnableBlend(state.enableBlend, bForce);
  setEnableScissor(state.enableScissor, bForce);
  setEnableLighting(state.enableLighting, bForce);
  setEnableColorMaterial(state.enableColorMaterial, bForce);

  // Do this by hand to avoid the redundant glActiveTexture calls
  /// \todo Grab the maximum number of texture units and use that instead...
  for (size_t i = 0; i < getMaxTextureUnits();  i++) 
  {
    if (bForce || state.enableTex[i] != m_InternalState.enableTex[i]) 
    {
      glActiveTexture(GLenum(GL_TEXTURE0+i));
      m_InternalState.enableTex[i] = state.enableTex[i];
      switch (m_InternalState.enableTex[i]) {
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
  mInternalState.activeTexUnit = state.activeTexUnit;
  glActiveTexture(GLenum(GL_TEXTURE0 + m_InternalState.activeTexUnit));

  setDepthMask(state.depthMask, bForce);
  setColorMask(state.colorMask, bForce);
  setBlendEquation(state.blendEquation, bForce);
  setBlendFunction(state.blendFuncSrc, state.blendFuncDst, bForce);
  setLineWidth(state.lineWidth, bForce);

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
  state.depthMask = b != 0;

  GLboolean	 col[4];
  glGetBooleanv(GL_COLOR_WRITEMASK, col);
  state.colorMask = col[0] != 0;  

  GLint src, dest;
  glGetIntegerv(GL_BLEND_SRC, &src);
  glGetIntegerv(GL_BLEND_DST, &dest);
  state.blendFuncSrc = GLToBLEND_FUNC(src);
  state.blendFuncDst = GLToBLEND_FUNC(dest);

  GLint equation;
  glGetIntegerv(GL_BLEND_EQUATION_RGB, &equation); 
  state.mBlendEquation = GLToBLEND_EQ(equation);

  GL_CHECK();
}

//------------------------------------------------------------------------------
void StateManager::setBlendEnable(bool value, bool force)
{
}

//------------------------------------------------------------------------------
void StateManager::setBlendEquation(BLEND_EQ value, bool force)
{
}

//------------------------------------------------------------------------------
void StateManager::setBlendFunction(BLEND_FUNC src, BLEND_FUNC dest, force)
{
}

//------------------------------------------------------------------------------
void StateManager::setColorMask(bool mask, bool force)
{
}

//------------------------------------------------------------------------------
void StateManager::setCullFaceEnable(bool value, bool force)
{
}

//------------------------------------------------------------------------------
void StateManager::setDepthFunc(DEPTH_FUNC value, bool force)
{
}

//------------------------------------------------------------------------------
void StateManager::setDepthMask(bool value, bool force)
{
}

//------------------------------------------------------------------------------
void StateManager::setDepthTestEnable(bool value, bool force)
{
}


} // end of namespace Spire
