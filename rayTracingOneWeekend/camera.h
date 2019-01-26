#pragma once

#include "vec3.h"
/* Windows specific, need to define the following*/
#define _USE_MATH_DEFINES
#include <math.h>

vec3 randomInUnitDisk() {
	vec3 p;
	do {
		p = 2.0 * vec3(unifRand(randomNumberGenerator), unifRand(randomNumberGenerator), 0) - vec3(1, 1, 0);
	} while (dot(p, p) >= 1.0);
	return p;
}

class camera {
public:
	camera(vec3 lookFrom, vec3 lookAt, vec3 upDirection, float vfov, float aspect, float aperture, float focusDistance) { //vfov is top to bottom in degrees
		lensRadius = aperture / 2;

		float theta = vfov * M_PI / 180;
		float half_height = tan(theta / 2);
		float half_width = aspect * half_height;

		_origin = lookFrom;
		w = unit_vector(lookFrom - lookAt);
		u = unit_vector(cross(upDirection, w));
		v = cross(w, u);

		//_lowerLeftCorner = vec3(-half_width, -half_height, -1.0);
		_lowerLeftCorner = _origin -
			half_width * focusDistance * u -
			half_height * focusDistance * v -
			focusDistance * w;

		_horizontal = 2 * half_width * focusDistance * u;
		_vertical = 2 * half_height * focusDistance * v;
	}
	ray getRay(float s, float t) {
		//DEBUG - DISABLED SO THAT I CAN RENDER WITHOUT DOF BLUR
		vec3 rd = lensRadius * randomInUnitDisk();// +randomInUnitSphere();
		vec3 offset = u * rd.x() + v * rd.y();

		return ray(_origin + offset, _lowerLeftCorner + s * _horizontal + t * _vertical - _origin - offset);
	}

	vec3 _origin;
	vec3 _lowerLeftCorner;
	vec3 _horizontal;
	vec3 _vertical;
	//orthonormal basis vectors that the camera is aligned to?
	vec3 u, v, w;
	float lensRadius;
};