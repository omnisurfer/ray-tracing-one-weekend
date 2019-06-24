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
	ConstantTexture(vec3 c) : _color(c) {}

	virtual vec3 value(float u, float v, const vec3 &p) const { 
		return _color;
	}

	vec3 _color;
};

class CheckerTexture : public Texture {
public:
	CheckerTexture() {}
	CheckerTexture(Texture *t0, Texture *t1) : _even(t0), _odd(t1) { }
	virtual vec3 value(float u, float v, const vec3 &p) const {
		float sines = sin(10 * p.x())*sin(10 * p.y())*sin(10 * p.z());
		if (sines < 0) {
			return _odd->value(u, v, p);
		}
		else {
			return _even->value(u, v, p);
		}
	}

	Texture *_odd;
	Texture *_even;
};

class NoiseTexture : public Texture {
public:
	NoiseTexture(bool enable, float scale) : _filter(enable), _scaled(scale) {}
	virtual vec3 value(float u, float v, const vec3& p) const {		
		//return vec3(1, 1, 1)*perlin.noise(p * scaled, filter);
		//return vec3(1, 1, 1)*perlin.turbulance(scaled * p);
		return vec3(1, 1, 1)*0.5*(1 + sin(_scaled*p.z() + 10 * _perlin.turbulance(p)));
	}

	float _scaled;
	bool _filter;
	Perlin _perlin;	
};

class ImageTexture : public Texture {
public:
	ImageTexture() {}
	ImageTexture(unsigned char *pixels, int A, int B) : _data(pixels), _nx(A), _ny(B) { }
	
	virtual vec3 value(float u, float v, const vec3 &p) const;	
	unsigned char *_data;
	int _nx, _ny;
};

vec3 ImageTexture::value(float u, float v, const vec3 &p) const {
	int i = (u)*_nx;
	int j = (1 - v)*_ny - 0.001;

	if (i < 0) i = 0;
	if (j < 0) j = 0;
	if (i > _nx - 1) i = _nx - 1;
	if (j > _ny - 1) j = _ny - 1;

	float r = int(_data[3 * i + 3 * _nx*j]) / 255.0;
	float g = int(_data[3 * i + 3 * _nx*j + 1]) / 255.0;
	float b = int(_data[3 * i + 3 * _nx*j + 2]) / 255.0;

	return vec3(r, g, b);
}