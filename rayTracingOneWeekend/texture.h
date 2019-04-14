#pragma once

#include "vec3.h"
#include "noise.h"

class Texture {
public:
	virtual vec3 value(float u, float v, const vec3 &p) const = 0;
};

class ConstantTexture : public Texture {
public:
	ConstantTexture() {}
	ConstantTexture(vec3 c) : color(c) {}

	virtual vec3 value(float u, float v, const vec3 &p) const { 
		return color;
	}

	vec3 color;
};

class CheckerTexture : public Texture {
public:
	CheckerTexture() {}
	CheckerTexture(Texture *t0, Texture *t1) : even(t0), odd(t1) { }
	virtual vec3 value(float u, float v, const vec3 &p) const {
		float sines = sin(10 * p.x())*sin(10 * p.y())*sin(10 * p.z());
		if (sines < 0) {
			return odd->value(u, v, p);
		}
		else {
			return even->value(u, v, p);
		}
	}

	Texture *odd;
	Texture *even;
};

class NoiseTexture : public Texture {
public:
	NoiseTexture(bool enable, float scale) : filter(enable), scaled(scale) {}
	virtual vec3 value(float u, float v, const vec3& p) const {		
		//return vec3(1, 1, 1)*perlin.noise(p * scaled, filter);
		return vec3(1, 1, 1)*perlin.turbulance(scaled * p);
	}

	float scaled;
	bool filter;
	Perlin perlin;	
};