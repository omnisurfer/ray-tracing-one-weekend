#pragma once

#include "ray.h"
#include "aabb.h"

class Material;

struct HitRecord {
	float pointAtParameterT;
	//drowan 20190414: not sure if u and v are ever used.
	float u;
	float v;
	vec3 point;
	vec3 normal;
	Material *materialPointer;
};

class Hitable {
public:
	virtual bool hit(const ray &rayCast, float minPointAtParameterT, float maxPointAtParmeterT, HitRecord &hitRecord) const = 0;
	virtual bool boundingBox(float t0, float t1, AABB &box) const = 0;
};

class FlipNormals : public Hitable {
public:
	FlipNormals(Hitable *hitable) : _hitable(hitable) {}
	virtual bool hit(const ray &inputRay, float t_min, float t_max, HitRecord &hitRecord) const {
		if (_hitable->hit(inputRay, t_min, t_max, hitRecord)) {
			hitRecord.normal = -hitRecord.normal;
			return true;
		}
		else {
			return false;
		}
	}
	virtual bool boundingBox(float t0, float t1, AABB &box) const {
		return _hitable->boundingBox(t0, t1, box);
	}

	Hitable *_hitable;
};