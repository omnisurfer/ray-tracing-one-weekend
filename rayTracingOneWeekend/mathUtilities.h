#pragma once

#if defined (PLATFORM_WIN) && PLATFORM_WIN == 1
#define _USE_MATH_DEFINES
#endif
#include <math.h>

#include "vec3.h"
#include "rngs.h"

vec3 randomInUnitDisk() {
	vec3 p;
	do {
		p = 2.0 * vec3(unifRand(randomNumberGenerator), unifRand(randomNumberGenerator), 0) - vec3(1, 1, 0);
	} while (dot(p, p) >= 1.0);
	return p;
}

vec3 randomInUnitSphere() {
	vec3 point;
	do {
		point = 2.0*vec3(unifRand(randomNumberGenerator), unifRand(randomNumberGenerator), unifRand(randomNumberGenerator)) - vec3(1, 1, 1);
	} while (point.squared_length() >= 1.0);
	return point;
}