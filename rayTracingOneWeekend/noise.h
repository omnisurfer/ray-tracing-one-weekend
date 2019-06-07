#pragma once

#include <math.h>

#include "vec3.h"
#include "rngs.h"
#include "mathUtilities.h"

class Perlin {
public:
	float noise(const vec3& p, bool filter) const {
		float u = p.x() - floor(p.x());
		float v = p.y() - floor(p.y());
		float w = p.z() - floor(p.z());

		int i = floor(p.x());
		int j = floor(p.y());
		int k = floor(p.z());
		
		if (filter) {
			vec3 c[2][2][2];			
			for (int di = 0; di < 2; di++) {
				for (int dj = 0; dj < 2; dj++) {
					for (int dk = 0; dk < 2; dk++) {
						c[di][dj][dk] = randVector[permX[(i + di) & 255] ^ permY[(j + dj) & 255] ^ permZ[(k + dk) & 255]];
					}
				}
			}
			return perlinInterp(c, u, v, w);
			//return trilinearInterp(c, u, v, w);
		}			
		else {
			return randFloat[permX[i] ^ permY[j] ^ permZ[k]];
		}

	}

	float turbulance(const vec3 &p, int depth = 7) const {
		float accum = 0;
		vec3 tempP = p;
		float weight = 1.0;

		for (int i = 0; i < depth; i++) {
			accum += weight * noise(tempP, true);
			weight *= 0.5;
			tempP *= 2;
		}
		return fabs(accum);
	}

	static float *randFloat;
	static vec3 *randVector;
	static int *permX;
	static int *permY;
	static int *permZ;
};

static float *perlinGenerate() {
	float *p = new float[256];
	for (int i = 0; i < 256; ++i) {
		p[i] = unifRand(randomNumberGenerator);
	}
	return p;
}

static vec3 *perlinGenerateVector() {
	vec3 p[256];
	for (int i = 0; i < 256; ++i) {

		double a, b, c;
		a = unifRand(randomNumberGenerator);
		b = unifRand(randomNumberGenerator);
		c = unifRand(randomNumberGenerator);

		//std::cout << "a, b c: " << a << " " << b << " " << c << " \n";

		p[i] = unit_vector(
			vec3(-1 + 2 * a, 
				-1 + 2 * b, 
				-1 + 2 * c
			)
		);		
	}

	vec3 *x = new vec3[256];

	for (int i = 0; i < 256; i++)
		x[i] = p[i];

	return x;
}

void permute(int *p, int n) {
	for (int i = n - 1; i > 0; i--) {
		int target = int(unifRand(randomNumberGenerator)*(i + 1));
		int tmp = p[i];

		p[i] = p[target];
		p[target] = tmp;
	}
}

static int *perlinGeneratePerm() {
	int *p = new int[256];
	for (int i = 0; i < 256; i++) {
		p[i] = i;		
	}
	permute(p,256);
	return p;
}

float *Perlin::randFloat = perlinGenerate();
vec3 *Perlin::randVector = perlinGenerateVector();
int *Perlin::permX = perlinGeneratePerm();
int *Perlin::permY = perlinGeneratePerm();
int *Perlin::permZ = perlinGeneratePerm();
