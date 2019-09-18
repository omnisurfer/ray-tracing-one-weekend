#pragma once

#include <iostream>
#include <stdlib.h>

#include "vec3.h"
#include "ray.h"
#include "hitable.h"
#include "material.h"

//Color is called recursively!
vec3 color(const ray &rayCast, Hitable *world, int depth) {	
	//provide a way to store the hit vector to act on it outside the hit check
	HitRecord hitRecord;

	//hits a point on the sphere or hittable.
	float maxFloat = std::numeric_limits<float>::max();	

	if (world->hit(rayCast, 0.001, maxFloat, hitRecord)) {
		ray scattered;
		vec3 attenuation;		
		vec3 emitted = hitRecord.materialPointer->emitted(hitRecord.u, hitRecord.v, hitRecord.point);

		//depth refers to number of recursive calls to bounce the ray around???
		if (depth < DEPTH_RECURSION && hitRecord.materialPointer->scatter(rayCast, hitRecord, attenuation, scattered)) {
			return emitted + attenuation * color(scattered, world, depth + 1);
		}
		else {
			return emitted;
		}
	}
	//does not hit anything, so "background" gradient
	else {
#if GLOBAL_ILLUM_EN == 1
		if (depth < 1) {
			// maybe this becomes like a sky box or global illumination???
			vec3 unit_direction = unit_vector(rayCast.direction());

			float tempPointAtParameterT = 0.5*(unit_direction.y() + 1.0);

			//Need to see how this works again but it seems that the first parameters color the "skybox" (things far away??) while the
			//second paramater colors closer things
			//as the pointAtParameter becomes larger (i.e. hit something close) it attenuates the first portion of the equation and
			//amplifies the second portion.
			return (1.0 - tempPointAtParameterT)*vec3(unit_direction.y() * 1.0, unit_direction.y() * 1.0, 1.0) + tempPointAtParameterT * vec3(GLOBAL_ILLUM_GAIN, GLOBAL_ILLUM_GAIN, GLOBAL_ILLUM_GAIN);
		}		
		else {
			return vec3(0.8, 0.8, 0.8);
		}
#else
		return vec3(0.0, 0.0, 0.0);
#endif
	}
}