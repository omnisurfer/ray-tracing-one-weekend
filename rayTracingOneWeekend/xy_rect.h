#pragma once

#include "hitable.h"
#include "debug.h"
#include "mathUtilities.h"

class XYRectangle : public Hitable {
public:
	XYRectangle() {}
	XYRectangle(float x0, float x1, float y0, float y1, float k, Material *material) : _x0(x0), _x1(x1), _y0(y0), _y1(y1), _k(k), _material(material) {};

	virtual bool hit(const ray &inputRay, float t0, float t1, HitRecord &hitRecord) const;
	virtual bool boundingBox(float t0, float t1, AABB &box) const {
		box = AABB(vec3(_x0, _y0, _k - 0.0001), vec3(_x1, _y1, _k + 0.0001));
		return true;
	}

	Material *_material;
	float _x0, _x1, _y0, _y1, _k;
};

bool XYRectangle::hit(const ray &inputRay, float t0, float t1, HitRecord &hitRecrod) const {
	float t = (_k - inputRay.origin().z()) / inputRay.direction().z();
	if (t < t0 || t > t1) {
		return false;
	}
	float x = inputRay.origin().x() + t * inputRay.direction().x();
	float y = inputRay.origin().y() + t * inputRay.direction().y();
	if (x < _x0 || x > _x1 || y < _y0 || y > _y1) {
		return false;
	}

	hitRecrod.u = (x - _x0) / (_x1 - _x0);
	hitRecrod.v = (y - _y0) / (_y1 - _y0);
	hitRecrod.pointAtParameterT = t;
	hitRecrod.materialPointer = _material;
	hitRecrod.point = inputRay.pointAtParameter(t);
	hitRecrod.normal = vec3(0, 0, 1);
	return true;
}

class XZRectangle : public Hitable {
public:
	XZRectangle() {}
	XZRectangle(float x0, float x1, float z0, float z1, float k, Material *material) : _x0(x0), _x1(x1), _z0(z0), _z1(z1), _k(k), _material(material) {}

	virtual bool hit(const ray &inputRay, float t0, float t1, HitRecord &hitRecord) const;
	virtual bool boundingBox(float t0, float t1, AABB &box) const {
		box = AABB(vec3(_x0, _k - 0.0001, _z0), vec3(_x1, _k + 0.0001, _z1));
		return true;
	}

	Material *_material;
	float _x0, _x1, _z0, _z1, _k;
};

bool XZRectangle::hit(const ray &inputRay, float t0, float t1, HitRecord &hitRecrod) const {
	float t = (_k - inputRay.origin().y()) / inputRay.direction().y();
	if (t < t0 || t > t1) {
		return false;
	}
	float x = inputRay.origin().x() + t * inputRay.direction().x();
	float z = inputRay.origin().z() + t * inputRay.direction().z();
	if (x < _x0 || x > _x1 || z < _z0 || z > _z1) {
		return false;
	}

	hitRecrod.u = (x - _x0) / (_x1 - _x0);
	hitRecrod.v = (z - _z0) / (_z1 - _z0);
	hitRecrod.pointAtParameterT = t;
	hitRecrod.materialPointer = _material;
	hitRecrod.point = inputRay.pointAtParameter(t);
	hitRecrod.normal = vec3(0, 1, 0);
	return true;
}

class YZRectangle : public Hitable {
public:
	YZRectangle() {}
	YZRectangle(float y0, float y1, float z0, float z1, float k, Material *material) : _y0(y0), _y1(y1), _z0(z0), _z1(z1), _k(k), _material(material) {}

	virtual bool hit(const ray &inputRay, float t0, float t1, HitRecord &hitRecord) const;
	virtual bool boundingBox(float t0, float t1, AABB &box) const {
		box = AABB(vec3(_k - 0.0001, _y0, _z0), vec3(_k + 0.0001, _y1, _z1));
		return true;
	}

	Material *_material;
	float _y0, _y1, _z0, _z1, _k;
};

bool YZRectangle::hit(const ray &inputRay, float t0, float t1, HitRecord &hitRecrod) const {
	float t = (_k - inputRay.origin().x()) / inputRay.direction().x();
	if (t < t0 || t > t1) {
		return false;
	}
	float y = inputRay.origin().y() + t * inputRay.direction().y();
	float z = inputRay.origin().z() + t * inputRay.direction().z();
	if (y < _y0 || y > _y1 || z < _z0 || z > _z1) {
		return false;
	}

	hitRecrod.u = (y - _y0) / (_y1 - _y0);
	hitRecrod.v = (z - _z0) / (_z1 - _z0);
	hitRecrod.pointAtParameterT = t;
	hitRecrod.materialPointer = _material;
	hitRecrod.point = inputRay.pointAtParameter(t);
	hitRecrod.normal = vec3(1, 0, 0);
	return true;
}