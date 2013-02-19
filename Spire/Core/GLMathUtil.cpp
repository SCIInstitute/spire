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
/// \date   January 2013
/// \brief  Common GL math utilities.

#include "Common.h"
#include "GLMathUtil.h"

namespace Spire {

void M33toArray9(const M33& in, float* out)
{
	// OpenGL is column major.

	// Column 1
	out[0 ] = in.m11;
	out[1 ] = in.m21;
	out[2 ] = in.m31;

	// Column 2
	out[3 ] = in.m12;
	out[4 ] = in.m22;
	out[5 ] = in.m32;

	// Column 3
	out[6 ] = in.m13;
	out[7 ] = in.m23;
	out[8 ] = in.m33;
}

void M44toArray16(const M44& in, float* out)
{
	// OpenGL is column major.

	// Column 1
	out[0 ] = in.m11;
	out[1 ] = in.m21;
	out[2 ] = in.m31;
	out[3 ] = in.m41;

	// Column 2
	out[4 ] = in.m12;
	out[5 ] = in.m22;
	out[6 ] = in.m32;
	out[7 ] = in.m42;

	// Column 3
	out[8 ] = in.m13;
	out[9 ] = in.m23;
	out[10] = in.m33;
	out[11] = in.m43;

	// Column 4
	out[12] = in.m14;
	out[13] = in.m24;
	out[14] = in.m34;
	out[15] = in.m44;
}

void V3toArray3(const V3& in, float* out)
{
	out[0] = in.x;
	out[1] = in.y;
	out[2] = in.z;
}

void V3toArray4(const V3& in, float* out)
{
	V3toArray3(in, out);
	out[3] = 1.0f;
}

void V4toArray3(const V4& in, float* out)
{
	out[0] = in.x;
	out[1] = in.y;
	out[2] = in.z;
}

void V4toArray4(const V4& in, float* out)
{
	V4toArray3(in, out);
	out[3] = in.w;
}

} // end of namespace Spire

