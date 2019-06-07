#pragma once

#include "ray.h"
#include "aabb.h"
#include "mathUtilities.h"

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

class Translate : public Hitable {
public:
	Translate(Hitable *hitable, const vec3 &displacement) : hitablePointer(hitable), offset(displacement) {}
	virtual bool hit(const ray &r, float tMin, float tMax, HitRecord &hitRecord) const;
	virtual bool boundingBox(float t0, float t1, AABB &box) const;
	Hitable *hitablePointer;
	vec3 offset;
};

bool Translate::hit(const ray &r, float tMin, float tMax, HitRecord &hitRecord) const {
	ray movedRay(r.origin() - offset, r.direction(), r.time());
	if (hitablePointer->hit(movedRay, tMin, tMax, hitRecord)) {
		hitRecord.point += offset;
		return true;
	}
	else {
		return false;
	}
}

bool Translate::boundingBox(float t0, float t1, AABB &box) const {
	if (hitablePointer->boundingBox(t0, t1, box)) {
		box = AABB(box.min() + offset, box.max() + offset);
		return true;
	}
	else {
		return false;
	}
}

class RotateY : public Hitable {
public:
	RotateY(Hitable *hitablePointer, float angle);
	virtual bool hit(const ray &r, float tMin, float tMax, HitRecord &hitRecord) const;
	virtual bool boundingBox(float t0, float t1, AABB &box) const {
		box = boundBox;
		return hasBox;
	}

	Hitable *pointer;
	float sinTheta;
	float cosTheta;
	bool hasBox;
	AABB boundBox;
};

RotateY::RotateY(Hitable *hitablePointer, float angle) : pointer(hitablePointer) {

	float radians = (M_PI / 180.0) * angle;
	sinTheta = sin(radians);
	cosTheta = cos(radians);

	hasBox = pointer->boundingBox(0, 1, boundBox);
	vec3 min(FLT_MAX, FLT_MAX, FLT_MAX);
	vec3 max(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			for (int k = 0; k < 2; k++) {
				float x = i * boundBox.max().x() + (1 - i)*boundBox.min().x();
				float y = j * boundBox.max().y() + (1 - j)*boundBox.min().y();
				float z = k * boundBox.max().z() + (1 - k)*boundBox.min().z();

				float newX = cosTheta * x + sinTheta * z;
				float newZ = -sinTheta * x + cosTheta * z;

				vec3 tester(newX, y, newZ);
				for (int c = 0; c < 3; c++) {
					if (tester[c] > max[c]) {
						max[c] = tester[c];
					}
					if (tester[c] < min[c]) {
						min[c] = tester[c];
					}
				}
			}
		}
	}
	boundBox = AABB(min, max);
}

bool RotateY::hit(const ray &r, float tMin, float tMax, HitRecord &hitRecord) const {
	vec3 origin = r.origin();
	vec3 direction = r.direction();
	
	origin[0] = cosTheta * r.origin()[0] - sinTheta * r.origin()[2];
	origin[2] = sinTheta * r.origin()[0] + cosTheta * r.origin()[2];

	direction[0] = cosTheta * r.direction()[0] - sinTheta * r.direction()[2];
	direction[2] = sinTheta * r.direction()[0] + cosTheta * r.direction()[2];

	ray rotatedRay(origin, direction, r.time());

	if (pointer->hit(rotatedRay, tMin, tMax, hitRecord)) {
		vec3 p = hitRecord.point;
		vec3 normal = hitRecord.normal;

		p[0] = cosTheta * hitRecord.point[0] + sinTheta * hitRecord.point[2];
		p[2] = -sinTheta * hitRecord.point[0] + cosTheta * hitRecord.point[2];

		normal[0] = cosTheta * hitRecord.normal[0] + sinTheta * hitRecord.normal[2];
		normal[2] = -sinTheta * hitRecord.normal[0] + cosTheta * hitRecord.normal[2];

		hitRecord.point = p;
		hitRecord.normal = normal;
		return true;
	}
	else {
		return false;
	}
}
