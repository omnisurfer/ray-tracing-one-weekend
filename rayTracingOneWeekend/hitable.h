#pragma once

#include "ray.h"

struct HitRecord {
	float pointAtParameterT;
	vec3 point;
	vec3 normal;
};

class Hitable {
public:
	virtual bool hit(const ray &rayCast, float minPointAtParameterT, float maxPointAtParmeterT, HitRecord &hitRecord) const = 0;
};