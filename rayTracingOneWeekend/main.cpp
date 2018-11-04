#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <vector>

#include "vec3.h"
#include "ray.h"
#include "sphere.h"
#include "hitableList.h"
#include "float.h"

#include "bitmap.h"

/* https://github.com/petershirley/raytracinginoneweekend
*/
//"screen" resolution
//4K 3840x2160, 2560x1440
int32_t resWidth = 2560, resHeight = 1440;
int32_t resRatioWH = resWidth / resHeight;
uint8_t bytesPerPixel = (BITS_PER_PIXEL / BITS_PER_BYTE);

vec3 color(const ray &rayCast, Hitable *world) {

	HitRecord hitRecord;

	//hits a point on the sphere or hittable
	if (world->hit(rayCast, 0.0, std::numeric_limits<float>::max(), hitRecord)) {
		return 0.5*vec3(hitRecord.normal.x() * 1 + 1, hitRecord.normal.y() * 0, hitRecord.normal.z() * 0);
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

	Hitable *hitableList[2];
	hitableList[0] = new Sphere(vec3(0, 0, -1), 0.5);
	hitableList[1] = new Sphere(vec3(0, -100.5, -1), 100);

	Hitable *world = new HitableList(hitableList, 2);

	//main raytracing loops
	for (int row = resHeight - 1; row >= 0; row--) {

		for (int column = 0; column < resWidth; column++) {			
			float u = float(column) / float(resWidth);
			float v = float(row) / float(resHeight);

			//A, the origin of the ray (camera)
			ray rayCast(origin, lower_left_corner + u*horizontal + v*vertical);

			vec3 p = rayCast.point_at_parameter(2.0);
			vec3 outputColor = color(rayCast, world);

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