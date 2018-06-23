#pragma once

/// ----------------------------------------------------------------------------
/// Standard types
/// ----------------------------------------------------------------------------
#include <cmath>

typedef double Scalar;
typedef float ScalarCompressed;
typedef unsigned __int64 BigUInt;

// A 3D Scalar vector type.
struct Vec3
{
	union {
		struct {
			Scalar x, y, z;
		};
		Scalar data[3];
	};

	Vec3() {}
	Vec3(Scalar x, Scalar y, Scalar z) { this->x = x; this->y = y; this->z = z; }

	// Component multiply/divide.
	Vec3 CMultiply(const Vec3& other) const { return Vec3(x*other.x, y*other.y, z*other.z); }
	Vec3 CDivision(const Vec3& other) const { return Vec3(x/other.x, y/other.y, z/other.z); }
	Scalar Length2() { return x*x+y*y+z*z; }
	Scalar Length() { return std::sqrt(x*x+y*y+z*z); }
	Vec3 Normal() { Scalar l = Length(); return Vec3(x/l, y/l, z/l); }
	void Normalize() { Scalar l = Length(); x /= l; y /= l; z/= l; }
};

// Standard vector operators, * dot product, ^ cross product.
inline Vec3 operator+(const Vec3& v1, const Vec3& v2) { return Vec3(v1.x+v2.x, v1.y+v2.y, v1.z+v2.z); }
inline Vec3 operator-(const Vec3& v1, const Vec3& v2) { return Vec3(v1.x-v2.x, v1.y-v2.y, v1.z-v2.z); }
inline Vec3& operator+=(Vec3& v1, const Vec3& v2) { v1.x+=v2.x; v1.y+=v2.y; v1.z+=v2.z; return v1; }
inline Vec3& operator-=(Vec3& v1, const Vec3& v2) { v1.x-=v2.x; v1.y-=v2.y; v1.z-=v2.z; return v1;}
inline Vec3 operator^(const Vec3& v1, const Vec3& v2) { return Vec3(v1.y*v2.z-v2.y*v1.z, -v1.x*v2.z+v2.x*v1.z, v1.x*v2.y-v2.x*v1.y); }
inline Scalar operator*(const Vec3& v1, const Vec3& v2) { return v1.x*v2.x+v1.y*v2.y+v1.z*v2.z; }
inline Vec3 operator*(const Vec3& v1, const Scalar f) { return Vec3(v1.x*f, v1.y*f, v1.z*f); }
inline Vec3 operator*(const Scalar f, const Vec3& v1) { return Vec3(v1.x*f, v1.y*f, v1.z*f); }
inline Vec3 operator/(const Vec3& v1, const Scalar f) { return Vec3(v1.x/f, v1.y/f, v1.z/f); }
inline Vec3 operator-(const Vec3& v) { return Vec3(-v.x, -v.y, -v.z); }