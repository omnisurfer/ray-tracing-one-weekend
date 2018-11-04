#pragma once

#include "hitable.h"

class Sphere : public Hitable {
public:
	Sphere() {}
	Sphere(vec3 center_, float radius_) : center(center_), radius(radius_) {};

	virtual bool hit(const ray &rayCast, float minPointAtParameterT, float maxPointAtParamterT, HitRecord &hitRecord) const;

	vec3 center;
	float radius;
};

bool Sphere::hit(const ray &rayCast, float minPointAtParameterT, float maxPointAtParamterT, HitRecord &hitRecord) const {	

	vec3 originCorrection = rayCast.origin() - center;
	float a = dot(rayCast.direction(), rayCast.direction());
	//float b = 2.0 * dot(originCorrection, rayCast.direction());	
	//float b = 2.0 * dot(rayCast.direction(), originCorrection);
	float b = dot(rayCast.direction(), originCorrection);
	float c = dot(originCorrection, originCorrection) - radius * radius;

	/*
		a * x^2			+ b*x				+ c						= 0
		dot(B,B) * t^2	+ 2.0*dot(B,A-C)*t	+ dot(A-C, A-C) - R^2	= 0
	*/

	//float discriminant = b * b - 4 * a*c;
	float discriminant = b*b - a*c;

	if (discriminant > 0) {
		float temp = (-b - sqrt(b*b - a * c)) / a;
		if (temp < maxPointAtParamterT && temp > minPointAtParameterT) {
			hitRecord.pointAtParameterT = temp;
			hitRecord.point = rayCast.point_at_parameter(hitRecord.pointAtParameterT);
			hitRecord.normal = (hitRecord.point - center) / radius;
			return true;
		}
		temp = (-b + sqrt(b*b - a * c)) / a;
		if (temp < maxPointAtParamterT && temp > minPointAtParameterT) {
			hitRecord.pointAtParameterT = temp;
			hitRecord.point = rayCast.point_at_parameter(hitRecord.pointAtParameterT);
			hitRecord.normal = (hitRecord.point - center) / radius;
			return true;
		}
	}

	return false;
}