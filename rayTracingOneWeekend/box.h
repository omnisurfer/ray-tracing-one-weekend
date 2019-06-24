#pragma once

#include "hitable.h"
#include "hitableList.h"
#include "debug.h"
#include "mathUtilities.h"

#include "xy_rect.h"

class Box : public Hitable {
public:
	Box() {}
	Box(const vec3 &p0, const vec3 &p1, Material *materialPointer);

	virtual bool hit(const ray &r, float t0, float t1, HitRecord &hitRecord) const;
	virtual bool boundingBox(float t0, float t1, AABB &box) const {
		box = AABB(_pMin, _pMax);
		return true;
	}

	vec3 _pMin, _pMax;
	Hitable *_hitableList;
};

Box::Box(const vec3 &p0, const vec3 &p1, Material *materialPointer) {
	_pMin = p0;
	_pMax = p1;
	int sides = 6;
	Hitable **list = new Hitable*[sides];

	list[0] = new FlipNormals(new XYRectangle(p0.x(), p1.x(), p0.y(), p1.y(), p1.z(), materialPointer));
	list[1] = new XYRectangle(p0.x(), p1.x(), p0.y(), p1.y(), p0.z(), materialPointer);
	
	list[2] = new XZRectangle(p0.x(), p1.x(), p0.z(), p1.z(), p1.y(), materialPointer);
	list[3] = new FlipNormals(new XZRectangle(p0.x(), p1.x(), p0.z(), p1.z(), p0.y(), materialPointer));

	list[4] = new YZRectangle(p0.y(), p1.y(), p0.z(), p1.z(), p1.x(), materialPointer);
	list[5] = new FlipNormals(new YZRectangle(p0.y(), p1.y(), p0.z(), p1.z(), p0.x(), materialPointer));
	
	_hitableList = new HitableList(list, sides);
}

bool Box::hit(const ray &ray, float t0, float t1, HitRecord &hitRecord) const {
	return _hitableList->hit(ray, t0, t1, hitRecord);
}