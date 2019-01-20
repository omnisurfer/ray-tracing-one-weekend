#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <vector>
#include <random>
#include <chrono>

#include "vec3.h"
#include "ray.h"
#include "sphere.h"
#include "material.h"
#include "hitableList.h"
#include "float.h"

#include "debug.h"

#include "bitmap.h"

/* TODO:
	drowan 20190120: More cleanly seperate and encapsulate functions and implement general OOP best practices. For now, just trying to get through the book.
*/

/* 
* https://github.com/petershirley/raytracinginoneweekend
* http://jamie-wong.com/2016/07/15/ray-marching-signed-distance-functions/
* http://www.codinglabs.net/article_world_view_projection_matrix.aspx
* http://iquilezles.org/index.html
*/
//"screen" resolution
//4K 3840x2160, 2K 2560x1440
int32_t resWidth = 640, resHeight = 480;
int32_t resRatioWH = resWidth / resHeight;
//TODO: Remove this direct dependancy on defines located in the BMP class
uint8_t bytesPerPixel = (BMP_BITS_PER_PIXEL / BMP_BITS_PER_BYTE);
uint32_t antiAliasingSamples = 16;

//Color is called recursively!
vec3 color(const ray &rayCast, Hitable *world, int depth) {	
	colorCallCount++;
	//provide a way to store the hit vector to act on it outside the hit check
	HitRecord hitRecord;

	//hits a point on the sphere or hittable.
	if (world->hit(rayCast, 0.001, std::numeric_limits<float>::max(), hitRecord)) {
		ray scattered;
		vec3 attenuation;

		//depth refers to number of recursive calls to bounce the ray around???
		if (depth < 50 && hitRecord.materialPointer->scatter(rayCast, hitRecord, attenuation, scattered)) {
			return attenuation * color(scattered, world, depth + 1);
		}
		else {
			//what does it mean when this returns?
			return vec3(0, 0, 0);
		}				
	}
	//does not hit anything, so "background" gradient
	else {
		vec3 unit_direction = unit_vector(rayCast.direction());

		float tempPointAtParameterT = 0.5*(unit_direction.y() + 1.0);

		return (1.0 - tempPointAtParameterT)*vec3(1.0, 1.0, 1.0) + tempPointAtParameterT * vec3(0.5, 0.7, 1.0);
	}
}

int main() {

	DEBUG_MSG_L0(__func__, "");

	uint32_t tempImageBufferSizeInBytes = resWidth * resHeight * bytesPerPixel;

	std::unique_ptr<uint8_t> tempImageBuffer(new uint8_t[tempImageBufferSizeInBytes]);		

	WINDIBBitmap winDIBBmp;

	std::cout << "Raytracing...\n";

	float x = (float_t)resWidth / (float_t)resWidth,
	y = (float_t)resHeight / (float_t)resWidth;

	vec3 lower_left_corner(-x, -y, -1.0);
	vec3 horizontal(2.0*x, 0.0, 0.0);
	vec3 vertical(0.0, 2.0*y, 0.0);
	vec3 origin(0.0, 0.0, 0.0);

	camera mainCamera(lower_left_corner, horizontal, vertical, origin);
	
	//replace with linked list, std::list<Hitable>?
	Hitable *hitableList[6];
	hitableList[0] = new Sphere(vec3(0, -100.5, -1), 100, new metal(vec3(0.1, 0.1, 0.1), 0.01));
	
	hitableList[1] = new Sphere(vec3(-0.5, 1, -1.5), 0.5, new metal(vec3(0.8, 0.1, 0.1), 0.3));	
	hitableList[2] = new Sphere(vec3(0.5, 1, -1.5), 0.5, new metal(vec3(0.1, 0.1, 0.8), 0.3));	
	hitableList[3] = new Sphere(vec3(0, 0, -1), -0.4, new dielectric(1.0));//lambertian(vec3(0.07, 0.25, 0.83)));

	//metal spheres
	hitableList[4] = new Sphere(vec3(1, 0, -1), 0.5, new metal(vec3(0.1, 0.8, 0.1), 0.1));
	hitableList[5] = new Sphere(vec3(-1, 0, -1), 0.5, new metal(vec3(0.8, 0.1, 1.0), 0.5));	
	
	Hitable *world = new HitableList(hitableList, 6);
	

	timeSeed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	std::seed_seq seedSequence { 
			uint32_t(timeSeed & 0xffffffff), 
			uint32_t(timeSeed>>32) 
	};

	//this is created in material.h
	randomNumberGenerator.seed(seedSequence);	

	//main raytracing loops, the movement across u and v "drive" the render (i.e. when to stop)
	for (int row = resHeight - 1; row >= 0; row--) {
		if(row%10 == 0 || row == resHeight - 1)
			std::cout << "\nRow: " << row << " ";

		int columnProgress = 0;
		//loop to move ray across width of frame
		for (int column = 0; column < resWidth; column++, columnProgress++) {
			if(columnProgress%1000 == 0 || columnProgress == 0)
				std::cout << ". ";

			//loop to produce AA samples
			vec3 outputColor(0, 0, 0);
			for (int sample = 0; sample < antiAliasingSamples; sample++) { 
				float u = float(column + unifRand(randomNumberGenerator)) / float(resWidth);
				float v = float(row + unifRand(randomNumberGenerator)) / float(resHeight);

				//A, the origin of the ray (camera)
				//rayCast stores a ray projected from the camera as it points into the scene that is swept across the uv "picture" frame.
				ray rayCast = mainCamera.getRay(u, v);				

				//NOTE: not sure about magic number 2.0 in relation with my tweaks to the viewport frame
				vec3 p = rayCast.pointAtParameter(2.0);
				outputColor += color(rayCast, world, 0);
			}

			outputColor /= float(antiAliasingSamples);
			outputColor = vec3(sqrt(outputColor[0]), sqrt(outputColor[1]), sqrt(outputColor[2]));
			int ir = int(255.99 * outputColor[0]);
			int ig = int(255.99 * outputColor[1]);
			int ib = int(255.99 * outputColor[2]);

			//also store values into tempBuffer
			tempImageBuffer.get()[row*resWidth * bytesPerPixel + (column * bytesPerPixel)] = ib;
			tempImageBuffer.get()[row*resWidth * bytesPerPixel + (column * bytesPerPixel) + 1] = ig;
			tempImageBuffer.get()[row*resWidth * bytesPerPixel + (column * bytesPerPixel) + 2] = ir;
		}
	}

	std::cout << "\nRaytracing complete, pres any key to write to bmp.\n";
		
	std::cin.get();

	std::cout << "Writing to debug bmp file...\n";

	winDIBBmp.writeBMPToFile(tempImageBuffer.get(), tempImageBufferSizeInBytes, resWidth, resHeight, BMP_BITS_PER_PIXEL);

	delete[] world;

	DEBUG_MSG_L0("colorCallCount: ", colorCallCount);

	return 0;
}