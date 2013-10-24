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

#ifndef SPIRE_HIGH_GPUSTATEMANAGER_H
#define SPIRE_HIGH_GPUSTATEMANAGER_H

#include <cstddef>

namespace CPM_SPIRE_NS {
namespace spire {

#define MAX_TEXTURE_UNITS 12
#define MIN_TEXTURE_UNITS 4

/// Texture state
enum STATE_TEX
{
  TEX_1D,
  TEX_2D,
  TEX_3D,
  TEX_NONE
};

/// Front or back face culling.
enum STATE_CULL 
{
  CULL_FRONT,
  CULL_BACK
};

enum CULL_ORDER
{
  ORDER_CCW,
  ORDER_CW
};

enum BLEND_FUNC 
{
  BF_ZERO,
  BF_ONE,
  BF_SRC_COLOR,
  BF_ONE_MINUS_SRC_COLOR,
  BF_DST_COLOR,
  BF_ONE_MINUS_DST_COLOR,
  BF_SRC_ALPHA,
  BF_ONE_MINUS_SRC_ALPHA,
  BF_DST_ALPHA,
  BF_ONE_MINUS_DST_ALPHA,
  BF_SRC_ALPHA_SATURATE
};

enum BLEND_EQ {
  BE_FUNC_ADD,
  BE_FUNC_SUBTRACT,
  BE_FUNC_REVERSE_SUBTRACT,
  BE_MIN,
  BE_MAX
};

/// Settings for the depth function.
enum DEPTH_FUNC {
  DF_NEVER,
  DF_LESS,
  DF_EQUAL,
  DF_LEQUAL,
  DF_GREATER,
  DF_NOTEQUAL,
  DF_GEQUAL,
  DF_ALWAYS
};

/// Represents a possible snapshot of the GPUs state.
/// In order for changes to be reflected on the GPU, state must be modified
/// through the StateManager. This class only represents a snapshot of some GPU 
/// state. It knows nothing about the current state of the GPU.
/// \todo re-add final.
class GPUState //final
{
public:

  GPUState() :
      mDepthTestEnable(true),
      mDepthFunc(DF_LESS),
      mCullFaceEnable(false),
      mCullState(CULL_BACK),
      mCullOrder(ORDER_CCW),
      mBlendEnable(true),
      mBlendEquation(BE_FUNC_ADD),
      mBlendFuncSrc(BF_SRC_ALPHA),
      mBlendFuncDst(BF_ONE_MINUS_SRC_ALPHA),
      mDepthMask(true),
      mColorMask(true),
      mLineWidth(2.0f),
      mLineSmoothing(false),
      mTexActiveUnit(0)
  {
    for (size_t i = 0; i < MAX_TEXTURE_UNITS ; i++) mTexEnable[i] = TEX_NONE;
  }
  ~GPUState() {}

  // Direct access to member variables to avoid a false sense of 'setting'
  // a GPU state. The only place where GPU state is modified is via the
  // state manager.

  bool        mDepthTestEnable;
  DEPTH_FUNC  mDepthFunc;

  bool        mCullFaceEnable;
  STATE_CULL  mCullState;
  CULL_ORDER  mCullOrder;

  bool        mBlendEnable;
  BLEND_EQ    mBlendEquation;
  BLEND_FUNC  mBlendFuncSrc;
  BLEND_FUNC  mBlendFuncDst;

  bool        mDepthMask;
  bool        mColorMask;

  float       mLineWidth;       ///< glLineWidth(...)
  bool        mLineSmoothing;   ///< GL_LINE_SMOOTH - Anti-aliasing for lines

  size_t      mTexActiveUnit;
  STATE_TEX   mTexEnable[MAX_TEXTURE_UNITS];
};

/// Manages OpenGL state. Provides something of an abstraction layer on top
/// of OpenGL. This class can easily be made a pure virtual base class upon 
/// which DX and OpenGL could sit.
class GPUStateManager
{
public:
  GPUStateManager();
  virtual ~GPUStateManager();
  
  /// Applies 'state'.
  /// Unless force == true, takes into consideration the currently known GPU 
  /// state and only toggles state where necessary.
  /// If force == true, then GPU state is modified regardless of the current 
  /// state indicated by mInternalState.
  void apply(const GPUState& state, bool force = false);

  /// Retrieves the current GPU state.
  const GPUState& getState() const                {return mInternalState;}

  /// Returns a GPUState structure based on what the state is currently enabled
  /// in OpenGL.
  GPUState getStateFromOpenGL() const;

  /// Misc functions for applying state
  /// @{
  void setDepthTestEnable(bool value, bool force = false);
  void setDepthFunc(DEPTH_FUNC value, bool force = false);
  void setCullState(STATE_CULL value, bool force = false);
  void setCullFaceEnable(bool value, bool force = false);
  void setCullFrontFaceOrder(CULL_ORDER val, bool force = false);
  void setBlendEnable(bool value, bool force = false);
  void setBlendEquation(BLEND_EQ value, bool force = false);
  void setBlendFunction(BLEND_FUNC src, BLEND_FUNC dest, bool force = false);
  void setDepthMask(bool value, bool force = false);
  void setColorMask(bool mask, bool force = false);
  void setLineWidth(float width, bool force = false);
  void setLineSmoothingEnable(bool value, bool force = false);
  /// @}

  /// Returns the maximum number of supported texture units.
  size_t getMaxTextureUnits() const;

protected:
  GPUState mInternalState;
};

} // namespace spire
} // namespace CPM_SPIRE_NS

#endif 
