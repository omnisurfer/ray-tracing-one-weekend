#pragma once

#include <math.h>

#include "vec3.h"
#include "rngs.h"
#include "mathUtilities.h"

class Perlin {
public:
	float noise(const vec3& p, bool type) const {
		float u = p.x() - floor(p.x());
		float v = p.y() - floor(p.y());
		float w = p.z() - floor(p.z());
		int i = int(4 * p.x()) & 255;
		int j = int(4 * p.y()) & 255;
		int k = int(4 * p.z()) & 255;
		

		if(type)
			return ranfloat[permX[i] ^ permY[j] ^ permZ[k]];
		else {
			float c[2][2][2];
			for (int di = 0; di < 2; di++) {
				for (int dj = 0; dj < 2; dj++) {
					for (int dk = 0; dk < 2; dk++) {
						c[di][dj][dk] = ranfloat[permX[(i + di) & 255] ^ permY[(j + dj) & 255] ^ permZ[(k + dk) & 255]];
					}
				}
			}
			return TrilinearInterp(c, u, v, w);
		}

	}
	static float *ranfloat;
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

float *Perlin::ranfloat = perlinGenerate();
int *Perlin::permX = perlinGeneratePerm();
int *Perlin::permY = perlinGeneratePerm();
int *Perlin::permZ = perlinGeneratePerm();
