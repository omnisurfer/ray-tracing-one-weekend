#pragma once

#include <math.h>

class vec4 {
public:
	vec4() {}
	vec4(float e0, float e1, float e2, float e3) { e[0] = e0; e[1] = e1; e[2] = e2; e[3] = e3; }

	inline float x() const { return e[0]; }
	inline float y() const { return e[1]; }
	inline float z() const { return e[2]; }
	inline float w() const { return e[3]; }

	inline float r() const { return e[0]; }
	inline float g() const { return e[1]; }
	inline float b() const { return e[2]; }
	inline float a() const { return e[3]; }

	inline const vec4& operator+() const { return *this; }	
	inline vec4 operator-() const { return vec4(-e[0], -e[1], -e[2], -e[3]); }
	inline float operator[](int i) const { return e[i]; }
	inline float& operator[](int i) { return e[i]; }

	inline vec4& operator+=(const vec4 &v2);
	inline vec4& operator-=(const vec4 &v2);
	inline vec4& operator*=(const vec4 &v2);
	inline vec4& operator/=(const vec4 &v2);
	inline vec4& operator*=(const float t);
	inline vec4& operator/=(const float t);

	inline float length() const {
		return sqrt(e[0]*e[0] + e[1]*e[1] + e[2]*e[2] + e[3]*e[3]);
	}

	inline float squared_length() const {
		return e[0]*e[0] + e[1]*e[1] + e[2]*e[2] + e[3]*e[3];
	}

	inline void make_unit_vector();

	float e[4];
};

inline std::istream& operator>>(std::istream &is, vec4 &t) {
	is >> t.e[0] >> t.e[1] >> t.e[2] >> t.e[3];
}

inline std::ostream& operator<<(std::ostream &os, const vec4 &t) {
	os << t.e[0] << "x + " << t.e[1] << "y + " << t.e[2] << "z + " << t.e[3] << "w";
	return os;
}

inline void vec4::make_unit_vector() {
	float k = 1.0 / sqrt(e[0] * e[0] + e[1] * e[1] + e[2] * e[2] + e[3] * e[3]);

	e[0] *= k; e[1] *= k; e[2] *= k; e[3] *= k;
}

inline vec4 operator+(const vec4 &v1, const vec4 &v2) {
	return vec4(v1.e[0] + v2.e[0], v1.e[1] + v2.e[1], v1.e[2] + v2.e[2], v1.e[3] + v2.e[3]);
}

inline vec4 operator-(const vec4 &v1, const vec4 &v2) {
	return vec4(v1.e[0] - v2.e[0], v1.e[1] - v2.e[1], v1.e[2] - v2.e[2], v1.e[3] - v2.e[3]);
}

inline vec4 operator*(const vec4 &v1, const vec4 &v2) {
	return vec4(v1.e[0] * v2.e[0], v1.e[1] * v2.e[1], v1.e[2] * v2.e[2], v1.e[3] * v2.e[3]);
}

inline vec4 operator/(const vec4 &v1, const vec4 &v2) {
	return vec4(v1.e[0] / v2.e[0], v1.e[1] / v2.e[1], v1.e[2] / v2.e[2], v1.e[3] / v2.e[3]);
}

inline vec4 operator*(float t, const vec4 &v) {
	return vec4(t*v.e[0], t*v.e[1], t*v.e[2], t*v.e[3]);
}

inline vec4 operator*(const vec4 &v, float t) {
	return vec4(t*v.e[0], t*v.e[1], t*v.e[2], t*v.e[3]);
}

inline vec4 operator/(const vec4 &v, float t) {
	return vec4(v.e[0] / t, v.e[1] / t, v.e[2] / t, v.e[3] / t);
}

inline float dot(const vec4 &v1, const vec4 &v2) {
	return v1.e[0] * v2.e[0] + v1.e[1] * v2.e[1] + v1.e[2] * v2.e[2] + v1.e[3] * v2.e[3];
}

/* Only defined for 3D vectors */
/*
inline vec4 cross(const vec4 &v1, const vec4 &v2) {
	
	return vec4(
		(v1.e[1] * v2.e[2] - v1.e[2] * v2.e[1]),
		(-(v1.e[0] * v2.e[2] - v1.e[2] * v2.e[0])),
		(v1.e[0] * v2.e[1] - v1.e[1] * v2.e[0])
	);
}
*/

inline vec4& vec4::operator+=(const vec4 &v) {
	e[0] += v.e[0];
	e[1] += v.e[1];
	e[2] += v.e[2];
	e[3] += v.e[3];
	return *this;
}

inline vec4& vec4::operator*=(const vec4 &v) {
	e[0] *= v.e[0];
	e[1] *= v.e[1];
	e[2] *= v.e[2];
	e[3] *= v.e[3];
	return *this;
}

inline vec4& vec4::operator/=(const vec4 &v) {
	e[0] /= v.e[0];
	e[1] /= v.e[1];
	e[2] /= v.e[2];
	e[3] /= v.e[3];
	return *this;
}

inline vec4& vec4::operator-=(const vec4 &v) {
	e[0] -= v.e[0];
	e[1] -= v.e[1];
	e[2] -= v.e[2];
	e[3] -= v.e[3];
	return *this;
}

inline vec4& vec4::operator*=(const float t) {
	e[0] *= t;
	e[1] *= t;
	e[2] *= t;
	e[3] *= t;
	return *this;
}

inline vec4& vec4::operator/=(const float t) {
	float k = 1.0 / t;
	e[0] *= k;
	e[1] *= k;
	e[2] *= k;
	e[3] *= k;
	return *this;
}

inline vec4 unit_vector(vec4 v) {
	if (v.length() == 0.0) {
		return vec4(0.0, 0.0, 0.0, 0.0);
	}
	else {
		return v / v.length();
	}
}