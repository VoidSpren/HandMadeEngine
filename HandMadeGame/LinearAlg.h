#pragma once

#include "utilDefs.h"

namespace voi {
	/* simple vector2 struct */
	template<typename T>
	struct Vec2 {
		T x;
		T y;

		Vec2() : x(0), y(0) {}
		Vec2(T x, T y) : x(x), y(y) {}
		Vec2(const Vec2& other) : x(other.x), y(other.y) {}
		template<typename E> Vec2(const Vec2<E>& other) : x(other.x), y(other.y) {}

		static T dotProd(const Vec2& a, const Vec2& b) { return a.x * b.x + a.y * b.y; }
		static Vec2 unit(const Vec2& in) {
			float l = sqrtf(in.x * in.x + in.y * in.y);
			return { in.x / l, in.y / l };
		}
		static Vec2 lerp(const Vec2& a, const Vec2& b, float f) {
			return ((b - a) * f) + a;
		}
		static T Sum(const Vec2& in) { return in.x + in.y; }


		void toUnit() {
			float l = sqrtf(x * x + y * y);
			x / l; y / l;
		}
		T Sum() const { return x + y; }
		

		Vec2 operator + (const Vec2& o) const { return { x + o.x, y + o.y }; }
		Vec2 operator - (const Vec2& o) const { return { x - o.x, y - o.y }; }
		Vec2 operator * (const Vec2& o) const { return { x * o.x, y * o.y }; }
		Vec2 operator * (const T s) const { return { x * s, y * s }; }
		Vec2 operator += (const Vec2& o) { x += o.x; y += o.y; return *this; }
		Vec2 operator -= (const Vec2& o) { x -= o.x; y -= o.y; return *this; }
		Vec2 operator *= (const Vec2& o) { x *= o.x; y *= o.y; return *this; }
		Vec2 operator *= (const T& s) { x *= s; y *= s; return *this; }

	};
	typedef Vec2<int> Vec2i;
	typedef Vec2<i64> Vec2l;
	typedef Vec2<float> Vec2f;
	typedef Vec2<double> Vec2d;

	/* simple vector3 struct */
	template<typename T>
	struct Vec3 {
		T x, y, z;

		Vec3() : x(0), y(0), z(0) {}
		Vec3(T x, T y, T z) : x(x), y(y), z(z) {}
		Vec3(const Vec3& other) : x(other.x), y(other.y), z(other.z) {}
		template<typename E> Vec3(const Vec3<E>& other) : x(other.x), y(other.y) {}


		static T dotProd(const Vec3& a, const Vec3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
		static Vec3 cross(const Vec3& a, const Vec3& b) { return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x }; }
		static Vec3 unit(const Vec3& in) {
			float l = sqrtf(in.x * in.x + in.y * in.y + in.z * in.z);
			return { in.x / l, in.y / l,in.z / l };
		}
		static Vec3 lerp(const Vec3& a, const Vec3& b, float f) { return ((b - a) * f) + a; }
		static T Sum(const Vec3& in) { return in.x + in.y + in.z; }

		void toUnit() {
			float l = sqrtf(x * x + y * y + z * z);
			x / l; y / l; z / l;
		}
		T Sum() const { return x + y + z; }
		T vecMin() const {
			if (x <= y && x <= z) return x;
			if (y <= x && y <= z) return y;
			if (z <= x && z <= y) return z;
		}
		T vecMax() const {
			if (x >= y && x >= z) return x;
			if (y >= x && y >= z) return y;
			if (z >= x && z >= y) return z;
		}

		Vec3 operator + (const Vec3& o) const { return { x + o.x, y + o.y, z + o.z }; }
		Vec3 operator - (const Vec3& o) const { return { x - o.x, y - o.y, z - o.z }; }
		Vec3 operator * (const Vec3& o) const { return { x * o.x, y * o.y, z * o.z }; }
		Vec3 operator * (const T s) const { return { x * s, y * s, z * s }; }
		Vec3 operator += (const Vec3& o) { x += o.x; y += o.y; z += o.z; return *this; }
		Vec3 operator -= (const Vec3& o) { x -= o.x; y -= o.y; z -= o.z; return *this; }
		Vec3 operator *= (const Vec3& o) { x *= o.x; y *= o.y; z *= o.z; return *this; }
		Vec3 operator *= (const T& s) { x *= s; y *= s; z *= s; return *this; }

	};
	typedef Vec3<int> Vec3i;
	typedef Vec3<i64> Vec3l;
	typedef Vec3<float> Vec3f;
	typedef Vec3<double> Vec3d;

	struct Pixel;

	/* simple vector4 struct */
	template<typename T>
	struct Vec4 {
		T x, y, z, w;

		Vec4() : x(0), y(0), z(0), w(0) {}
		Vec4(T x, T y, T z) : x(x), y(y), z(z), w((T)1) {}
		Vec4(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}
		Vec4(const Vec4& other) : x(other.x), y(other.y), z(other.z), w(other.w) {}
		Vec4(const Pixel& other);
		template<typename E> Vec4(const Vec4<E>& other) : x(other.x), y(other.y), w(other.w) {}


		//static T dotProd(const Vec4& a, const Vec4& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
		//static Vec4 cross(const Vec4& a, const Vec4& b) { return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x }; }
		//static Vec4 unit(const Vec4& in) {
		//	float l = sqrtf(in.x * in.x + in.y * in.y + in.z * in.z);
		//	return { in.x / l, in.y / l,in.z / l };
		//}

		static Vec4 lerp(const Vec4& a, const Vec4& b, float f) { return ((b - a) * f) + a; }
		static T Sum(const Vec4& in) { return in.x + in.y + in.z + in.w; }

		void toUnit() {
			float l = sqrtf(x * x + y * y + z * z + w * w);
			x / l; y / l; z / l; w / l;
		}
		T Sum() const { return x + y + z + w; }
		//T vecMin() const {
		//	if (x <= y && x <= z) return x;
		//	if (y <= x && y <= z) return y;
		//	if (z <= x && z <= y) return z;
		//}
		//T vecMax() const {
		//	if (x >= y && x >= z) return x;
		//	if (y >= x && y >= z) return y;
		//	if (z >= x && z >= y) return z;
		//}

		Vec4 operator + (const Vec4& o) const { return { x + o.x, y + o.y, z + o.z, w + o.w }; }
		Vec4 operator - (const Vec4& o) const { return { x - o.x, y - o.y, z - o.z, w - o.w }; }
		Vec4 operator * (const Vec4& o) const { return { x * o.x, y * o.y, z * o.z, w * o.w }; }
		Vec4 operator * (const T s) const { return { x * s, y * s, z * s, w * s }; }
		Vec4 operator += (const Vec4& o) { x += o.x; y += o.y; z += o.z; w += o.w; return *this; }
		Vec4 operator -= (const Vec4& o) { x -= o.x; y -= o.y; z -= o.z; w -= o.w; return *this; }
		Vec4 operator *= (const Vec4& o) { x *= o.x; y *= o.y; z *= o.z; w *= o.w; return *this; }
		Vec4 operator *= (const T& s) { x *= s; y *= s; z *= s; w *= s; return *this; }

	};
	typedef Vec4<int> Vec4i;
	typedef Vec4<i64> Vec4l;
	typedef Vec4<float> Vec4f;
	typedef Vec4<double> Vec4d;

}
