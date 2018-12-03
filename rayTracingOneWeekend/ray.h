#pragma once
#ifndef RAY_H
#define RAY_H

#include "vec3.h"

class ray {
public:
	ray() {}
	ray(const vec3& a, const vec3& b) { A = a; B = b; }
	
	vec3 origin() const { return A; }
	vec3 direction() const { return B; }
	vec3 pointAtParameter(float t) const { return A + t*B; }

	vec3 A;
	vec3 B;
};

class camera {
public: 
	camera(vec3 lowerLeftCorner, vec3 horizontal, vec3 vertical, vec3 origin) {
		_lowerLeftCorner = lowerLeftCorner;
		_horizontal = horizontal;
		_vertical = vertical;
		_origin = origin;
	}
	ray getRay(float u, float v) {
		return ray(_origin, _lowerLeftCorner + u * _horizontal + v * _vertical - _origin);
	}

	vec3 _origin;
	vec3 _lowerLeftCorner;
	vec3 _horizontal;
	vec3 _vertical;
};
#endif
