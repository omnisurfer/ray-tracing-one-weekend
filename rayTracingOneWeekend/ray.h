#pragma once
#ifndef RAY_H
#define RAY_H

class ray {
public:
	ray() {}
	ray(const vec3& a, const vec3& b) { A = a; B = b; }
	
	vec3 origin() const { return A; }
	vec3 direction() const { return B; }
	vec3 pointAtParameter(float t) const { return A + t*B; }

	vec3 A;
	vec3 B;
};
#endif
