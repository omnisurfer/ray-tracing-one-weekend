#pragma once

#include "ray.h"

class material;

struct HitRecord {
	float pointAtParameterT;
	vec3 point;
	vec3 normal;
	material *materialPointer;
};

class Hitable {
public:
	virtual bool hit(const ray &rayCast, float minPointAtParameterT, float maxPointAtParmeterT, HitRecord &hitRecord) const = 0;
};