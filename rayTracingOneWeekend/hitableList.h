#pragma once

#include "hitable.h"

AABB surroundingBox(AABB box0, AABB box1);

class HitableList : public Hitable {

public:
	HitableList() {}
	HitableList(Hitable **hitableList_, uint32_t listSize_) { hitableList = hitableList_; listSize = listSize_; }	

	virtual bool hit(const ray &rayCast, float minPointAtParameterT, float maxPointAtParmeterT, HitRecord &hitRecord) const;
	virtual bool boundingBox(float t0, float t1, AABB &box) const;

	Hitable **hitableList;
	uint32_t listSize;	
};

bool HitableList::hit(const ray &rayCast, float minPointAtParameterT, float maxPointAtParmeterT, HitRecord &hitRecord) const {
	
	HitRecord tempHitRecord;
	bool hitAnything = false;
	double closestHitSoFar = maxPointAtParmeterT;	

	for (uint32_t i = 0; i < listSize; i++) {
		//this calls the hit method of each sphere or hittable in the hittableList populated in main.
		//It looks like it is a recursive call but it is not.
		//basically this can be interpreted as asking each object in the list if ray intercepts it
		if (hitableList[i]->hit(rayCast, minPointAtParameterT, closestHitSoFar, tempHitRecord)) {
			hitAnything = true;
			closestHitSoFar = tempHitRecord.pointAtParameterT;
			hitRecord = tempHitRecord;
		}
	}

	return hitAnything;
}

bool HitableList::boundingBox(float t0, float t1, AABB &box) const {
	if (listSize < 1) return false;
	
	AABB tempBox;

	bool firstTrue = hitableList[0]->boundingBox(t0, t1, tempBox);

	if (!firstTrue)
		return false;
	else
		box = tempBox;

	for (int i = 0; i < listSize; i++) {
		if (hitableList[0]->boundingBox(t0, t1, tempBox)) {
			box = surroundingBox(box, tempBox);
		}
		else
			return false;
	}
	return true;
}