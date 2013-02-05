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

#ifndef SPIRE_CORE_VECTORS_H
#define SPIRE_CORE_VECTORS_H

#include <cassert>
#include <cmath>
#include <istream>
#include <iomanip>
#include <limits>
#include <vector>
#include <ostream>

namespace Spire {

template <class T> class Matrix2;
template <class T> class Matrix3;
template <class T> class Matrix4;

template <class T=int> class Vector2 {
public:
  T x,y;

  Vector2<T>(): x(0), y(0) {}
  template <class S> explicit Vector2<T>( const std::vector<S>& v, const T& defaultVal = T(0) ) {
    x = T(v.size()>0 ? v[0] : defaultVal);
    y = T(v.size()>1 ? v[1] : defaultVal);
  }
  Vector2<T>(const Vector2<T> &other): x(other.x), y(other.y) {}
  template <class S> explicit Vector2<T>(const Vector2<S> &other): x(T(other.x)), y(T(other.y)) {}
  Vector2<T>(const T _x, const T _y) : x(_x), y(_y) {}
  Vector2<T>(const T* vec) : x(vec[0]), y(vec[1]) {}

  bool operator == ( const Vector2<T>& other ) const {return (other.x==x && other.y==y); }
  bool operator != ( const Vector2<T>& other ) const {return (other.x!=x || other.y!=y); }

    // binary operators with scalars
  Vector2<T> operator + ( T scalar ) const {return Vector2<T>(x+scalar,y+scalar);}
  Vector2<T> operator - ( T scalar ) const {return Vector2<T>(x-scalar,y-scalar);}
  Vector2<T> operator * ( T scalar ) const {return Vector2<T>(x*scalar,y*scalar);}
  Vector2<T> operator / ( T scalar ) const {return Vector2<T>(x/scalar,y/scalar);}
  Vector2<T> operator % ( T scalar ) const {return Vector2<T>(x%scalar,y%scalar);}

  // binary operators with vectors
  Vector2<T> operator + ( const Vector2& other ) const {return Vector2<T>(x+other.x,y+other.y);}
  Vector2<T> operator - ( const Vector2& other ) const {return Vector2<T>(x-other.x,y-other.y);}
  Vector2<T> operator / ( const Vector2& other ) const {return Vector2<T>(x/other.x,y/other.y);}
  Vector2<T> operator * ( const Vector2& other ) const {return Vector2<T>(x*other.x,y*other.y);} // component product
  T operator ^ ( const Vector2<T>& other ) const {return T(x*other.x+y*other.y);} // dot product

  // unary operators
  Vector2<T> operator + () const {return *this;}
  Vector2<T> operator - () const {return *this * -1;}
  Vector2<T> operator ~ () const {return Vector2<T>(T(1)/x,T(1)/y);}

  // binary operators with a matrix
  Vector2<T> operator * ( const Matrix2<T>& matrix ) const {
    return Vector2<T>(x*matrix.m11+y*matrix.m21,
                      x*matrix.m12+y*matrix.m22);
  }

  Vector2<T>& operator=(const Vector2<T>& other)  { x = other.x; y = other.y;return *this; }
  Vector2<T>& operator+=(const Vector2<T>& other) { x += other.x; y += other.y;return *this; }
  Vector2<T>& operator-=(const Vector2<T>& other) { x -= other.x; y -= other.y;return *this; }
  Vector2<T>& operator*=(const Vector2<T>& other) { x *= other.x; y *= other.y;return *this; }
  Vector2<T>& operator/=(const Vector2<T>& other) { x /= other.x; y /= other.y;return *this; }
  Vector2<T>& operator=(const T& other) { x = other; y = other;return *this; }
  Vector2<T>& operator+=(const T& other) { x += other; y += other;return *this; }
  Vector2<T>& operator-=(const T& other) { x -= other; y -= other;return *this; }
  Vector2<T>& operator*=(const T& other) { x *= other; y *= other;return *this; }
  Vector2<T>& operator/=(const T& other) { x /= other; y /= other;return *this; }

  friend std::ostream& operator<<(std::ostream &os,const Vector2<T>& v){os << v.x << '\t' << v.y; return os;}

  T& operator [](size_t i) {
    assert(i <= 1);
    return (i == 0) ? this->x : this->y;
  }
  const T& operator [](size_t i) const {
    assert(i <= 1);
    return (i == 0) ? this->x : this->y;
  }

  Vector2<T> abs() const {return Vector2<T>(fabs(x),fabs(y));}
  T area() const {return x*y;}
  T length() const {return sqrt(T(x*x+y*y));}
  void normalize() {T len = length(); x/=len;y/=len;}
  T maxVal() const {return MAX(x,y);}
  T minVal() const {return MIN(x,y);}

  Vector2<T> makepow2() const {
    Vector2<T> vOut;
    vOut.x = T(1<<int(ceil(log(double(x))/log(2.0))));
    vOut.y = T(1<<int(ceil(log(double(y))/log(2.0))));
    return vOut;
  }

  void storeMin(const Vector2<T> &other)
  {
    x = std::min(x,other.x);
    y = std::min(y,other.y);
  }

  void storeMax(const Vector2<T> &other)
  {
    x = std::max(x,other.x);
    y = std::max(y,other.y);
  }

  void glVertex()
  {
    glVertex2f(GLfloat(x),GLfloat(y));
  }
};


template <class T> Vector2<T> operator + ( T scalar, const Vector2<T>& vec ) {return Vector2<T>(scalar+vec.x,scalar+vec.y);}
template <class T> Vector2<T> operator - ( T scalar, const Vector2<T>& vec ) {return Vector2<T>(scalar-vec.x,scalar-vec.y);}
template <class T> Vector2<T> operator * ( T scalar, const Vector2<T>& vec ) {return Vector2<T>(scalar*vec.x,scalar*vec.y);}
template <class T> Vector2<T> operator / ( T scalar, const Vector2<T>& vec ) {return Vector2<T>(scalar/vec.x,scalar/vec.y);}
template <class T> Vector2<T> operator % ( T scalar, const Vector2<T>& vec ) {return Vector2<T>(scalar%vec.x,scalar%vec.y);}


template <class T=int> class Vector3
{
public:
  T x,y,z;

  Vector3<T>(): x(0), y(0),z(0) {}
  template <class S> explicit Vector3<T>(const std::vector<S>& v, const T& defaultVal = T(0) )
  {
    x = T(v.size()>0 ? v[0] : defaultVal);
    y = T(v.size()>1 ? v[1] : defaultVal);
    z = T(v.size()>2 ? v[2] : defaultVal);
  }

  Vector3<T>(const Vector3<T> &other): x(other.x), y(other.y), z(other.z) {}
  template <class S> explicit Vector3<T>(const Vector3<S> &other): x(T(other.x)), y(T(other.y)), z(T(other.z)) {}
  Vector3<T>(const Vector2<T> &other, const T _z): x(other.x), y(other.y), z(_z) {}
  Vector3<T>(const T _x, const T _y, const T _z) : x(_x), y(_y), z(_z) {}
  Vector3<T>(const T* vec) : x(vec[0]), y(vec[1]), z(vec[2]) {}


  bool operator == ( const Vector3<T>& other ) const {return (other.x==x && other.y==y && other.z==z); }
  bool operator != ( const Vector3<T>& other ) const {return (other.x!=x || other.y!=y || other.z!=z); }

    // binary operators with scalars
  Vector3<T> operator + ( T scalar ) const {return Vector3<T>(x+scalar,y+scalar,z+scalar);}
  Vector3<T> operator - ( T scalar ) const {return Vector3<T>(x-scalar,y-scalar,z-scalar);}
  Vector3<T> operator * ( T scalar ) const {return Vector3<T>(x*scalar,y*scalar,z*scalar);}
  Vector3<T> operator / ( T scalar ) const {return Vector3<T>(x/scalar,y/scalar,z/scalar);}
  Vector3<T> operator % ( T scalar ) const {return Vector3<T>(x%scalar,y%scalar,z%scalar);}

  // binary operators with vectors
  Vector3<T> operator + ( const Vector3<T>& other ) const {return Vector3<T>(x+other.x,y+other.y,z+other.z);}
  Vector3<T> operator - ( const Vector3<T>& other ) const {return Vector3<T>(x-other.x,y-other.y,z-other.z);}
  Vector3<T> operator / ( const Vector3<T>& other ) const {return Vector3<T>(x/other.x,y/other.y,z/other.z);}
  Vector3<T> operator * ( const Vector3<T>& other ) const {return Vector3<T>(x*other.x,y*other.y,z*other.z);} // component product
  Vector3<T> operator % ( const Vector3<T>& other ) const {return Vector3<T>(y*other.z-z*other.y,z*other.x-x*other.z,x*other.y-y*other.x);} // cross product
  T operator ^ ( const Vector3<T>& other ) const {return T(x*other.x+y*other.y+z*other.z);} // dot product

  // unary opartors
  Vector3<T> operator + () const {return *this;}
  Vector3<T> operator - () const {return *this * -1;}
  Vector3<T> operator ~ () const {return Vector3<T>(T(1)/x,T(1)/y,T(1)/z);}

  // binary operators with a matrix
  Vector3<T> operator * ( const Matrix3<T>& matrix ) const
  {
    return Vector3<T>(x*matrix.m11+y*matrix.m21+z*matrix.m31,
                      x*matrix.m12+y*matrix.m22+z*matrix.m32,
                      x*matrix.m13+y*matrix.m23+z*matrix.m33);
  }

  Vector3<T>& operator=(const Vector3<T>& other)  { x = other.x; y = other.y; z = other.z; return *this; }
  Vector3<T>& operator+=(const Vector3<T>& other) { x += other.x; y += other.y; z += other.z; return *this; }
  Vector3<T>& operator-=(const Vector3<T>& other) { x -= other.x; y -= other.y; z -= other.z; return *this; }
  Vector3<T>& operator*=(const Vector3<T>& other) { x *= other.x; y *= other.y; z *= other.z; return *this; }
  Vector3<T>& operator/=(const Vector3<T>& other) { x /= other.x; y /= other.y; z /= other.z; return *this; }
  Vector3<T>& operator=(const T& other) { x = other; y = other; z = other; return *this; }
  Vector3<T>& operator+=(const T& other) { x += other; y += other; z += other; return *this; }
  Vector3<T>& operator-=(const T& other) { x -= other; y -= other; z -= other; return *this; }
  Vector3<T>& operator*=(const T& other) { x *= other; y *= other; z *= other; return *this; }
  Vector3<T>& operator/=(const T& other) { x /= other; y /= other; z /= other; return *this; }

  friend std::ostream& operator<<(std::ostream &os,const Vector3<T>& v){os << v.x << '\t' << v.y << '\t' << v.z; return os;}

  T& operator [](size_t i)
  {
    assert(i <= 2);
    switch(i) {
      case 0: return this->x;
      case 1: return this->y;
      default: return this->z;
    }
  }
  const T& operator [](size_t i) const
  {
    assert(i <= 2);
    switch(i) {
      case 0: return this->x;
      case 1: return this->y;
      default: return this->z;
    }
  }

  Vector3<T> abs() const {return Vector3<T>(fabs(x),fabs(y),fabs(z));}
  T maxVal() const {return MAX(x,MAX(y,z));}
  T minVal() const {return MIN(x,MIN(y,z));}
  T volume() const {return x*y*z;}
  T length() const {return sqrt(T(x*x+y*y+z*z));}
  void normalize() {T len = length(); x/=len;y/=len;z/=len;}
  void normalize(T epsilon, const Vector3<T> replacement=Vector3<T>(T(0),T(0),T(1)))
  {
    T len = length();
    if (len > epsilon) {
      x/=len;
      y/=len;
      z/=len;
    } else { // specify some arbitrary normal
      x = replacement.x;
      y = replacement.y;
      z = replacement.z;
    }
  }
  Vector3<T> normalized() const
  {
    T len = length(); 
    return Vector3<T>(x/len,y/len,z/len);
  }
  Vector3<T> normalized(T epsilon,
                        const Vector3<T> replacement =
                        Vector3<T>(T(0),T(0),T(1))) const
  {
    T len = length(); 
    if (len > epsilon) {
      return Vector3<T>(x/len,y/len,z/len);
    } else { // specify some arbitrary normal
      return replacement;
    }
  }

  Vector2<T> xx() const {return Vector2<T>(x,x);}
  Vector2<T> xy() const {return Vector2<T>(x,y);}
  Vector2<T> xz() const {return Vector2<T>(x,z);}
  Vector2<T> yx() const {return Vector2<T>(y,x);}
  Vector2<T> yy() const {return Vector2<T>(y,y);}
  Vector2<T> yz() const {return Vector2<T>(y,z);}
  Vector2<T> zx() const {return Vector2<T>(z,x);}
  Vector2<T> zy() const {return Vector2<T>(z,y);}
  Vector2<T> zz() const {return Vector2<T>(z,z);}

  Vector3<T> makepow2() const
  {
    Vector3<T> vOut;
    vOut.x = T(1<<int(ceil(log(double(x))/log(2.0))));
    vOut.y = T(1<<int(ceil(log(double(y))/log(2.0))));
    vOut.z = T(1<<int(ceil(log(double(z))/log(2.0))));
    return vOut;
  }

  void storeMin(const Vector3<T> &other)
  {
    x = std::min(x,other.x);
    y = std::min(y,other.y);
    z = std::min(z,other.z);
  }

  void storeMax(const Vector3<T> &other)
  {
    x = std::max(x,other.x);
    y = std::max(y,other.y);
    z = std::max(z,other.z);
  }
};

template <class T> Vector3<T> operator + ( T scalar, const Vector3<T>& vec ) {return Vector3<T>(scalar+vec.x,scalar+vec.y,scalar+vec.z);}
template <class T> Vector3<T> operator - ( T scalar, const Vector3<T>& vec ) {return Vector3<T>(scalar-vec.x,scalar-vec.y,scalar-vec.z);}
template <class T> Vector3<T> operator * ( T scalar, const Vector3<T>& vec ) {return Vector3<T>(scalar*vec.x,scalar*vec.y,scalar*vec.z);}
template <class T> Vector3<T> operator / ( T scalar, const Vector3<T>& vec ) {return Vector3<T>(scalar/vec.x,scalar/vec.y,scalar/vec.z);}
template <class T> Vector3<T> operator % ( T scalar, const Vector3<T>& vec ) {return Vector3<T>(scalar%vec.x,scalar%vec.y,scalar%vec.z);}

// RedHat's patched gcc warns on the operator>> below if we don't explicitly
// mark the symbol as hidden, despite -fvisibility settings.
#if defined(__GNUC__) && __GNUC__ >= 4
# define HIDDEN __attribute__ ((visibility("hidden")))
#else
# define HIDDEN /* nothing */
#endif

template <class T=int> class Vector4
{
  template <class U>
  HIDDEN friend std::istream& operator >>(std::istream &, Vector4<U>&);
public:
  T x,y,z,w;

  Vector4<T>(): x(0), y(0),z(0), w(0) {}
  template <class S> explicit Vector4<T>( const std::vector<S>& v, const T& defaultVal = T(0) ) {
    x = T(v.size()>0 ? v[0] : defaultVal);
    y = T(v.size()>1 ? v[1] : defaultVal);
    z = T(v.size()>2 ? v[2] : defaultVal);
    w = T(v.size()>3 ? v[3] : defaultVal);
  }
  Vector4<T>(const Vector2<T> &other, const T _z, const T _w): x(other.x), y(other.y), z(_z), w(_w) {}
  Vector4<T>(const Vector3<T> &other, const T _w): x(other.x), y(other.y), z(other.z), w(_w) {}
  Vector4<T>(const Vector4<T> &other): x(other.x), y(other.y), z(other.z), w(other.w) {}
  template <class S> explicit Vector4<T>(const Vector4<S> &other): x(T(other.x)), y(T(other.y)), z(T(other.z)), w(T(other.w)) {}
  Vector4<T>(const Vector3<T> &other): x(other.x), y(other.y), z(other.z), w(1) {}
  Vector4<T>(const T _x, const T _y, const T _z, const T _w) : x(_x), y(_y), z(_z), w(_w) {}
  Vector4<T>(const T* vec) : x(vec[0]), y(vec[1]), z(vec[2]), w(vec[3]) {}

  bool operator == ( const Vector4<T>& other ) const {return (other.x==x && other.y==y && other.z==z && other.w==w); }
  bool operator != ( const Vector4<T>& other ) const {return (other.x!=x || other.y!=y || other.z!=z || other.w!=w); }

    // binary operators with scalars
  Vector4<T> operator + ( T scalar ) const {return Vector4<T>(x+scalar,y+scalar,z+scalar,w+scalar);}
  Vector4<T> operator - ( T scalar ) const {return Vector4<T>(x-scalar,y-scalar,z-scalar,w-scalar);}
  Vector4<T> operator * ( T scalar ) const {return Vector4<T>(x*scalar,y*scalar,z*scalar,w*scalar);}
  Vector4<T> operator / ( T scalar ) const {return Vector4<T>(x/scalar,y/scalar,z/scalar,w/scalar);}
  Vector4<T> operator % ( T scalar ) const {return Vector4<T>(x%scalar,y%scalar,z%scalar,w%scalar);}

  // binary operators with vectors
  Vector4<T> operator + ( const Vector4<T>& other ) const {return Vector4<T>(x+other.x,y+other.y,z+other.z,w+other.w);}
  Vector4<T> operator - ( const Vector4<T>& other ) const {return Vector4<T>(x-other.x,y-other.y,z-other.z,w-other.w);}
  Vector4<T> operator * ( const Vector4<T>& other ) const {return Vector4<T>(x*other.x,y*other.y,z*other.z,w*other.w);}
  Vector4<T> operator / ( const Vector4<T>& other ) const {return Vector4<T>(x/other.x,y/other.y,z/other.z,w/other.w);}
  T operator ^ ( const Vector4<T>& other ) const {return T(x*other.x+y*other.y+z*other.z+w*other.w);} // dot product

  // binary operators with a matrix
  Vector4<T> operator * ( const Matrix4<T>& matrix ) const
  {
    return Vector4<T>(x*matrix.m11+y*matrix.m21+z*matrix.m31+w*matrix.m41,
                      x*matrix.m12+y*matrix.m22+z*matrix.m32+w*matrix.m42,
                      x*matrix.m13+y*matrix.m23+z*matrix.m33+w*matrix.m43,
                      x*matrix.m14+y*matrix.m24+z*matrix.m34+w*matrix.m44);
  }

  // unary opartors
  Vector4<T> operator + () const {return *this;}
  Vector4<T> operator - () const {return *this * -1;}
  Vector4<T> operator ~ () const {return Vector4<T>(T(1)/x,T(1)/y,T(1)/z,T(1)/w);}

  T& operator [](size_t i)
  {
    assert(i <= 3);
    switch(i) {
      case 0: return this->x;
      case 1: return this->y;
      case 2: return this->z;
      default: return this->w;
    }
  }
  const T& operator [](size_t i) const
  {
    assert(i <= 3);
    switch(i) {
      case 0: return this->x;
      case 1: return this->y;
      case 2: return this->z;
      default: return this->w;
    }
  }

  Vector4<T>& operator=(const Vector4<T>& other)  { x = other.x; y = other.y; z = other.z; w = other.w; return *this; }
  Vector4<T>& operator+=(const Vector4<T>& other) { x += other.x; y += other.y; z += other.z; w += other.w; return *this; }
  Vector4<T>& operator-=(const Vector4<T>& other) { x -= other.x; y -= other.y; z -= other.z; w -= other.w; return *this; }
  Vector4<T>& operator*=(const Vector4<T>& other) { x *= other.x; y *= other.y; z *= other.z; w *= other.w; return *this; }
  Vector4<T>& operator/=(const Vector4<T>& other) { x /= other.x; y /= other.y; z /= other.z; w /= other.w; return *this; }
  Vector4<T>& operator=(const T& other)  { x = other; y = other; z = other; w = other; return *this; }
  Vector4<T>& operator+=(const T& other) { x += other; y += other; z += other;  w += other; return *this; }
  Vector4<T>& operator-=(const T& other) { x -= other; y -= other; z -= other;  w -= other; return *this; }
  Vector4<T>& operator*=(const T& other) { x *= other; y *= other; z *= other;  w *= other; return *this; }
  Vector4<T>& operator/=(const T& other) { x /= other; y /= other; z /= other;  w /= other; return *this; }

  friend std::ostream& operator<<(std::ostream &os,const Vector4<T>& v)
  {
    os << "[" << std::setiosflags(std::ios::left)
       << std::setw(7) << std::setprecision(3) << v.x
       << std::resetiosflags(std::ios::left)
       << std::setw(7) << std::setprecision(3) << v.y
       << std::setw(7) << std::setprecision(3) << v.z
       << std::setw(7) << std::setprecision(3) << v.w
       << "]";
    return os;
  }

  T max() const {return std::max(std::max(x,y),std::max(z,w));}
  T min() const {return std::min(std::min(x,y),std::min(z,w));}

  Vector4<T> abs() const {return Vector4<T>(fabs(x),fabs(y),fabs(z),fabs(w));}

  Vector4<T> makepow2() const
  {
    Vector4<T> vOut;
    vOut.x = T(1<<int(ceil(log(double(x))/log(2.0))));
    vOut.y = T(1<<int(ceil(log(double(y))/log(2.0))));
    vOut.z = T(1<<int(ceil(log(double(z))/log(2.0))));
    vOut.w = T(1<<int(ceil(log(double(w))/log(2.0))));
    return vOut;
  }

  Vector3<T> dehomo() const
  {
    Vector3<T> vOut;
    vOut.x = x/w;
    vOut.y = y/w;
    vOut.z = z/w;
    return vOut;
  }

  void storeMin(const Vector4<T> &other)
  {
    x = std::min(x,other.x);
    y = std::min(y,other.y);
    z = std::min(z,other.z);
    w = std::min(w,other.w);
  }

  void storeMax(const Vector4<T> &other)
  {
    x = std::max(x,other.x);
    y = std::max(y,other.y);
    z = std::max(z,other.z);
    w = std::max(w,other.w);
  }

  Vector2<T> xx() const {return Vector2<T>(x,x);}
  Vector2<T> xy() const {return Vector2<T>(x,y);}
  Vector2<T> xz() const {return Vector2<T>(x,z);}
  Vector2<T> yx() const {return Vector2<T>(y,x);}
  Vector2<T> yy() const {return Vector2<T>(y,y);}
  Vector2<T> yz() const {return Vector2<T>(y,z);}
  Vector2<T> zx() const {return Vector2<T>(z,x);}
  Vector2<T> zy() const {return Vector2<T>(z,y);}
  Vector2<T> zz() const {return Vector2<T>(z,z);}
  Vector2<T> zw() const {return Vector2<T>(z,w);}

  Vector3<T> xyz() const {return Vector3<T>(x,y,z);}
};

template <class T>
std::istream& operator >>(std::istream &is, Vector4<T>& v4)
{
  is >> v4[0];
  is >> v4[1];
  is >> v4[2];
  is >> v4[3];
  return is;
}

template <class T> Vector4<T> operator + ( T scalar, const Vector4<T>& vec ) {return Vector4<T>(scalar+vec.x,scalar+vec.y,scalar+vec.z,scalar+vec.w);}
template <class T> Vector4<T> operator - ( T scalar, const Vector4<T>& vec ) {return Vector4<T>(scalar-vec.x,scalar-vec.y,scalar-vec.z,scalar-vec.w);}
template <class T> Vector4<T> operator * ( T scalar, const Vector4<T>& vec ) {return Vector4<T>(scalar*vec.x,scalar*vec.y,scalar*vec.z,scalar*vec.w);}
template <class T> Vector4<T> operator / ( T scalar, const Vector4<T>& vec ) {return Vector4<T>(scalar/vec.x,scalar/vec.y,scalar/vec.z,scalar/vec.w);}
template <class T> Vector4<T> operator % ( T scalar, const Vector4<T>& vec ) {return Vector4<T>(scalar%vec.x,scalar%vec.y,scalar%vec.z,scalar%vec.w);}

typedef Vector4<> V4I;
typedef Vector3<> V3I;
typedef Vector2<> V2I;

typedef Vector4<uint32_t> V4UI;
typedef Vector3<uint32_t> V3UI;
typedef Vector2<uint32_t> V2UI;

typedef Vector4<uint64_t> V4UI64;
typedef Vector3<uint64_t> V3UI64;
typedef Vector2<uint64_t> V2UI64;

typedef Vector4<float> V4;
typedef Vector3<float> V3;
typedef Vector2<float> V2;

typedef Vector4<double> V4D;
typedef Vector3<double> V3D;
typedef Vector2<double> V2D;

template <class T=int> class Matrix2
{
public:
  union
  {
    struct
    {
      T m11,m12,
        m21,m22;
    };
    T array[4];
  };

  Matrix2() : m11(1), m12(0),
        m21(0), m22(1) {};
  Matrix2( const T *e ) : m11(e[0]),  m12(e[1]),
              m21(e[4]),  m22(e[5])  {};
    Matrix2( const Matrix2<T>& other ) : m11(other.m11), m12(other.m12),
                                  m21(other.m21), m22(other.m22) {};
    Matrix2( const Matrix3<T>& other ) : m11(other.m11), m12(other.m12),
                                  m21(other.m21), m22(other.m22) {};
    Matrix2( const Matrix4<T>& other ) : m11(other.m11), m12(other.m12),
                                  m21(other.m21), m22(other.m22) {};
  Matrix2( const Vector2<T> *rows ) : m11(rows[0].x), m12(rows[0].y),
                    m21(rows[1].x), m22(rows[1].y) {};
    Matrix2(T _m11, T _m12, T _m21, T _m22) : m11(_m11), m12(_m12),
                        m21(_m21), m22(_m22) {};

  bool operator == ( const Matrix2<T>& other ) const {return (other.m11==m11 && other.m12==m12 && other.m21==m21 && other.m22==m22); }
  bool operator != ( const Matrix2<T>& other ) const {return (other.m11!=m11 || other.m12!=m12 || other.m21!=m21 || other.m22!=m22); }

  friend std::ostream& operator<<(std::ostream &os,const Matrix2<T>& m){os << m.m11 << '\t' << m.m12 << '\n' << m.m21 << '\t' << m.m22; return os;}

  // binary operators with scalars
  Matrix2<T> operator * ( T scalar ) const {return Matrix2<T>(m11*scalar,m12*scalar,
                                m21*scalar,m22*scalar);}
  Matrix2<T> operator + ( T scalar ) const {return Matrix2<T>(m11+scalar,m12+scalar,
                                m21+scalar,m22+scalar);}
  Matrix2<T> operator - ( T scalar ) const {return Matrix2<T>(m11-scalar,m12-scalar,
                                m21-scalar,m22-scalar);}
  Matrix2<T> operator / ( T scalar ) const {return Matrix2<T>(m11/scalar,m12/scalar,
                                m21/scalar,m22/scalar);}


  // binary operators with matrices
  Matrix2<T> operator * ( const Matrix4<T>& other ) const
  {
    Matrix2<T> result;

    for (int x = 0;x<4;x+=2)
      for (int y = 0;y<2;y++)
        result[x+y] = array[0+x]*other.array[0+y] + array[1+x]*other.array[2+y];

    return result;
  }

  // binary operators with vectors
  Vector2<T> operator * ( const Vector2<T>& other ) const
  {
    return Vector2<T>(other.x*m11+other.y*m12,
                      other.x*m22+other.y*m22);
  }

  Matrix2<T> inverse() const
  {
    T determinant = m11*m22-m12*m21;
    return Matrix2<T> (m22/determinant, -m12/determinant,-m21/determinant, m11/determinant);
  }
};

template <class T> class Matrix3
{
public:
  union
  {
    struct
    {
      T m11,m12,m13,
        m21,m22,m23,
        m31,m32,m33;
    };
    T array[9];
  };

    Matrix3() : m11(1), m12(0), m13(0),
        m21(0), m22(1), m23(0),
        m31(0), m32(0), m33(1) {};
    Matrix3( const T *e ) : m11(e[0]), m12(e[1]), m13(e[2]),
              m21(e[3]), m22(e[4]), m23(e[5]),
              m31(e[6]), m32(e[7]), m33(e[8]) {};
    Matrix3( const Matrix3<T>& other ) : m11(other.m11), m12(other.m12), m13(other.m13),
                                  m21(other.m21), m22(other.m22), m23(other.m23),
                    m31(other.m31), m32(other.m32), m33(other.m33) {};
    Matrix3( const Matrix4<T>& other ) : m11(other.m11), m12(other.m12), m13(other.m13),
                                  m21(other.m21), m22(other.m22), m23(other.m23),
                    m31(other.m31), m32(other.m32), m33(other.m33) {};
    Matrix3( const Vector3<T> *rows ) : m11(rows[0].x), m12(rows[0].y), m13(rows[0].z),
                    m21(rows[1].x), m22(rows[1].y), m23(rows[1].z),
                    m31(rows[2].x), m32(rows[2].y), m33(rows[2].z) {};
    Matrix3(T _m11, T _m12, T _m13,
      T _m21, T _m22, T _m23,
      T _m31, T _m32, T _m33) : m11(_m11), m12(_m12), m13(_m13),
                    m21(_m21), m22(_m22), m23(_m23),
                    m31(_m31), m32(_m32), m33(_m33) {};

  bool operator == ( const Matrix3<T>& other ) const {return (other.m11==m11 && other.m12==m12 && other.m13==m13 &&
                                other.m21==m21 && other.m22==m22 && other.m23==m23 &&
                                other.m31==m31 && other.m32==m32 && other.m33==m33); }
  bool operator != ( const Matrix3<T>& other ) const {return (other.m11!=m11 || other.m12!=m12 || other.m13!=m13 ||
                                other.m21!=m21 || other.m22!=m22 || other.m23!=m23 ||
                                other.m31!=m31 || other.m32!=m32 || other.m33!=m33); }

  friend std::ostream& operator<<(std::ostream &os,const Matrix3<T>& m)
  {
    os  << m.m11 << '\t' << m.m12 << '\t' << m.m13 << '\n'
        << m.m21 << '\t' << m.m22 << '\t' << m.m23 << '\n'
        << m.m31 << '\t' << m.m32 << '\t' << m.m33;
    return os;
  }

  // binary operators with matrices
  Matrix3<T> operator * ( const Matrix3<T>& other ) const
  {
    Matrix3<T> result;
    for (int x = 0;x<9;x+=3)
      for (int y = 0;y<3;y++)
        result[x+y] = array[1+x]  * other.array[0+y]+
                      array[2+x]  * other.array[3+y]+
                      array[3+x]  * other.array[6+y];

    return result;
  }

  // binary operators with scalars
  Matrix3<T> operator * ( T scalar ) const {return Matrix3<T>(m11*scalar,m12*scalar,m13*scalar,
                                m21*scalar,m22*scalar,m23*scalar,
                                m31*scalar,m32*scalar,m33*scalar);}
  Matrix3<T> operator + ( T scalar ) const {return Matrix3<T>(m11+scalar,m12+scalar,m13+scalar,
                                m21+scalar,m22+scalar,m23+scalar,
                                m31+scalar,m32+scalar,m33+scalar);}
  Matrix3<T> operator - ( T scalar ) const {return Matrix3<T>(m11-scalar,m12-scalar,m13-scalar,
                                m21-scalar,m22-scalar,m23-scalar,
                                m31-scalar,m32-scalar,m33-scalar);}
  Matrix3<T> operator / ( T scalar ) const {return Matrix3<T>(m11/scalar,m12/scalar,m13/scalar,
                                m21/scalar,m22/scalar,m23/scalar,
                                m31/scalar,m32/scalar,m33/scalar);}

  // binary operators with vectors
  Vector3<T> operator * ( const Vector3<T>& other ) const
  {
    return Vector3<T>(other.x*m11+other.y*m12+other.z*m13,
                      other.x*m21+other.y*m22+other.z*m23,
                      other.x*m31+other.y*m32+other.z*m33);
  }

  void scaling(T x, T y, T z)
  {
    m11=x; m12=0; m13=0;
    m21=0; m22=y; m23=0;
    m31=0; m32=0; m33=z;
  }

  void rotationX(double angle)
  {
    T dCosAngle = T(cos(angle));
    T dSinAngle = T(sin(angle));

    m11=1;    m12=0;      m13=0;
    m21=0;    m22= dCosAngle;  m23=dSinAngle;
    m31=0;    m32=-dSinAngle;  m33=dCosAngle;
  }

  void rotationY(double angle)
  {
    T dCosAngle = T(cos(angle));
    T dSinAngle = T(sin(angle));

    m11=dCosAngle;  m12=0;  m13=-dSinAngle;
    m21=0;      m22=1;  m23=0;
    m31=dSinAngle;  m32=0;  m33=dCosAngle;
  }

  void rotationZ(double angle)
  {
    T dCosAngle = T(cos(angle));
    T dSinAngle = T(sin(angle));

    m11=dCosAngle;  m12=dSinAngle;  m13=0;
    m21=-dSinAngle;  m22=dCosAngle;  m23=0;
    m31=0;      m32=0;      m33=1;
  }

  void rotationAxis(const Vector3<T>& axis, double angle)
  {
    T dCosAngle = T(cos(angle));
    T dSinAngle = T(sin(angle));
    T dOneMinusCosAngle = 1-dCosAngle;

    Vector3<T> sqrAxis   = axis * axis;

    m11 = dCosAngle+dOneMinusCosAngle*sqrAxis.x;        m12 = dOneMinusCosAngle*axis.x*axis.y-dSinAngle*axis.z;  m13 = dOneMinusCosAngle*axis.x*axis.z+dSinAngle*axis.y;
    m21 = dOneMinusCosAngle*axis.x*axis.y+dSinAngle*axis.z;    m22 = dCosAngle+dOneMinusCosAngle*sqrAxis.y;      m23 = dOneMinusCosAngle*axis.y*axis.z-dSinAngle*axis.x;
    m31 = dOneMinusCosAngle*axis.x*axis.z-dSinAngle*axis.y;    m32 = dOneMinusCosAngle*axis.y*axis.z+dSinAngle*axis.x;  m33=dCosAngle+dOneMinusCosAngle*sqrAxis.z;
  }

  Matrix3<T> inverse() const
  {
    T determ = 1.0f/(  array[0]*(array[4]*array[8]-array[5]*array[7]) 
                     - array[1]*(array[3]*array[8]-array[5]*array[6])
                     + array[2]*(array[3]*array[7]-array[4]*array[6]));

    Matrix3<T> result;

    result.array[0]=determ*(array[4]*array[8]-array[5]*array[7]);
    result.array[3]=determ*(array[2]*array[7]-array[1]*array[8]);
    result.array[6]=determ*(array[1]*array[5]-array[2]*array[4]);
    result.array[1]=determ*(array[5]*array[6]-array[3]*array[8]);
    result.array[4]=determ*(array[0]*array[8]-array[2]*array[6]);
    result.array[7]=determ*(array[2]*array[3]-array[0]*array[5]);
    result.array[2]=determ*(array[3]*array[7]-array[4]*array[6]);
    result.array[5]=determ*(array[1]*array[6]-array[0]*array[7]);
    result.array[8]=determ*(array[0]*array[4]-array[1]*array[3]);

    return result;
  }
};

template <class T> class Matrix4
{
public:
  union
  {
    struct
    {
      T m11,m12,m13,m14,
        m21,m22,m23,m24,
        m31,m32,m33,m34,
        m41,m42,m43,m44;
    };
    T array[16];
  };

  Matrix4() : m11(1), m12(0), m13(0), m14(0),
              m21(0), m22(1), m23(0), m24(0),
              m31(0), m32(0), m33(1), m34(0),
              m41(0), m42(0), m43(0), m44(1) {};
  Matrix4( const T *e ) : m11(e[0]),  m12(e[1]),  m13(e[2]),  m14(e[3]),
              m21(e[4]),  m22(e[5]),  m23(e[6]),  m24(e[7]),
              m31(e[8]),  m32(e[9]),  m33(e[10]), m34(e[11]),
              m41(e[12]), m42(e[13]), m43(e[14]), m44(e[15])  {};
  Matrix4( const Matrix4<T>& other ) : 
      m11(other.m11), m12(other.m12), m13(other.m13), m14(other.m14),
      m21(other.m21), m22(other.m22), m23(other.m23), m24(other.m24),
      m31(other.m31), m32(other.m32), m33(other.m33), m34(other.m34),
      m41(other.m41), m42(other.m42), m43(other.m43), m44(other.m44) {};
  Matrix4( const Matrix3<T>& other ) :
      m11(other.m11), m12(other.m12), m13(other.m13), m14(0),
      m21(other.m21), m22(other.m22), m23(other.m23), m24(0),
      m31(other.m31), m32(other.m32), m33(other.m33), m34(0),
      m41(0), m42(0), m43(0), m44(1) {};
  Matrix4( const Vector4<T> *rows ) :
      m11(rows[0].x), m12(rows[0].y), m13(rows[0].z), m14(rows[0].w),
      m21(rows[1].x), m22(rows[1].y), m23(rows[1].z), m24(rows[1].w),
      m31(rows[2].x), m32(rows[2].y), m33(rows[2].z), m34(rows[2].w),
      m41(rows[3].x), m42(rows[3].y), m43(rows[3].z), m44(rows[3].w) {};
  Matrix4(T _m11, T _m12, T _m13, T _m14,
          T _m21, T _m22, T _m23, T _m24,
          T _m31, T _m32, T _m33, T _m34,
          T _m41, T _m42, T _m43, T _m44) :
      m11(_m11), m12(_m12), m13(_m13), m14(_m14),
      m21(_m21), m22(_m22), m23(_m23), m24(_m24),
      m31(_m31), m32(_m32), m33(_m33), m34(_m34),
      m41(_m41), m42(_m42), m43(_m43), m44(_m44) {};
  Matrix4(const Vector4<T> col0, const Vector4<T> col1,
          const Vector4<T> col2, const Vector4<T> col3)
  {
    setCol0(col0);
    setCol1(col1);
    setCol2(col2);
    setCol3(col3);
  }

  bool operator == ( const Matrix4<T>& other ) const
  {
    return (other.m11==m11 && other.m12==m12 && other.m13==m13 && other.m14==m14 &&
            other.m21==m21 && other.m22==m22 && other.m23==m23 && other.m24==m24 &&
            other.m31==m31 && other.m32==m32 && other.m33==m33 && other.m34==m34 &&
            other.m31==m41 && other.m32==m42 && other.m33==m43 && other.m44==m44);
  }

  bool operator != ( const Matrix4<T>& other ) const
  {
    return (other.m11!=m11 || other.m12!=m12 || other.m13!=m13 || other.m14!=m14 ||
            other.m21!=m21 || other.m22!=m22 || other.m23!=m23 || other.m24!=m24 ||
            other.m31!=m31 || other.m32!=m32 || other.m33!=m33 || other.m34!=m34 ||
            other.m41!=m41 || other.m42!=m42 || other.m43!=m43 || other.m44!=m44);
  }

  friend std::ostream& operator<<(std::ostream &os,const Matrix4<T>& m)
  {
    os  << m.m11 << '\t' << m.m12 << '\t' << m.m13 << '\t' << m.m14 << '\n'
        << m.m21 << '\t' << m.m22 << '\t' << m.m23 << '\t' << m.m24 << '\n'
        << m.m31 << '\t' << m.m32 << '\t' << m.m33 << '\t' << m.m34 << '\n'
        << m.m41 << '\t' << m.m42 << '\t' << m.m43 << '\t' << m.m44 ; return os;
  }

  operator T*(void) {return &m11;}
  operator const T*(void) const  {return &m11;}

  const T *operator *(void) const {return &m11;}
  T *operator *(void) {return &m11;}

  // binary operators with scalars
  Matrix4<T> operator * ( T scalar ) const {return Matrix4<T>(m11*scalar,m12*scalar,m13*scalar,m14*scalar,
                                m21*scalar,m22*scalar,m23*scalar,m24*scalar,
                                m31*scalar,m32*scalar,m33*scalar,m34*scalar,
                                m41*scalar,m42*scalar,m43*scalar,m44*scalar);}
  Matrix4<T> operator + ( T scalar ) const {return Matrix4<T>(m11+scalar,m12+scalar,m13+scalar,m14+scalar,
                                m21+scalar,m22+scalar,m23+scalar,m24+scalar,
                                m31+scalar,m32+scalar,m33+scalar,m34+scalar,
                                m41+scalar,m42+scalar,m43+scalar,m44+scalar);}
  Matrix4<T> operator - ( T scalar ) const {return Matrix4<T>(m11-scalar,m12-scalar,m13-scalar,m14-scalar,
                                m21-scalar,m22-scalar,m23-scalar,m24-scalar,
                                m31-scalar,m32-scalar,m33-scalar,m34-scalar,
                                m41-scalar,m42-scalar,m43-scalar,m44-scalar);}
  Matrix4<T> operator / ( T scalar ) const {return Matrix4<T>(m11/scalar,m12/scalar,m13/scalar,m14/scalar,
                                m21/scalar,m22/scalar,m23/scalar,m24/scalar,
                                m31/scalar,m32/scalar,m33/scalar,m34/scalar,
                                m41/scalar,m42/scalar,m43/scalar,m44/scalar);}

  Vector4<T> getCol0() {return Vector4<T>(m11, m21, m31, m41);}
  Vector4<T> getCol1() {return Vector4<T>(m12, m22, m32, m42);}
  Vector4<T> getCol2() {return Vector4<T>(m13, m23, m33, m43);}
  Vector4<T> getCol3() {return Vector4<T>(m14, m24, m34, m44);}

  void setCol0(const Vector4<T>& c)
  { m11 = c.x; m21 = c.y; m31 = c.z; m41 = c.w; }
  void setCol1(const Vector4<T>& c)
  { m12 = c.x; m22 = c.y; m32 = c.z; m42 = c.w; }
  void setCol2(const Vector4<T>& c)
  { m13 = c.x; m23 = c.y; m33 = c.z; m43 = c.w; }
  void setCol3(const Vector4<T>& c)
  { m14 = c.x; m24 = c.y; m34 = c.z; m44 = c.w; }

  inline const Vector3<T> getTranslation() const
  {
    return Vector3<T>(m14, m24, m34);
  }

  inline const void setTranslation(const Vector3<T>& t) const
  {
    m14 = t.x;
    m24 = t.y;
    m34 = t.z;
  }

  static const Matrix4<T> orthoInverse( const Matrix4<T> & mat )
  {
    // An orthogonal homogeonous inversion.
    Matrix4<T> out = mat.transpose();
    float x = mat.m14; float y = mat.m24; float z = mat.m34;
    out.m41 = 0; out.m42 = 0; out.m43 = 0; out.m44 = 1;
    Vector3<T> inv0 = out.getCol0().xyz();
    Vector3<T> inv1 = out.getCol1().xyz();
    Vector3<T> inv2 = out.getCol2().xyz();
    Vector3<T> trans = -(inv0*x + inv1*y + inv2*z);
    out.m14 = trans.x; out.m24 = trans.y; out.m34 = trans.z;
    return out;
  }

  static const Matrix4<T> lookAt(
      const Vector3<T>& eyePos, const Vector3<T>& lookAtPos,
      const Vector3<T>& upVec)
  {
    Matrix4<T> m4EyeFrame;
    Vector3<T> v3X, v3Y, v3Z;
    v3Y = upVec.normalized();
    v3Z = ( eyePos - lookAtPos ).normalized();
    v3X = ( v3Y % v3Z ).normalized(); // Cross product.
    v3Y = v3Z % v3X; // Cross product.
    m4EyeFrame = Matrix4<T>(Vector4<T>( v3X, 0 ), Vector4<T>( v3Y, 0 ), Vector4<T>( v3Z, 0 ), Vector4<T>( eyePos, 1 ));
    return orthoInverse( m4EyeFrame );
  }

  static const Matrix4<T> perspective(float fovyRadians, float aspect,
                                      float zNear, float zFar )
  {
    float f, rangeInv;
    f = tanf( ( (float)( PI / 2.0f ) - ( 0.5f * fovyRadians ) ) );
    rangeInv = ( 1.0f / ( zNear - zFar ) );
    return Matrix4<T>(
        Vector4<T>( ( f / aspect ), 0.0f, 0.0f, 0.0f ),
        Vector4<T>( 0.0f, f, 0.0f, 0.0f ),
        Vector4<T>( 0.0f, 0.0f, ( ( zNear + zFar ) * rangeInv ), -1.0f ),
        Vector4<T>( 0.0f, 0.0f, ( ( ( zNear * zFar ) * rangeInv ) * 2.0f ), 0.0f )
        );
  }

  // binary operators with matrices
  Matrix4<T> operator * ( const Matrix4<T>& other ) const
  {
    Matrix4<T> result;
    for (int x = 0;x<16;x+=4)
      for (int y = 0;y<4;y++)
        result[x+y] = array[0+x] * other.array[0+y]+
                      array[1+x] * other.array[4+y]+
                      array[2+x] * other.array[8+y]+
                      array[3+x] * other.array[12+y];

    return result;
  }

  // binary operators with vectors
  Vector4<T> operator * ( const Vector4<T>& other ) const
  {
    return Vector4<T>(other.x*m11+other.y*m12+other.z*m13+other.w*m14,
                      other.x*m21+other.y*m22+other.z*m23+other.w*m24,
                      other.x*m31+other.y*m32+other.z*m33+other.w*m34,
                      other.x*m41+other.y*m42+other.z*m43+other.w*m44);
  }

  Vector3<T> operator * ( const Vector3<T>& other ) const
  {
    return Vector3<T>(other.x*m11+other.y*m12+other.z*m13,
                      other.x*m21+other.y*m22+other.z*m23,
                      other.x*m31+other.y*m32+other.z*m33);
  }

  void translation(Vector3<T> trans)
  {
    translation(trans.x, trans.y, trans.z);
  }

  void scaling(Vector3<T> scale)
  {
    Scaling(scale.x, scale.y, scale.z);
  }

  void translation(T x, T y, T z)
  {
    m11=1; m12=0; m13=0; m14=0;
    m21=0; m22=1; m23=0; m24=0;
    m31=0; m32=0; m33=1; m34=0;
    m41=x; m42=y; m43=z; m44=1;
  }

  void scaling(T x, T y, T z)
  {
    m11=x; m12=0; m13=0; m14=0;
    m21=0; m22=y; m23=0; m24=0;
    m31=0; m32=0; m33=z; m34=0;
    m41=0; m42=0; m43=0; m44=1;
  }

  static Matrix4<T> rotationX(double angle)
  {
    Matrix4<T> o;

    T dCosAngle = T(cos(angle));
    T dSinAngle = T(sin(angle));

    o.m11=1;    o.m12=0;          o.m13=0;          o.m14=0;
    o.m21=0;    o.m22= dCosAngle; o.m23=dSinAngle;  o.m24=0;
    o.m31=0;    o.m32=-dSinAngle; o.m33=dCosAngle;  o.m34=0;
    o.m41=0;    o.m42=0;          o.m43=0;          o.m44=1;

    return o;
  }

  static Matrix4<T> rotationY(double angle)
  {
    Matrix4<T> o;

    T dCosAngle = T(cos(angle));
    T dSinAngle = T(sin(angle));

    o.m11=dCosAngle;  o.m12=0;  o.m13=-dSinAngle; o.m14=0;
    o.m21=0;          o.m22=1;  o.m23=0;          o.m24=0;
    o.m31=dSinAngle;  o.m32=0;  o.m33=dCosAngle;  o.m34=0;
    o.m41=0;          o.m42=0;  o.m43=0;          o.m44=1;

    return o;
  }

  static Matrix4<T> rotationZ(double angle)
  {
    Matrix4<T> o;

    T dCosAngle = T(cos(angle));
    T dSinAngle = T(sin(angle));

    o.m11=dCosAngle;  o.m12=dSinAngle;  o.m13=0;    o.m14=0;
    o.m21=-dSinAngle; o.m22=dCosAngle;  o.m23=0;    o.m24=0;
    o.m31=0;          o.m32=0;          o.m33=1;    o.m34=0;
    o.m41=0;          o.m42=0;          o.m43=0;    o.m44=1;

    return o;
  }

  static const Matrix4<T> orthographic(float left, float right,
                                       float bottom, float top,
                                       float zNear, float zFar)
  {
    float sum_rl, sum_tb, sum_nf, inv_rl, inv_tb, inv_nf;
    sum_rl = ( right + left );
    sum_tb = ( top + bottom );
    sum_nf = ( zNear + zFar );
    inv_rl = ( 1.0f / ( right - left ) );
    inv_tb = ( 1.0f / ( top - bottom ) );
    inv_nf = ( 1.0f / ( zNear - zFar ) );
    return Matrix4<T>(
        Vector4<T>( ( inv_rl + inv_rl ), 0.0f, 0.0f, 0.0f ),
        Vector4<T>( 0.0f, ( inv_tb + inv_tb ), 0.0f, 0.0f ),
        Vector4<T>( 0.0f, 0.0f, ( inv_nf + inv_nf ), 0.0f ),
        Vector4<T>( ( -sum_rl * inv_rl ), ( -sum_tb * inv_tb ), ( sum_nf * inv_nf ), 1.0f )
        );
  }

  void rotationAxis(const Vector3<T>& axis, double angle)
  {
    T dCosAngle = T(cos(angle));
    T dSinAngle = T(sin(angle));
    T dOneMinusCosAngle = 1-dCosAngle;

    Vector3<T> sqrAxis   = axis * axis;

    m11 = dCosAngle+dOneMinusCosAngle*sqrAxis.x;        m12 = dOneMinusCosAngle*axis.x*axis.y-dSinAngle*axis.z;  m13 = dOneMinusCosAngle*axis.x*axis.z+dSinAngle*axis.y;    m14=0;
    m21 = dOneMinusCosAngle*axis.x*axis.y+dSinAngle*axis.z;    m22 = dCosAngle+dOneMinusCosAngle*sqrAxis.y;      m23 = dOneMinusCosAngle*axis.y*axis.z-dSinAngle*axis.x;    m24=0;
    m31 = dOneMinusCosAngle*axis.x*axis.z-dSinAngle*axis.y;    m32 = dOneMinusCosAngle*axis.y*axis.z+dSinAngle*axis.x;  m33 = dCosAngle+dOneMinusCosAngle*sqrAxis.z;        m34=0;
    m41 = 0;                          m42 = 0;                        m43 = 0;                          m44=1;
  }

  Matrix4<T> transpose() const
  {
    Matrix4<T> result;

    result.m41 = m14;
    result.m31 = m13;  result.m42 = m24;
    result.m21 = m12;  result.m32 = m23;  result.m43 = m34;
    result.m11 = m11;  result.m22 = m22;  result.m33 = m33;  result.m44 = m44;
    result.m12 = m21;  result.m23 = m32;  result.m34 = m43;
    result.m13 = m31;  result.m24 = m42;
    result.m14 = m41;

    return result;
  }


  Matrix4<T> inverse() const
  {
    Matrix4<T> result;

    T Q =   m21 *(m34*( m12 * m43 - m13  * m42)+
            m14 *(-m32 * m43 + m42 * m33)+
            m44*( m13 *  m32 - m12  * m33))
          +
          m24 *(m11 *( m32 * m43 - m42 * m33)+
            m13 *(-m41* m32  + m31  * m42)+
            m12 *(-m31 * m43 + m41 * m33))
          +
          m44*(m22 *(-m31 *  m13 + m11  * m33)+
            m23 *(-m11 *  m32 + m12  * m31))
          +
          m34*(m11 *(-m22 * m43 + m23  * m42)+
            m41*( m13 * m22  - m23  * m12))
          +
          m14* (m23 *( m32 * m41 - m42 * m31)+
            m22 *( m31 * m43 - m41 * m33));

    result.m11 =  ( m24  * m32  * m43
        + m44 * m22  * m33
        - m44 * m23  * m32
        - m34 * m22  * m43
        - m24  * m42 * m33
        + m34 * m23  * m42)/Q;
      result.m21 = -( m21  * m44 * m33
        - m21  * m34 * m43
        - m44 * m23  * m31
        + m34 * m23  * m41
        + m24  * m31  * m43
        - m24  * m41 * m33)/Q;
    result.m31 = (- m21  * m34 * m42
        + m21  * m44 * m32
        - m44 * m31  * m22
        - m24  * m41 * m32
        + m34 * m41 * m22
        + m24  * m31  * m42)/Q;
      result.m41 =  -(m21  * m32  * m43
        - m21  * m42 * m33
        + m41 * m22  * m33
        - m32  * m23  * m41
        - m31  * m22  * m43
        + m42 * m23  * m31)/Q;
    /// 2
    result.m12 = (- m12  * m44 * m33
        + m12  * m34 * m43
        - m34 * m13  * m42
        - m14  * m32  * m43
        + m44 * m13  * m32
        + m14  * m42 * m33)/Q;

    result.m22 = (- m44 * m13  * m31
        + m44 * m11  * m33
        - m34 * m11  * m43
        - m14  * m41 * m33
        + m34 * m13  * m41
        + m14  * m31  * m43)/Q;

    result.m32 = -(-m12  * m44 * m31
        + m12  * m34 * m41
        + m44 * m11  * m32
        - m14  * m32  * m41
        + m14  * m42 * m31
        - m34 * m11  * m42)/Q;

    result.m42 = (- m12  * m31  * m43
        + m12  * m41 * m33
        + m11  * m32  * m43
        - m11  * m42 * m33
        - m41 * m13  * m32
        + m31  * m13  * m42)/Q;
    /// 3
    result.m13 = -( m44 * m13  * m22
        - m24  * m13  * m42
        - m14  * m22  * m43
        + m12  * m24  * m43
        - m12  * m44 * m23
        + m14  * m42 * m23)/Q;

    result.m23 = (- m21  * m14  * m43
        + m21  * m44 * m13
        + m24  * m11  * m43
        - m44 * m23  * m11
        - m24  * m41 * m13
        + m14  * m23  * m41)/Q;

    result.m33 = -(-m44 * m11  * m22
        + m44 * m12  * m21
        + m14  * m41 * m22
        + m24  * m11  * m42
        - m24  * m12  * m41
        - m14  * m21  * m42)/Q;

    result.m43 = -( m43 * m11  * m22
        - m43 * m12  * m21
        - m13  * m41 * m22
        - m23  * m11  * m42
        + m23  * m12  * m41
        + m13  * m21  * m42)/Q;
    /// 4
    result.m14 = (- m12  * m34 * m23
        + m12  * m24  * m33
        - m24  * m13  * m32
        - m14  * m22  * m33
        + m34 * m13  * m22
        + m14  * m32  * m23)/Q;

    result.m24 = -(-m21  * m14  * m33
        + m21  * m34 * m13
        + m24  * m11  * m33
        - m34 * m23  * m11
        + m14  * m23  * m31
        - m24  * m31  * m13)/Q;

    result.m34 = (- m34 * m11  * m22
        + m34 * m12  * m21
        + m14  * m31  * m22
        + m24  * m11  * m32
        - m24  * m12  * m31
        - m14  * m21  * m32)/Q;

    result.m44 =  ( m33 * m11  * m22
        - m33 * m12  * m21
        - m13  * m31  * m22
        - m23  * m11  * m32
        + m23  * m12  * m31
        + m13  * m21  * m32)/Q;
    return result;
  }

  static void buildStereoLookAtAndProjection(const Vector3<T> vEye, const Vector3<T> vAt, const Vector3<T> vUp,
                                             T fFOVY, T fAspect, T fZNear, T fZFar, T fFocalLength,
                                             T fEyeDist, int iEyeID, Matrix4<T>& mView, Matrix4<T>& mProj)
  {
    T radians = T(3.14159265358979323846/180.0) * fFOVY/T(2);
    T wd2     = fZNear * T(tan(radians));
    T nfdl    = fZNear / fFocalLength;
    T fShift  =   fEyeDist * nfdl;
    T left    = - fAspect * wd2 + T(iEyeID)*fShift;
    T right   =   fAspect * wd2 + T(iEyeID)*fShift;
    T top     =   wd2;
    T bottom  = - wd2;

    // projection matrix
    mProj.MatrixPerspectiveOffCenter(left, right, bottom, top, fZNear, fZFar);

    // view matrix
    mView.BuildLookAt(vEye, vAt, vUp);
    Matrix4<T> mTranslate;
    mTranslate.Translation(fEyeDist*T(iEyeID), 0.0f, 0.0f);
    mView= mTranslate * mView;
  }


  static void buildStereoLookAtAndProjection(const Vector3<T> vEye, const Vector3<T> vAt, const Vector3<T> vUp,
                                             T fFOVY, T fAspect, T fZNear, T fZFar, T fFocalLength,
                                             T fEyeDist, Matrix4<T>& mViewLeft, Matrix4<T>& mViewRight,
                                             Matrix4<T>& mProjLeft, Matrix4<T>& mProjRight)
  {
    T radians = T(3.14159265358979323846/180.0) *  fFOVY/2;
    T wd2     = fZNear * T(tan(radians));
    T nfdl    = fZNear / fFocalLength;
    T fShift  =   fEyeDist * nfdl;
    T left    = - fAspect * wd2 + fShift;
    T right   =   fAspect * wd2 + fShift;
    T top     =   wd2;
    T bottom  = - wd2;

    // projection matrices
    mProjLeft.MatrixPerspectiveOffCenter(left, right, bottom, top, fZNear, fZFar);
    left    = - fAspect * wd2 - fShift;
    right   =   fAspect * wd2 - fShift;
    mProjRight.MatrixPerspectiveOffCenter(left, right, bottom, top, fZNear, fZFar);

    // view matrices
    mViewLeft.BuildLookAt(vEye, vAt, vUp);
    mViewRight.BuildLookAt(vEye, vAt, vUp);

    // eye translation
    Matrix4<T> mTranslate;

    mTranslate.Translation(fEyeDist, 0.0f, 0.0f);
    mViewLeft = mTranslate * mViewLeft;

    mTranslate.Translation(-fEyeDist, 0.0f, 0.0f);
    mViewRight = mTranslate * mViewRight;
  }

  void buildLookAt(const Vector3<T> vEye, const Vector3<T> vAt, const Vector3<T> vUp)
  {
    Vector3<T> F = vAt-vEye;
    Vector3<T> U = vUp;
    Vector3<T> S = F % U;
    U = S % F;

    F.normalize();
    U.normalize();
    S.normalize();

    array[ 0]= S[0];  array[ 4]= S[1];  array[ 8]= S[2];  array[12]=-(S^vEye);
    array[ 1]= U[0];  array[ 5]= U[1];  array[ 9]= U[2];  array[13]=-(U^vEye);
    array[ 2]=-F[0];  array[ 6]=-F[1];  array[10]=-F[2];  array[14]= (F^vEye);
    array[ 3]= T(0);  array[ 7]=T(0);   array[11]=T(0);   array[15]= T(1);
  }

  //void perspective(T fovy, T aspect, T n, T f)
  //{
  //  // deg 2 rad
  //  fovy = fovy * T(3.14159265358979323846/180.0);

  //  T cotan = T(1.0/tan(double(fovy)/2.0));

  //  array[ 0]= cotan/aspect;  array[ 4]=T(0);     array[ 8]=T(0);             array[12]=T(0);
  //  array[ 1]= T(0);          array[ 5]=cotan;    array[ 9]=T(0);             array[13]=T(0);
  //  array[ 2]= T(0);          array[ 6]=T(0);     array[10]=-(f+n)/(f-n);     array[14]=T(-2)*(f*n)/(f-n);
  //  array[ 3]= T(0);          array[ 7]=T(0);     array[11]=T(-1);            array[15]=T(0);
  //}

  void ortho(T left, T right, T bottom, T top, T znear, T zfar )
  {
    array[ 0]= T(2)/(right-left);  array[ 4]=T(0);                array[ 8]=T(0);               array[12]=-(right+left)/(right-left);
    array[ 1]= T(0);               array[ 5]=T(2)/(top-bottom);   array[ 9]=T(0);               array[13]=-(top+bottom)/(top-bottom);
    array[ 2]= T(0);               array[ 6]=T(0);                array[10]=-T(2)/(zfar-znear); array[14]=-(zfar+znear)/(zfar-znear);
    array[ 3]= T(0);               array[ 7]=T(0);                array[11]=T(0);               array[15]=T(1);
  }

  void matrixPerspectiveOffCenter(T left, T right, T bottom, T top, T n, T f)
  {
    array[ 0]= T(2)*n/(right-left);   array[ 4]=T(0);                   array[ 8]=(right+left)/(right-left);  array[12]=T(0);
    array[ 1]= T(0);                  array[ 5]=T(2)*n/(top-bottom);    array[ 9]=(top+bottom)/(top-bottom);  array[13]=T(0);
    array[ 2]= T(0);                  array[ 6]=T(0);                   array[10]=-(f+n)/(f-n);               array[14]=T(-2)*(f*n)/(f-n);
    array[ 3]= T(0);                  array[ 7]=T(0);                   array[11]=T(-1);                      array[15]=T(0);
  }

};

typedef Matrix2<int> M22I;
typedef Matrix3<int> M33I;
typedef Matrix4<int> M44I;
typedef Matrix2<float> M22;
typedef Matrix3<float> M33;
typedef Matrix4<float> M44;
typedef Matrix2<double> M22D;
typedef Matrix3<double> M33D;
typedef Matrix4<double> M44D;

template <class T> class Quaternion4
{
public:
  float x, y, z, w;

  Quaternion4<T>(): x(0), y(0),z(0), w(0) {}
  Quaternion4<T>(T _x, T _y, T _z, T _w): x(_x), y(_y), z(_z), w(_w) {}
  Quaternion4<T>(const T& other) :
      x(other.x), y(other.y), z(other.z), w(other.w) {}

  Matrix4<T> computeRotation()
  {
    float n, s;
    float xs, ys, zs;
    float wx, wy, wz;
    float xx, xy, xz;
    float yy, yz, zz;

    n = (x * x) + (y * y) + (z * z) + (w * w);
    s = (n > 0.0f) ? (2.0f / n) : 0.0f;

    xs = x * s;
    ys = y * s;
    zs = z * s;
    wx = w * xs;
    wy = w * ys;
    wz = w * zs;
    xx = x * xs;
    xy = x * ys;
    xz = x * zs;
    yy = y * ys;
    yz = y * zs;
    zz = z * zs;

    return Matrix4<T>(T(1.0f - (yy + zz)), T(xy - wz),          T(xz + wy),          T(0),
                      T(xy + wz),          T(1.0f - (xx + zz)), T(yz - wx),          T(0),
                      T(xz - wy),          T(yz + wx),          T(1.0f - (xx + yy)), T(0),
                      T(0),                T(0),                T(0),                T(1));

  }

  bool operator == ( const Quaternion4<T>& other ) const {return (other.x==x && other.y==y && other.z==z && other.w==w); }
  bool operator != ( const Quaternion4<T>& other ) const {return (other.x!=x || other.y!=y || other.z!=z || other.w!=w); }

  // binary operators with other quaternions
  Quaternion4<T> operator + ( const Quaternion4<T>& other ) const {return Quaternion4<T>(x+other.x,y+other.y,z+other.z,w+other.w);}
  Quaternion4<T> operator - ( const Quaternion4<T>& other ) const {return Quaternion4<T>(x-other.x,y-other.y,z-other.z,w-other.w);}
  Quaternion4<T> operator * ( const Quaternion4<T>& other ) const
  {
    Vector3<T> v1(x,y,z);
    Vector3<T> v2(other.x,other.y,other.z);

    T _w = w * other.w - (v1 ^ v2);
    Vector3<T> _v = (v2 * w) + (v1 * other.w) + (v1 % v2);

    return Quaternion4<T>(_v.x, _v.y, _v.z, _w);
  }


  // binary operator with scalars
  Quaternion4<T> operator * ( const T other ) const
  {
    return Quaternion4<T>(x*other,y*other,z*other,w*other);
  }

  Quaternion4<T>& operator=(const Quaternion4<T>& other)  { x = other.x; y = other.y; z = other.z; w = other.w; return *this; }
  Quaternion4<T>& operator+=(const T& other) { x += other; y += other; z += other;  w += other; return *this; }
  Quaternion4<T>& operator-=(const T& other) { x -= other; y -= other; z -= other;  w -= other; return *this; }
};

typedef Quaternion4<float>  Quat;
typedef Quaternion4<double> QuatD;


/// Tests to see if the two values are basically the same.
template <class T> static bool epsilonEqual(T a, T b)
{
  /// @todo FIXME provide specializations for types; currently this would only
  ///       be correct if instantiated as a float!
  return fabs(a-b) <= std::numeric_limits<T>::epsilon();
}

/// a Plane is a Vector4 which is always normalized.
template <class T> class Plane : public Vector4<T>
{
public:
  Plane<T>(): Vector4<T>(0,0,0,0) {}
  // plane from paramters (usually all 4 are given)
  template <class S> explicit Plane<T>( const std::vector<S>& v )
{
    this->x = T(v.size()>0 ? v[0] : 0);
    this->y = T(v.size()>1 ? v[1] : 0);
    this->z = T(v.size()>2 ? v[2] : 0);
    this->w = T(v.size()>3 ? v[3] : 0);
  }
  // plane from points
  template <class S> explicit Plane<T>( const Vector3<S>& v0, const Vector3<S>& v1, const Vector3<S>& v2 ) {
    this->x =   T(v0.y) * (T(v1.z) - T(v2.z)) + T(v1.y) * (T(v2.z) - T(v0.z)) + T(v2.y) * (T(v0.z) - T(v1.z));
    this->y =   T(v0.z) * (T(v1.x) - T(v2.x)) + T(v1.z) * (T(v2.x) - T(v0.x)) + T(v2.z) * (T(v0.x) - T(v1.x));
    this->z =   T(v0.x) * (T(v1.y) - T(v2.y)) + T(v1.x) * (T(v2.y) - T(v0.y)) + T(v2.x) * (T(v0.y) - T(v1.y));
    this->w = - (T(v0.x) * (T(v1.y) * T(v2.z) - T(v2.y) * T(v1.z)) +
                 T(v1.x) * (T(v2.y) * T(v0.z) - T(v0.y) * T(v2.z)) +
                 T(v2.x) * (T(v0.y) * T(v1.z) - T(v1.y) * T(v0.z)));
  }
  Plane<T>(const Vector2<T> &other, const T _z, const T _w):
    Vector4<T>(other, _z, _w) {}
  Plane<T>(const Vector3<T> &other, const T _w = 1): Vector4<T>(other, _w) {}
  Plane<T>(const Vector4<T> &other): Vector4<T>(other) {}

  template <class S> explicit Plane<T>(const Plane<S> &other):
    Vector4<T>(other) {}

  Plane<T>(const T _x, const T _y, const T _z, const T _w) :
    Vector4<T>(_x,_y,_z,_w) {}
  Plane<T>(const T* vec) : Vector4<T>(vec) {}

  /// @return true if the given point is clipped by this plane.
  bool clip(Vector3<T> point) const
  {
    return ((V4::xyz() ^ point)+this->w >= 0);
  }

  /// Transform the plane by the given matrix.
  void transform(const Matrix4<T> &m)
  {
    M44 mIT(m.inverse());
    mIT = mIT.transpose();
    transformIT(mIT);
  }

  /// Transform the plane by the inverse transpose of the given matrix.
  void transformIT(const Matrix4<T> &M)
  {
    (*this) = V4::operator*(M);
    normalize();
  }

  void normalize()
  {
    const T x = this->x;
    const T y = this->y;
    const T z = this->z;
    T length = sqrt(x*x + y*y + z*z);
    (*this) /= length;
  }
  T d() const { return this->w; }
  Vector3<T> normal() const { return this->xyz(); }

  // binary operators with a matrix
  Plane<T> operator * ( const Matrix4<T>& matrix ) const
  {
    Plane<T> tmp(*this);
    tmp.transform(matrix);
    return tmp;
  }

  /// Determine the intersection point of the plane and a line `ab'.
  /// @return whether or not the two intersect.  If false, `hit' will not be
  ///         valid.
  bool intersect(const Vector3<T>& a, const Vector3<T>& b,
                 Vector3<T>& hit) const
  {
    T t;
    bool bIntersect = intersect(a,b,t);
    if (!bIntersect) return false;
    hit = a + (t*(b - a));
    return true;
  }

  /// Determine the parametric intersection point of the plane and a line `ab'.
  /// @return whether or not the two intersect.  If false, `hit' will not be
  ///         valid.
  bool intersect(const Vector3<T>& a, const Vector3<T>& b,
                 T& hit) const
  {
    const T denom = (*this).xyz() ^ (a - b);
    if(EpsilonEqual(denom, 0.0f)) {
      return false;
    }
    hit = (((*this).xyz() ^ a) + this->d()) / denom;
    return true;
  }

};

} // namespace Spire

#endif // SPIRE_CORE_VECTORS_H

