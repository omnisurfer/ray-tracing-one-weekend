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

	Hitable *left;
	Hitable *right;
	AABB box;
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
		left = right = l[0];
	}
	else if (n == 2) {
		left = l[0];
		right = l[1];
	}
	else {
		left = new BvhNode(l, n / 2, time0, time1);
		right = new BvhNode(l + n / 2, n - n / 2, time0, time1);
	}

	AABB boxLeft, boxRight;

	if (!left->boundingBox(time0, time1, boxLeft) || !right->boundingBox(time0, time1, boxRight)) {
		std::cout << "No boudning box in BvhNode constructor\n";
		box = surroundingBox(boxLeft, boxRight);
	}
}

bool BvhNode::boundingBox(float t0, float t1, AABB &b) const {
	b = box;
	return true;
}

bool BvhNode::hit(const ray &r, float tmin, float tmax, HitRecord &record) const {
	if (box.hit(r, tmin, tmax)) {
		HitRecord leftRecord, rightRecord;

		bool hitLeft = left->hit(r, tmin, tmax, leftRecord);
		bool hitRight = right->hit(r, tmin, tmax, rightRecord);

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