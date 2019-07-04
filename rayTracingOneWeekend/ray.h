#pragma once

class ray {
public:
	ray() {}
	ray(const vec3& a, const vec3& b, float ti = 0.0) { _a = a; _b = b; _time = ti; }
	
	vec3 origin() const { return _a; }
	vec3 direction() const { return _b; }
	float time() const { return _time; }
	vec3 pointAtParameter(float t) const { return _a + t*_b; }

	vec3 _a;
	vec3 _b;
	float _time;
};