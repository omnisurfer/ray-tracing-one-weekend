#pragma once

#include "ray.h"
#include "aabb.h"

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
	virtual bool boundingBox(float t0, float t1, AABB &box) const = 0;
};