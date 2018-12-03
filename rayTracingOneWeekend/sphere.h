#pragma once

#include "hitable.h"
#include "debug.h"

class Sphere : public Hitable {
public:
	Sphere() {}
	Sphere(vec3 center_, float radius_, material *material_) : center(center_), radius(radius_), materialPointer(material_) {};

	virtual bool hit(const ray &rayCast, float minPointAtParameterT, float maxPointAtParamterT, HitRecord &hitRecord) const;

	vec3 center;
	float radius;
	material *materialPointer;
};

bool Sphere::hit(const ray &rayCast, float minPointAtParameterT, float maxPointAtParamterT, HitRecord &hitRecord) const {	

	//figure out where the sphere is in relation to the origin of the rayCast
	vec3 originCorrection = rayCast.origin() - center;

	//figure out values use din quadratic equation which determine if the ray hit the sphere
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
			hitRecord.normal = (hitRecord.point - center) / radius;	
			hitRecord.materialPointer = materialPointer;
			return true;
		}
		temp = (-b + sqrt(b*b - a * c)) / a;
		if (temp < maxPointAtParamterT && temp > minPointAtParameterT) {
			hitRecord.pointAtParameterT = temp;
			hitRecord.point = rayCast.pointAtParameter(hitRecord.pointAtParameterT);
			hitRecord.normal = (hitRecord.point - center) / radius;
			hitRecord.materialPointer = materialPointer;
			return true;
		}
	}

	return false;
}