#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <vector>
#include <random>
#include <chrono>

#include "vec3.h"
#include "ray.h"
#include "sphere.h"
#include "hitableList.h"
#include "float.h"

#include "bitmap.h"

/* https://github.com/petershirley/raytracinginoneweekend
*/
//"screen" resolution
//4K 3840x2160, 2K 2560x1440
int32_t resWidth = 800, resHeight = 600;
int32_t resRatioWH = resWidth / resHeight;
uint8_t bytesPerPixel = (BITS_PER_PIXEL / BITS_PER_BYTE);
uint32_t antiAliasingSamples = 4;

//setup RNG
//https://stackoverflow.com/questions/9878965/rand-between-0-and-1

std::mt19937_64 randomNumberGenerator;
uint64_t timeSeed;
std::uniform_real_distribution<double> unifRand(0, 1.0);

vec3 random_in_unit_sphere() {
	vec3 point;
	do {
		point = 2.0*vec3(unifRand(randomNumberGenerator), unifRand(randomNumberGenerator), unifRand(randomNumberGenerator)) - vec3(1, 1, 1);
	} while (point.squared_length() >= 1.0);
	return point;
}

vec3 color(const ray &rayCast, Hitable *world) {

	HitRecord hitRecord;

	//hits a point on the sphere or hittable
	if (world->hit(rayCast, 0.001, std::numeric_limits<float>::max(), hitRecord)) {
		
		//produce a "reflection" ray that originates at the point where a hit was detected and is cast in some random direction away from the impact surface.
		vec3 target = hitRecord.point + hitRecord.normal + random_in_unit_sphere();

		return 0.5*color(ray(hitRecord.point, target - hitRecord.point), world);
	}
	//does not hit anything, so "background" gradient
	else {
		vec3 unit_direction = unit_vector(rayCast.direction());

		float tempPointAtParameterT = 0.5*(unit_direction.y() + 1.0);

		return (1.0 - tempPointAtParameterT)*vec3(1.0, 1.0, 1.0) + tempPointAtParameterT * vec3(0.5, 0.7, 1.0);
	}
}

int main() {	

	uint32_t tempImageBufferSizeInBytes = resWidth * resHeight * bytesPerPixel;

	uint8_t *tempImageBuffer = new uint8_t[tempImageBufferSizeInBytes]();

	for (int i = 0; i < tempImageBufferSizeInBytes - 1; i++) {
		tempImageBuffer[i] = 0x24;
		tempImageBuffer[tempImageBufferSizeInBytes - 1] = 0x24;
	}

	WINDIBBitmap winDIBBmp;

	std::cout << "Raytracing...\n";

	float x = (float_t)resWidth / (float_t)resWidth,
	y = (float_t)resHeight / (float_t)resWidth;

	vec3 lower_left_corner(-x, -y, -1.0);
	vec3 horizontal(2.0*x, 0.0, 0.0);
	vec3 vertical(0.0, 2.0*y, 0.0);
	vec3 origin(0.0, 0.0, 0.0);

	camera mainCamera(lower_left_corner, horizontal, vertical, origin);

	Hitable *hitableList[2];
	hitableList[0] = new Sphere(vec3(0, 0, -1), 0.5);
	hitableList[1] = new Sphere(vec3(0, -100.5, -1), 100);

	Hitable *world = new HitableList(hitableList, 2);
	
	timeSeed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	std::seed_seq seedSequence { 
			uint32_t(timeSeed & 0xffffffff), 
			uint32_t(timeSeed>>32) 
	};

	randomNumberGenerator.seed(seedSequence);	

	//main raytracing loops
	for (int row = resHeight - 1; row >= 0; row--) {

		
		//loop to move ray across width of frame
		for (int column = 0; column < resWidth; column++) {
			//loop to produce AA samples
			vec3 outputColor(0, 0, 0);
			for (int sample = 0; sample < antiAliasingSamples; sample++) { 
				float u = float(column + unifRand(randomNumberGenerator)) / float(resWidth);
				float v = float(row + +unifRand(randomNumberGenerator)) / float(resHeight);

				//A, the origin of the ray (camera)
				ray rayCast = mainCamera.getRay(u, v);				

				//NOTE: not sure about magic number 2.0 in relation with my tweaks to the viewport frame
				vec3 p = rayCast.point_at_parameter(2.0);
				outputColor += color(rayCast, world);
			}

			outputColor /= float(antiAliasingSamples);
			outputColor = vec3(sqrt(outputColor[0]), sqrt(outputColor[1]), sqrt(outputColor[2]));
			int ir = int(255.99 * outputColor[0]);
			int ig = int(255.99 * outputColor[1]);
			int ib = int(255.99 * outputColor[2]);			

			//also store values into tempBuffer
			tempImageBuffer[row*resWidth * bytesPerPixel + (column * bytesPerPixel)] = ib;
			tempImageBuffer[row*resWidth * bytesPerPixel + (column * bytesPerPixel) + 1] = ig;
			tempImageBuffer[row*resWidth * bytesPerPixel + (column * bytesPerPixel) + 2] = ir;
		}
	}

	std::cout << "Raytracing complete, pres any key to write to bmp.\n";
		
	//std::cin.get();

	std::cout << "Writing to debug bmp file...\n";

	winDIBBmp.writeBMPToFile(tempImageBuffer, tempImageBufferSizeInBytes, resWidth, resHeight, BITS_PER_PIXEL);

	delete[] tempImageBuffer;

	return 0;
}