/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2008 Scientific Computing and Imaging Institute,
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

/**
  \file    ArcBall.cpp
  \author  Jens Krueger
           SCI Institute
           University of Utah
           Based on the NeHe Tutorial 48
  \date    October 2008
*/

#include "ArcBall.h"

float ArcBall::ms_fEpsilon = 1.0e-5f;

//------------------------------------------------------------------------------
ArcBall::ArcBall(int32_t iWinWidth, int32_t iWinHeight,
                 int32_t iWinOffsetX, int32_t iWinOffsetY,
                 bool bUseTranslation) :
    m_vStartDrag(),
    m_iWinDim(iWinWidth, iWinHeight),
    m_iWinOffsets(iWinOffsetX, iWinOffsetY),
    m_fRadius(1.0f),
    m_bUseTranslation(bUseTranslation)
{
}

//------------------------------------------------------------------------------
void ArcBall::SetWindowSize(int32_t iWinWidth, int32_t iWinHeight)
{
    m_iWinDim = Vector2<int>(iWinWidth, iWinHeight);
}

//------------------------------------------------------------------------------
void ArcBall::SetWindowOffset(int32_t iWinOffsetX, int32_t iWinOffsetY)
{
  m_iWinOffsets = Vector2<int32_t>(iWinOffsetX, iWinOffsetY);
}

//------------------------------------------------------------------------------
void ArcBall::Click(Vector2<int32_t> vPosition)
{
  m_vStartDrag = MapToSphere(vPosition);
}

//------------------------------------------------------------------------------
Quat ArcBall::Drag(Vector2<int32_t> vPosition)
{
  Quat qRotation;

  // Map the point to the sphere
  V3 vCurrent = MapToSphere(vPosition);

  // Compute the vector perpendicular to the begin and end vectors
  V3    vCross(vCurrent % m_vStartDrag);
  float fDot(vCurrent ^ m_vStartDrag);

  if (vCross.length() > ms_fEpsilon)    //if its non-zero
      return Quat(vCross.x, vCross.y, vCross.z, fDot);
  else
      return Quat(0,0,0,0);
}

//------------------------------------------------------------------------------
V3 ArcBall::MapToSphere(Vector2<int32_t> vPosition) const
{
  V3 vResult;

  // normalize position to [-1 ... 1]
  Vector2<int32_t> vNormPosition;
  vNormPosition.x =  -(((vPosition.x-m_iWinOffsets.x) / (float(m_iWinDim.x - 1) / 2.0f)) - 1.0f);
  vNormPosition.y =  ((vPosition.y-m_iWinOffsets.y) / (float(m_iWinDim.y - 1) / 2.0f)) - 1.0f;

  if (m_bUseTranslation)
  {
    M44 mTranslation(m_mTranslation);
    mTranslation.m43 = 0;
    vNormPosition = (V4(vNormPosition,0.0f,1.0f) * mTranslation).xy();
  }

  // Compute the length of the vector to the point from the center
  float length = vNormPosition.length();

  // If the point is mapped outside of the sphere... (length > radius)
  if (length > m_fRadius)
  {
      // Compute a normalizing factor (radius / length)
      float norm = float(m_fRadius / length);

      // Return the "normalized" vector, a point on the sphere
      vResult.x = vNormPosition.x * norm;
      vResult.y = vNormPosition.y * norm;
      vResult.z = 0.0f;
  }
  else    // Else it's on the inside
  {
      // Return a vector to a point mapped inside the sphere
      vResult.x = vNormPosition.x;
      vResult.y = vNormPosition.y;
      vResult.z = length-m_fRadius;
  }

  return vResult;
}

