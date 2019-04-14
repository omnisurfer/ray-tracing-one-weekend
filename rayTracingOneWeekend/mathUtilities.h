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

inline float trilinearInterp(float c[2][2][2], float u, float v, float w) {
	float accum = 0;
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			for (int k = 0; k < 2; k++) {
				accum += (i*u + (1 - i)*(1 - u)) *
					(j*v + (1 - j)*(1 - v)) *
					(k*w + (1 - k)*(1 - w))*c[i][j][k];
			}
		}
	}
	return accum;
}

inline float perlinInterp(vec3 c[2][2][2], float u, float v, float w) {
	float uu = u * u*(3 - 2 * u);
	float vv = v * v*(3 - 2 * v);
	float ww = w * w*(3 - 2 * w);
	float accum = 0;

	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			for (int k = 0; k < 2; k++) {
				vec3 weightV(u - i, v - j, w - k);
				accum +=
						(i*uu + (1 - i)*(1 - uu)) *
						(j*vv + (1 - j)*(1 - vv)) *
						(k*ww + (1 - k)*(1 - ww)) * dot(c[i][j][k], weightV);
					
			}
		}
	}
	return accum;
}