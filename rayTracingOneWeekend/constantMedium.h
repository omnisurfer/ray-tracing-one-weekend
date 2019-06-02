#pragma once

#include "hitable.h"
#include "hitableList.h"
#include "texture.h"
#include "material.h"
#include "debug.h"
#include "mathUtilities.h"

class ConstantMedium : public Hitable {
public:
	ConstantMedium(Hitable *boundary, float density, Texture *texture) : _boundary(boundary), _density(density) { 
		_phaseFunction = new Isotropic(texture);
	}

	virtual bool hit(const ray &inputRay, float tMin, float tMax, HitRecord &hitRecord) const;
	virtual bool boundingBox(float t0, float t1, AABB &boundingBox) const {
		return _boundary->boundingBox(t0, t1, boundingBox);
	}

	Hitable *_boundary;
	float _density;
	Material *_phaseFunction;
};

bool ConstantMedium::hit(const ray &inputRay, float tMin, float tMax, HitRecord &hitRecord) const {
	bool db = (unifRand(randomNumberGenerator) < 0.00001);
	db = false;

	HitRecord hitRecordAlpha, hitRecordBeta;

	if (_boundary->hit(inputRay, -FLT_MAX, FLT_MAX, hitRecordAlpha)) {
		if (_boundary->hit(inputRay, hitRecordAlpha.pointAtParameterT + 0.0001, FLT_MAX, hitRecordBeta)) {
			if (db) std::cerr << "\nt0 t1 " << hitRecordAlpha.pointAtParameterT << " " << hitRecordBeta.pointAtParameterT << "\n";

			if (hitRecordAlpha.pointAtParameterT < tMin) {
				hitRecordAlpha.pointAtParameterT = tMin;
			}
			if (hitRecordBeta.pointAtParameterT > tMax) {
				hitRecordBeta.pointAtParameterT = tMax;
			}
			if (hitRecordAlpha.pointAtParameterT >= hitRecordBeta.pointAtParameterT) {
				return false;
			}
			if (hitRecordAlpha.pointAtParameterT < 0) {
				hitRecordAlpha.pointAtParameterT = 0;
			}

			float distanceInsideBoundary = (hitRecordBeta.pointAtParameterT - hitRecordAlpha.pointAtParameterT) * inputRay.direction().length();
			float hitDistance = -(1 / _density)*log(unifRand(randomNumberGenerator));

			if (hitDistance < distanceInsideBoundary) {
				if (db) std::cerr << "hitDistance = " << hitDistance << "\n";
			
				hitRecord.pointAtParameterT = hitRecordAlpha.pointAtParameterT + hitDistance / inputRay.direction().length();

				if (db) {
					std::cerr << "hitRecord.pointAtParameterT = " << hitRecord.pointAtParameterT << "\n";
				}

				hitRecord.point = inputRay.pointAtParameter(hitRecord.pointAtParameterT);

				if (db) {
					std::cerr << "hitRecord.point = " << hitRecord.point << "\n";
				}

				hitRecord.normal = vec3(1, 0, 0); //arbitary choice?
				hitRecord.materialPointer = _phaseFunction;
				return true;
			}
		}
	}
	return false;
}