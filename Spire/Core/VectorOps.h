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

#ifndef SPIRE_CORE_VECTOROPS_H
#define SPIRE_CORE_VECTOROPS_H

#include "Vectors.h"

namespace Spire {
namespace VecOps {

//------------------------------------------------------------------------------
// Vector4
//------------------------------------------------------------------------------

template <class T>
inline const Vector4<T> mulPerElem( const Vector4<T> & vec0, const Vector4<T> & vec1 )
{
    return Vector4<T>(
        ( vec0.x * vec1.x ),
        ( vec0.y * vec1.y ),
        ( vec0.z * vec1.z ),
        ( vec0.w * vec1.w )
    );
}

template <class T>
inline const Vector4<T> divPerElem( const Vector4<T> & vec0, const Vector4<T> & vec1 )
{
    return Vector4<T>(
        ( vec0.x / vec1.x ),
        ( vec0.y / vec1.y ),
        ( vec0.z / vec1.z ),
        ( vec0.w / vec1.w )
    );
}

template <class T>
inline const Vector4<T> recipPerElem( const Vector4<T> & vec )
{
    return Vector4<T>(
        ( 1.0f / vec.x ),
        ( 1.0f / vec.y ),
        ( 1.0f / vec.z ),
        ( 1.0f / vec.w )
    );
}

template <class T>
inline const Vector4<T> sqrtPerElem( const Vector4<T> & vec )
{
    return Vector4<T>(
        sqrtf( vec.x ),
        sqrtf( vec.y ),
        sqrtf( vec.z ),
        sqrtf( vec.w )
    );
}

template <class T>
inline const Vector4<T> rsqrtPerElem( const Vector4<T> & vec )
{
    return Vector4<T>(
        ( 1.0f / sqrtf( vec.x ) ),
        ( 1.0f / sqrtf( vec.y ) ),
        ( 1.0f / sqrtf( vec.z ) ),
        ( 1.0f / sqrtf( vec.w ) )
    );
}

template <class T>
inline const Vector4<T> absPerElem( const Vector4<T> & vec )
{
    return Vector4<T>(
        fabsf( vec.x ),
        fabsf( vec.y ),
        fabsf( vec.z ),
        fabsf( vec.w )
    );
}

template <class T>
inline const Vector4<T> copySignPerElem( const Vector4<T> & vec0, const Vector4<T> & vec1 )
{
    return Vector4<T>(
        ( vec1.x < 0.0f )? -fabsf( vec0.x ) : fabsf( vec0.x ),
        ( vec1.y < 0.0f )? -fabsf( vec0.y ) : fabsf( vec0.y ),
        ( vec1.z < 0.0f )? -fabsf( vec0.z ) : fabsf( vec0.z ),
        ( vec1.w < 0.0f )? -fabsf( vec0.w ) : fabsf( vec0.w )
    );
}

template <class T>
inline const Vector4<T> maxPerElem( const Vector4<T> & vec0, const Vector4<T> & vec1 )
{
    return Vector4<T>(
        (vec0.x > vec1.x)? vec0.x : vec1.x,
        (vec0.y > vec1.y)? vec0.y : vec1.y,
        (vec0.z > vec1.z)? vec0.z : vec1.z,
        (vec0.w > vec1.w)? vec0.w : vec1.w
    );
}

template <class T>
inline float maxElem( const Vector4<T> & vec )
{
    float result;
    result = (vec.x > vec.y)? vec.x : vec.y;
    result = (vec.z > result)? vec.z : result;
    result = (vec.w > result)? vec.w : result;
    return result;
}

template <class T>
inline const Vector4<T> minPerElem( const Vector4<T> & vec0, const Vector4<T> & vec1 )
{
    return Vector4<T>(
        (vec0.x < vec1.x)? vec0.x : vec1.x,
        (vec0.y < vec1.y)? vec0.y : vec1.y,
        (vec0.z < vec1.z)? vec0.z : vec1.z,
        (vec0.w < vec1.w)? vec0.w : vec1.w
    );
}

template <class T>
inline float minElem( const Vector4<T> & vec )
{
    float result;
    result = (vec.x < vec.y)? vec.x : vec.y;
    result = (vec.z < result)? vec.z : result;
    result = (vec.w < result)? vec.w : result;
    return result;
}

template <class T>
inline float sum( const Vector4<T> & vec )
{
    float result;
    result = ( vec.x + vec.y );
    result = ( result + vec.z );
    result = ( result + vec.w );
    return result;
}

template <class T>
inline float dot( const Vector4<T> & vec0, const Vector4<T> & vec1 )
{
    float result;
    result = ( vec0.x * vec1.x );
    result = ( result + ( vec0.y * vec1.y ) );
    result = ( result + ( vec0.z * vec1.z ) );
    result = ( result + ( vec0.w * vec1.w ) );
    return result;
}

template <class T>
inline float lengthSqr( const Vector4<T> & vec )
{
    float result;
    result = ( vec.x * vec.x );
    result = ( result + ( vec.y * vec.y ) );
    result = ( result + ( vec.z * vec.z ) );
    result = ( result + ( vec.w * vec.w ) );
    return result;
}

template <class T>
inline float length( const Vector4<T> & vec )
{
    return ::sqrtf( lengthSqr( vec ) );
}

template <class T>
inline const Vector4<T> normalize( const Vector4<T> & vec )
{
    float lenSqr, lenInv;
    lenSqr = lengthSqr( vec );
    lenInv = ( 1.0f / sqrtf( lenSqr ) );
    return Vector4<T>(
        ( vec.x * lenInv ),
        ( vec.y * lenInv ),
        ( vec.z * lenInv ),
        ( vec.w * lenInv )
    );
}

template <class T>
inline const Vector4<T> select( const Vector4<T> & vec0, const Vector4<T> & vec1, bool select1 )
{
    return Vector4<T>(
        ( select1 )? vec1.x : vec0.x,
        ( select1 )? vec1.y : vec0.y,
        ( select1 )? vec1.z : vec0.z,
        ( select1 )? vec1.w : vec0.w
    );
}


//------------------------------------------------------------------------------
// Vector3
//------------------------------------------------------------------------------

template <class T>
inline const Vector3<T> mulPerElem( const Vector3<T> & vec0, const Vector3<T> & vec1 )
{
    return Vector3<T>(
        ( vec0.x * vec1.x ),
        ( vec0.y * vec1.y ),
        ( vec0.z * vec1.z )
    );
}

template <class T>
inline const Vector3<T> divPerElem( const Vector3<T> & vec0, const Vector3<T> & vec1 )
{
    return Vector3<T>(
        ( vec0.x / vec1.x ),
        ( vec0.y / vec1.y ),
        ( vec0.z / vec1.z )
    );
}

template <class T>
inline const Vector3<T> recipPerElem( const Vector3<T> & vec )
{
    return Vector3<T>(
        ( 1.0f / vec.x ),
        ( 1.0f / vec.y ),
        ( 1.0f / vec.z )
    );
}

template <class T>
inline const Vector3<T> sqrtPerElem( const Vector3<T> & vec )
{
    return Vector3<T>(
        sqrtf( vec.x ),
        sqrtf( vec.y ),
        sqrtf( vec.z )
    );
}

template <class T>
inline const Vector3<T> rsqrtPerElem( const Vector3<T> & vec )
{
    return Vector3<T>(
        ( 1.0f / sqrtf( vec.x ) ),
        ( 1.0f / sqrtf( vec.y ) ),
        ( 1.0f / sqrtf( vec.z ) )
    );
}

template <class T>
inline const Vector3<T> absPerElem( const Vector3<T> & vec )
{
    return Vector3<T>(
        fabsf( vec.x ),
        fabsf( vec.y ),
        fabsf( vec.z )
    );
}

template <class T>
inline const Vector3<T> copySignPerElem( const Vector3<T> & vec0, const Vector3<T> & vec1 )
{
    return Vector3<T>(
        ( vec1.x < 0.0f )? -fabsf( vec0.x ) : fabsf( vec0.x ),
        ( vec1.y < 0.0f )? -fabsf( vec0.y ) : fabsf( vec0.y ),
        ( vec1.z < 0.0f )? -fabsf( vec0.z ) : fabsf( vec0.z )
    );
}

template <class T>
inline const Vector3<T> maxPerElem( const Vector3<T> & vec0, const Vector3<T> & vec1 )
{
    return Vector3<T>(
        (vec0.x > vec1.x)? vec0.x : vec1.x,
        (vec0.y > vec1.y)? vec0.y : vec1.y,
        (vec0.z > vec1.z)? vec0.z : vec1.z
    );
}

template <class T>
inline float maxElem( const Vector3<T> & vec )
{
    float result;
    result = (vec.x > vec.y)? vec.x : vec.y;
    result = (vec.z > result)? vec.z : result;
    return result;
}

template <class T>
inline const Vector3<T> minPerElem( const Vector3<T> & vec0, const Vector3<T> & vec1 )
{
    return Vector3<T>(
        (vec0.x < vec1.x)? vec0.x : vec1.x,
        (vec0.y < vec1.y)? vec0.y : vec1.y,
        (vec0.z < vec1.z)? vec0.z : vec1.z
    );
}

template <class T>
inline float minElem( const Vector3<T> & vec )
{
    float result;
    result = (vec.x < vec.y)? vec.x : vec.y;
    result = (vec.z < result)? vec.z : result;
    return result;
}

template <class T>
inline float sum( const Vector3<T> & vec )
{
    float result;
    result = ( vec.x + vec.y );
    result = ( result + vec.z );
    return result;
}

template <class T>
inline float dot( const Vector3<T> & vec0, const Vector3<T> & vec1 )
{
    float result;
    result = ( vec0.x * vec1.x );
    result = ( result + ( vec0.y * vec1.y ) );
    result = ( result + ( vec0.z * vec1.z ) );
    return result;
}

template <class T>
inline float lengthSqr( const Vector3<T> & vec )
{
    float result;
    result = ( vec.x * vec.x );
    result = ( result + ( vec.y * vec.y ) );
    result = ( result + ( vec.z * vec.z ) );
    return result;
}

template <class T>
inline float length( const Vector3<T> & vec )
{
    return ::sqrtf( lengthSqr( vec ) );
}

template <class T>
inline const Vector3<T> normalize( const Vector3<T> & vec )
{
    float lenSqr, lenInv;
    lenSqr = lengthSqr( vec );
    lenInv = ( 1.0f / sqrtf( lenSqr ) );
    return Vector3<T>(
        ( vec.x * lenInv ),
        ( vec.y * lenInv ),
        ( vec.z * lenInv )
    );
}

template <class T>
inline const Vector3<T> cross( const Vector3<T> & vec0, const Vector3<T> & vec1 )
{
    return Vector3<T>(
        ( ( vec0.y * vec1.z ) - ( vec0.z * vec1.y ) ),
        ( ( vec0.z * vec1.x ) - ( vec0.x * vec1.z ) ),
        ( ( vec0.x * vec1.y ) - ( vec0.y * vec1.x ) )
    );
}

template <class T>
inline const Vector3<T> select( const Vector3<T> & vec0, const Vector3<T> & vec1, bool select1 )
{
    return Vector3<T>(
        ( select1 )? vec1.x : vec0.x,
        ( select1 )? vec1.y : vec0.y,
        ( select1 )? vec1.z : vec0.z
    );
}

} // namespace VecOps
} // namespace Spire

#endif
