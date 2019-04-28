#pragma once

#include "hitable.h"
#include "debug.h"
#include "mathUtilities.h"

class Sphere : public Hitable {
public:
	Sphere() {}
	Sphere(vec3 center_, float radius_, Material *material_) : center(center_), radius(radius_), materialPointer(material_) {};

	virtual bool hit(const ray &rayCast, float minPointAtParameterT, float maxPointAtParamterT, HitRecord &hitRecord) const;
	virtual bool boundingBox(float t0, float t1, AABB &box) const;

	vec3 center;
	float radius;
	Material *materialPointer;
};

bool Sphere::hit(const ray &rayCast, float minPointAtParameterT, float maxPointAtParamterT, HitRecord &hitRecord) const {	

	//figure out where the sphere is in relation to the origin of the rayCast
	//maybe this is more about making sure the rayCast is outside the object being tested for intersection and not 
	//correcting the origin? OR maybe this is more about making sure you can compute a dot product since the center of the sphere is
	//(maybe) assumed to be a ray from 0,0 to the center? The rays don't have a common origin depending upon where the rayCast is?
	vec3 originCorrection = rayCast.origin() - center;

	//figure out values used in quadratic equation which determine if the ray hit the sphere
	float a = dot(rayCast.direction(), rayCast.direction());
	float b = dot(rayCast.direction(), originCorrection);
	float c = dot(originCorrection, originCorrection) - radius * radius;

	/*
		a * x^2			+ b*x				+ c						= 0
		dot(B,B) * t^2	+ 2.0*dot(B,A-C)*t	+ dot(A-C, A-C) - R^2	= 0
	*/

	//float discriminant = b * b - 4 * a*c;
	float discriminant = b*b - a*c;

	//hit the sphere, only using at least one real solution
	if (discriminant > 0) {

		float temp = (-b - sqrt(b*b - a * c)) / a;
		if (temp < maxPointAtParamterT && temp > minPointAtParameterT) {
			hitRecord.pointAtParameterT = temp;
			hitRecord.point = rayCast.pointAtParameter(hitRecord.pointAtParameterT);
						
			get_sphere_uv((hitRecord.point - center)/radius, hitRecord.u, hitRecord.v);

			hitRecord.normal = (hitRecord.point - center) / radius;	
			hitRecord.materialPointer = materialPointer;
			return true;
		}
		temp = (-b + sqrt(b*b - a * c)) / a;
		if (temp < maxPointAtParamterT && temp > minPointAtParameterT) {
			hitRecord.pointAtParameterT = temp;
			hitRecord.point = rayCast.pointAtParameter(hitRecord.pointAtParameterT);

			get_sphere_uv((hitRecord.point - center)/radius, hitRecord.u, hitRecord.v);

			hitRecord.normal = (hitRecord.point - center) / radius;
			hitRecord.materialPointer = materialPointer;
			return true;
		}
	}

	return false;
}

bool Sphere::boundingBox(float t0, float t1, AABB &box) const {
	box = AABB(center - vec3(radius, radius, radius), center + vec3(radius, radius, radius));
	return true;
}

class MovingSphere : public Hitable {
public:
	MovingSphere() {}
	MovingSphere(vec3 center0, vec3 center1, float t0, float t1, float r, Material *m) :
		_center0(center0), _center1(center1), _time0(t0), _time1(t1), _radius(r), _materialPointer(m) {};

	virtual bool hit(const ray& r, float tmin, float tmax, HitRecord& record) const;
	virtual bool boundingBox(float t0, float t1, AABB &box) const;

	vec3 center(float time) const;

	vec3 _center0, _center1;
	float _time0, _time1;
	float _radius;
	Material *_materialPointer;
};

bool MovingSphere::hit(const ray& r, float t_min, float t_max, HitRecord& record) const {
	vec3 oc = r.origin() - center(r.time());
	float a = dot(r.direction(), r.direction());
	float b = dot(oc, r.direction());
	float c = dot(oc, oc) - _radius * _radius;
	float discriminant = b * b - a * c;
	if (discriminant > 0) {
		float temp = (-b - sqrt(discriminant)) / a;
		if (temp < t_max && temp > t_min) {
			record.pointAtParameterT = temp;
			record.point = r.pointAtParameter(record.pointAtParameterT);
			record.normal = (record.point - center(r.time())) / _radius;
			record.materialPointer = _materialPointer;
			return true;
		}
		temp = (-b + sqrt(discriminant)) / a;
		if (temp < t_max && temp > t_min) {
			record.pointAtParameterT = temp;
			record.point = r.pointAtParameter(record.pointAtParameterT);
			record.normal = (record.point - center(r.time())) / _radius;
			record.materialPointer = _materialPointer;
			return true;
		}
	}
	return false;
}

bool MovingSphere::boundingBox(float t0, float t1, AABB &box) const {
	AABB boxT0 = AABB(center(_time0) - vec3(_radius, _radius, _radius), center(_time0) + vec3(_radius, _radius, _radius));
	AABB boxT1 = AABB(center(_time1) - vec3(_radius, _radius, _radius), center(_time1) + vec3(_radius, _radius, _radius));

	box = surroundingBox(boxT0, boxT1);

	return true;
}

vec3 MovingSphere::center(float time) const {
	return _center0 + ((time - _time0) / (_time1 - _time0))*(_center1 - _center0);
}
