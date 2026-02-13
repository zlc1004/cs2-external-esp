#pragma once
#include <numbers>
#include <cmath>
#include <limits>

class Vec3_t {
public:
	float x, y, z;
public:
	inline Vec3_t() : x{}, y{}, z{} {}
	inline Vec3_t(float x, float y, float z) : x{ x }, y{ y }, z{ z } {}

	inline float& at(const size_t index) {
		return ((float*)this)[index];
	}
	inline float& at(const size_t index) const {
		return ((float*)this)[index];
	}

	inline float& operator( )(const size_t index) {
		return at(index);
	}
	inline const float& operator( )(const size_t index) const {
		return at(index);
	}
	inline float& operator[ ](const size_t index) {
		return at(index);
	}
	inline const float& operator[ ](const size_t index) const {
		return at(index);
	}

	inline bool operator==(const Vec3_t& v) const {
		return v.x == x && v.y == y && v.z == z;
	}
	inline bool operator!=(const Vec3_t& v) const {
		return v.x != x || v.y != y || v.z != z;
	}

	inline Vec3_t& operator=(const Vec3_t& v) {
		x = v.x;
		y = v.y;
		z = v.z;
		return *this;
	}

	inline Vec3_t operator-() const {
		return Vec3_t{ -x, -y, -z };
	}

	inline Vec3_t operator+(const Vec3_t& v) const {
		return {
			x + v.x,
			y + v.y,
			z + v.z
		};
	}

	inline Vec3_t operator-(const Vec3_t& v) const {
		return {
			x - v.x,
			y - v.y,
			z - v.z
		};
	}

	inline Vec3_t operator*(const Vec3_t& v) const {
		return {
			x * v.x,
			y * v.y,
			z * v.z
		};
	}

	inline Vec3_t operator/(const Vec3_t& v) const {
		return {
			x / v.x,
			y / v.y,
			z / v.z
		};
	}

	inline Vec3_t& operator+=(const Vec3_t& v) {
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	inline Vec3_t& operator-=(const Vec3_t& v) {
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}

	inline Vec3_t& operator*=(const Vec3_t& v) {
		x *= v.x;
		y *= v.y;
		z *= v.z;
		return *this;
	}

	inline Vec3_t& operator/=(const Vec3_t& v) {
		x /= v.x;
		y /= v.y;
		z /= v.z;
		return *this;
	}

	inline Vec3_t operator+(float f) const {
		return {
			x + f,
			y + f,
			z + f
		};
	}

	inline Vec3_t operator-(float f) const {
		return {
			x - f,
			y - f,
			z - f
		};
	}

	inline Vec3_t operator*(float f) const {
		return {
			x * f,
			y * f,
			z * f
		};
	}

	inline Vec3_t operator/(float f) const {
		return {
			x / f,
			y / f,
			z / f
		};
	}

	inline Vec3_t& operator+=(float f) {
		x += f;
		y += f;
		z += f;
		return *this;
	}

	inline Vec3_t& operator-=(float f) {
		x -= f;
		y -= f;
		z -= f;
		return *this;
	}

	inline Vec3_t& operator*=(float f) {
		x *= f;
		y *= f;
		z *= f;
		return *this;
	}

	inline Vec3_t& operator/=(float f) {
		x /= f;
		y /= f;
		z /= f;
		return *this;
	}

	inline void clear() {
		x = y = z = 0.f;
	}

	inline float length_sqr() const {
		return ((x * x) + (y * y) + (z * z));
	}

	inline float length_2d_sqr() const {
		return ((x * x) + (y * y));
	}

	inline float length() const {
		return std::sqrt(length_sqr());
	}

	inline float length_2d() const {
		return std::sqrt(length_2d_sqr());
	}

	inline float dot(const Vec3_t& v) const {
		return (x * v.x + y * v.y + z * v.z);
	}

	inline float dot(float* v) const {
		return (x * v[0] + y * v[1] + z * v[2]);
	}

	constexpr const bool zero() const noexcept {
		return x == 0.f && y == 0.f && z == 0.f;
	}

	inline Vec3_t cross(const Vec3_t& v) const {
		return {
			(y * v.z) - (z * v.y),
			(z * v.x) - (x * v.z),
			(x * v.y) - (y * v.x)
		};
	}

	inline float dist_to(const Vec3_t& vOther) const {
		Vec3_t delta;

		delta.x = x - vOther.x;
		delta.y = y - vOther.y;
		delta.z = z - vOther.z;

		return delta.length_2d();
	}

	inline float dist_to_3d(const Vec3_t& vOther) const {
		Vec3_t delta;

		delta.x = x - vOther.x;
		delta.y = y - vOther.y;
		delta.z = z - vOther.z;

		return delta.length();
	}

	const Vec3_t& ToAngle() const noexcept {
		return Vec3_t{
			std::atan2(-z, std::hypot(x, y)) * (180.0f / std::numbers::pi_v<float>),
			std::atan2(y, x) * (180.0f / std::numbers::pi_v<float>),
			0.0f
		};
	}

	inline float normalize() {
		float len = length();

		(*this) /= (length() + std::numeric_limits<float>::epsilon());

		return len;
	}

	inline Vec3_t normalized() const {
		auto vec = *this;

		vec.normalize();

		return vec;
	}
};