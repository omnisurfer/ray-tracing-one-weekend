#pragma once
#ifndef RAY_H
#define RAY_H

#include "vec3.h"
/* Windows specific, need to define the following*/
#define _USE_MATH_DEFINES
#include <math.h>

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
	camera(vec3 lookFrom, vec3 lookAt, vec3 upDirection, float vfov, float aspect) { //vfov is top to bottom in degrees
		vec3 u, v, w;
		float theta = vfov * M_PI / 180;
		float half_height = tan(theta / 2);
		float half_width = aspect * half_height;

		_origin = lookFrom;
		w = unit_vector(lookFrom - lookAt);
		u = unit_vector(cross(upDirection, w));
		v = cross(w, u);

		_lowerLeftCorner = vec3(-half_width, -half_height, -1.0);
		_lowerLeftCorner = _origin - half_width * u - half_height * v - w;

		_horizontal = 2*half_width*u;
		_vertical = 2*half_height*v;		
	}
	ray getRay(float s, float t) {
		return ray(_origin, _lowerLeftCorner + s * _horizontal + t * _vertical - _origin);
	}

	vec3 _origin;
	vec3 _lowerLeftCorner;
	vec3 _horizontal;
	vec3 _vertical;
};
#endif
