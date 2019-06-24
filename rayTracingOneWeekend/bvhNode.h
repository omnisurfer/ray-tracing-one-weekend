#pragma once

#include <iostream>

#include "hitable.h"
#include "rngs.h"

int boxXCompare(const void *a, const void *b);
int boxYCompare(const void *a, const void *b);
int boxZCompare(const void *a, const void *b);

class BvhNode : public Hitable {
public:
	BvhNode() {}
	BvhNode(Hitable **l, int n, float time0, float time1);

	virtual bool hit(const ray &r, float tmin, float tmax, HitRecord &record) const;
	virtual bool boundingBox(float t0, float t1, AABB &box) const;

	Hitable *_left;
	Hitable *_right;
	AABB _box;
};

BvhNode::BvhNode(Hitable **l, int n, float time0, float time1) {
	int axis = int(3 * unifRand(randomNumberGenerator));

	if (axis == 0) {
		qsort(l, n, sizeof(Hitable *), boxXCompare);
	}
	else if (axis == 1) {
		qsort(l, n, sizeof(Hitable *), boxYCompare);
	}
	else {
		qsort(l, n, sizeof(Hitable *), boxZCompare);
	}

	if (n == 1) {
		_left = _right = l[0];
	}
	else if (n == 2) {
		_left = l[0];
		_right = l[1];
	}
	else {
		//std::cout << __func__ << "left l: " << l << "\n";
		_left = new BvhNode(l, n / 2, time0, time1);
		//std::cout << __func__ << "right l + n/2: " << l + n / 2 << "\n";
		_right = new BvhNode(l + n / 2, n - n / 2, time0, time1);
	}

	AABB boxLeft, boxRight;

	if (!_left->boundingBox(time0, time1, boxLeft) || !_right->boundingBox(time0, time1, boxRight)) {
		std::cout << "No boudning box in BvhNode constructor\n";		
	}

	_box = surroundingBox(boxLeft, boxRight);
}

bool BvhNode::boundingBox(float t0, float t1, AABB &b) const {
	b = _box;
	return true;
}

bool BvhNode::hit(const ray &r, float tmin, float tmax, HitRecord &record) const {
	if (_box.hit(r, tmin, tmax)) {
		HitRecord leftRecord, rightRecord;

		bool hitLeft = _left->hit(r, tmin, tmax, leftRecord);
		bool hitRight = _right->hit(r, tmin, tmax, rightRecord);

		if (hitLeft && hitRight) {
			if (leftRecord.pointAtParameterT < rightRecord.pointAtParameterT)
				record = leftRecord;
			else
				record = rightRecord;

			return true;
		}
		else if (hitLeft) {
			record = leftRecord;
			return true;
		}
		else if (hitRight) {
			record = rightRecord;
			return true;
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}
}

int boxXCompare(const void *a, const void *b) {
	AABB boxLeft, boxRight;
	Hitable *ah = *(Hitable**)a;
	Hitable *bh = *(Hitable**)b;

	if (!ah->boundingBox(0, 0, boxLeft) || !bh->boundingBox(0, 0, boxRight)) {
		std::cerr << "No bounding box in BvhNode constructor\n";
	}
	if (boxLeft.min().x() - boxRight.min().x() < 0.0) {
		return -1;
	}
	else {
		return 1;
	}
}

int boxYCompare(const void *a, const void *b) {
	AABB boxLeft, boxRight;
	Hitable *ah = *(Hitable**)a;
	Hitable *bh = *(Hitable**)b;

	if (!ah->boundingBox(0, 0, boxLeft) || !bh->boundingBox(0, 0, boxRight)) {
		std::cerr << "No bounding box in BvhNode constructor\n";
	}
	if (boxLeft.min().y() - boxRight.min().y() < 0.0) {
		return -1;
	}
	else {
		return 1;
	}
}

int boxZCompare(const void *a, const void *b) {
	AABB boxLeft, boxRight;
	Hitable *ah = *(Hitable**)a;
	Hitable *bh = *(Hitable**)b;

	if (!ah->boundingBox(0, 0, boxLeft) || !bh->boundingBox(0, 0, boxRight)) {
		std::cerr << "No bounding box in BvhNode constructor\n";
	}
	if (boxLeft.min().z() - boxRight.min().z() < 0.0) {
		return -1;
	}
	else {
		return 1;
	}
}