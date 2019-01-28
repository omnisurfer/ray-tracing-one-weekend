#pragma once

#include "hitable.h"

class HitableList : public Hitable {

public:
	HitableList() {}
	///HitableList(Hitable **hitableList_, uint32_t listSize_) { hitableList = hitableList_; listSize = listSize_; }
	HitableList(const std::list<std::shared_ptr<Hitable>> &listOfHitables) { _listOfHitables = listOfHitables; }

	virtual bool hit(const ray &rayCast, float minPointAtParameterT, float maxPointAtParmeterT, HitRecord &hitRecord) const;

	//Hitable **hitableList;
	//uint32_t listSize;

	std::list<std::shared_ptr<Hitable>> _listOfHitables;
};

//drowan 20190127: repalced with a version that uses std::list and std::shared_ptr
//bool HitableList::hit(const ray &rayCast, float minPointAtParameterT, float maxPointAtParmeterT, HitRecord &hitRecord) const {
//	
//	HitRecord tempHitRecord;
//	bool hitAnything = false;
//	double closestHitSoFar = maxPointAtParmeterT;	
//
//	for (uint32_t i = 0; i < listSize; i++) {
//		//this calls the hit method of each sphere or hittable in the hittableList populated in main.
//		//It looks like it is a recursive call but it is not.
//		//basically this can be interpreted as asking each object in the list if ray intercepts it
//		if (hitableList[i]->hit(rayCast, minPointAtParameterT, closestHitSoFar, tempHitRecord)) {
//			hitAnything = true;
//			closestHitSoFar = tempHitRecord.pointAtParameterT;
//			hitRecord = tempHitRecord;
//		}
//	}
//
//	return hitAnything;
//}

bool HitableList::hit(const ray &rayCast, float minPointAtParameterT, float maxPointAtParmeterT, HitRecord &hitRecord) const {

	HitRecord tempHitRecord;
	bool hitAnything = false;
	double closestHitSoFar = maxPointAtParmeterT;

	std::list<std::shared_ptr<Hitable>>::const_iterator listIterator = _listOfHitables.begin();

	while (listIterator != _listOfHitables.end()) {
		if ((*listIterator)->hit(rayCast, minPointAtParameterT, closestHitSoFar, tempHitRecord)) {
			hitAnything = true;
			closestHitSoFar = tempHitRecord.pointAtParameterT;
			hitRecord = tempHitRecord;
		}
		listIterator++;
	}

	return hitAnything;
}