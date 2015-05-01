//======== (C) Copyright 2006 Menno Nijboer. All rights reserved. ========
// Solstice Ray Trace Engine
//
// Content: 
//
// 3D vector class using overloaded operators
//
//========================================================================

#pragma once

#include <float.h>
#include <math.h>

#define PI 3.1415926535897932384f
#define TO_RADIANS (PI / 180.0f)
#define TO_DEGREES (180.0f / PI)

template <typename T> inline
const T & myMin(T left, T right) {
  return (right < left ? right : left);
}

template <typename T> inline
const T & myMax(T left, T right) {
  return (right > left ? right : left);
}


class Vector2 {
public:

  float x, y;
	
  // CONST
  Vector2() : x(0.0f), y(0.0f) {
  }
  Vector2(float x1, float y1) : x(x1), y(y1) {
  }
  ~Vector2() { }
  // END CONST

  //Inline operator code
  Vector2 operator+(Vector2 &v) const {
    return Vector2 (x + v.x, y + v.y);
  }

  Vector2 operator+=(Vector2 &v) {
    x += v.x;
    y += v.y;
    return *this;
  }

  Vector2 operator-(Vector2 &v) const {
    return Vector2 (x - v.x, y - v.y);
  }

  Vector2 operator-=(Vector2 &v) {
    x -= v.x;
    y -= v.y;
    return *this;
  }
	
  Vector2 operator*(float f) const {
    return Vector2(f*x, f*y);
  }

  Vector2 &operator*=(float f) {
    x *= f;
    y *= f;
    return *this;
  }

  Vector2 operator/(float f) const {
    float inv = 1.f / f;
    return Vector2 (x*inv, y*inv);
  }

  Vector2 &operator/=(float f) {
    float inv = 1.f / f;
    x *= inv;
    y *= inv;
    return *this;
  }

  Vector2 operator-() const {
    return Vector2 (-x, -y);
  }

  float operator[](int i) const {
    //assert(i < 3);
    return (&x)[i];
  }

  float &operator[](int i) {
    //assert(i < 3);
    return (&x)[i];
  }
  //end inline operator code

  float lengthSquared() const {
    return x*x + y*y;
  }

  float length() const {
    return sqrtf( lengthSquared() );
  }
  void normalize() {
    float len = length();
    float inv = 1.0f/len;
    x *= inv;
    y *= inv;
  }

  Vector2 normalized() const {
    return (*this)/length();
  }

	float dot(Vector2& v) const {
		return x*v.x + y*v.y;
	}

	float calcAngleWith(Vector2& v) {
		Vector2 v1 = (*this).normalized();
		Vector2 v2 = v.normalized();
		return acos(v1.dot(v2));
	}

	float calcSignedAngleWith(Vector2& v) {
		return atan2f(v.y,v.x) - atan2f(y,x);
	}

	void rotateAroundZero(float angle)
	{
		float xx = x; float yy = y;
		x = xx*cos(angle) - yy*sin(angle);
		y = xx*sin(angle) + yy*cos(angle);
	}
};

