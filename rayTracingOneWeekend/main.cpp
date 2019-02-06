#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <vector>
#include <random>
#include <chrono>
#include <list>

#include "defines.h"
#include "vec3.h"
#include "ray.h"
#include "sphere.h"
#include "material.h"
#include "hitableList.h"
#include "float.h"
#include "camera.h"
#include "color.h"

#include "debug.h"

#include "winDIBbitmap.h"

/* TODO:
	- drowan 20190120: More cleanly seperate and encapsulate functions and implement general OOP best practices. For now, just trying to get through the book.
	- drowan 20190121: I need to really refactor and clean up code style. unifRand is being pulled from material.
*/

/* 
* https://github.com/petershirley/raytracinginoneweekend
* http://jamie-wong.com/2016/07/15/ray-marching-signed-distance-functions/
* http://www.codinglabs.net/article_world_view_projection_matrix.aspx
* http://iquilezles.org/index.html
*/

Hitable *randomScene();

int main() {

	DEBUG_MSG_L0(__func__, "");

	//Setup random number generator
	timeSeed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	std::seed_seq seedSequence{
			uint32_t(timeSeed & 0xffffffff),
			uint32_t(timeSeed >> 32)
	};

	randomNumberGenerator.seed(seedSequence);

	//Setup screen and output image
	//4K 3840x2160, 2K 2560x1440
	WINDIBBitmap winDIBBmp;

	int32_t resWidth = 240, resHeight = 180;
	uint8_t bytesPerPixel = (winDIBBmp.getBitsPerPixel() / 8);
	uint32_t antiAliasingSamples = 20;

	uint32_t tempImageBufferSizeInBytes = resWidth * resHeight * bytesPerPixel;

	std::unique_ptr<uint8_t> tempImageBuffer(new uint8_t[tempImageBufferSizeInBytes]);				

	//Setup camera
	vec3 lookFrom(2, 3, -10);
	vec3 lookAt(0, 0, 0);
	vec3 worldUp(0, 1, 0);
	float distToFocus = (lookFrom - lookAt).length();
	float aperture = 2.0;
	float aspectRatio = float(resWidth) / float(resHeight);
	float vFoV = 40.0;

	Camera mainCamera(lookFrom, lookAt, worldUp, vFoV, aspectRatio, aperture, distToFocus, 0, 0.5);

	mainCamera.setLookAt(vec3(0, 0, 0));

	Hitable *hitableList[6];
	
	//This is the "ground" sphere
	hitableList[0] = new Sphere(vec3(0, -100.5, -1), 100, new metal(vec3(0.1, 0.1, 0.1), 0.01));
	
	//red sphere
	hitableList[1] = new Sphere(vec3(-1.0, 0.0, -1.0), 0.5, new metal(vec3(0.8, 0.1, 0.1), 0.3));
	//green sphere
	hitableList[2] = new Sphere(vec3(1.0, 0.0, -1.5), 0.5, new metal(vec3(0.1, 0.8, 0.1), 0.3));

	//blue sphere
	hitableList[3] = new Sphere(vec3(0, 1.5, -2.0), 0.5, new metal(vec3(0.1, 0.1, 0.8), 0.1));
	//mixed sphere
	hitableList[4] = new Sphere(vec3(0.0, 0.0, -2.0), 0.5, new metal(vec3(0.2, 0.4, 0.6), 0.5));	

	//Glass sphere
	hitableList[5] = new Sphere(vec3(0, 0, -1), 0.5, new dielectric(5.0));//lambertian(vec3(0.07, 0.25, 0.83)));
	
	//world bundles all the hitables and provides a generic way to call hit recursively in color (it's hit calls all the objects hits)
	Hitable *world = new HitableList(hitableList, 6);

	world = randomScene();
	
	std::cout << "Raytracing...\n";

	//main raytracing loops, the movement across u and v "drive" the render (i.e. when to stop)
	for (int row = resHeight - 1; row >= 0; row--) {
		if(row%10 == 0 || row == resHeight - 1)
			std::cout << "\nRow " << row << " ";

		int columnProgress = 0;
		//loop to move ray across width of frame
		for (int column = 0; column < resWidth; column++, columnProgress++) {
			if(columnProgress%1000 == 0 || columnProgress == 0)
				std::cout << ". ";
			
			vec3 outputColor(0, 0, 0);
			//loop to produce AA samples
			for (int sample = 0; sample < antiAliasingSamples; sample++) { 
				float u = float(column + unifRand(randomNumberGenerator)) / float(resWidth);
				float v = float(row + unifRand(randomNumberGenerator)) / float(resHeight);

				//A, the origin of the ray (camera)
				//rayCast stores a ray projected from the camera as it points into the scene that is swept across the uv "picture" frame.
				ray rayCast = mainCamera.getRay(u, v);				

				//NOTE: not sure about magic number 2.0 in relation with my tweaks to the viewport frame
				vec3 pointAt = rayCast.pointAtParameter(2.0);
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
		
	//std::cin.get();

	std::cout << "Writing to debug bmp file...\n";

	winDIBBmp.writeBMPToFile(tempImageBuffer.get(), tempImageBufferSizeInBytes, resWidth, resHeight, BMP_BITS_PER_PIXEL);

	delete[] world;

	DEBUG_MSG_L0("colorCallCount: ", colorCallCount);

	return 0;
}

Hitable *randomScene() {
	//drowan 20190127: changing n from 500 to 250 causes a crash..?
	int n = 500;
	Hitable **list = new Hitable*[n + 1];
	list[0] = new Sphere(vec3(0, -1000, 0), 1000, new lambertian(vec3(0.5, 0.5, 0.5)));
	int i = 1;
	for (int a = -11; a < 11; a++) {
		for (int b = -11; b < 11; b++) {
			float chooseMaterial = unifRand(randomNumberGenerator);
			vec3 center(a + 0.9*unifRand(randomNumberGenerator), 0.2, b + 0.9*unifRand(randomNumberGenerator));
			if ((center - vec3(4, 0.2, 0)).length() > 0.9) {
				if (chooseMaterial < 0.8) { //diffuse
					list[i++] = new Sphere(center, 0.2,
						new lambertian(
							vec3(unifRand(randomNumberGenerator) * unifRand(randomNumberGenerator),
							unifRand(randomNumberGenerator) * unifRand(randomNumberGenerator),
							unifRand(randomNumberGenerator) * unifRand(randomNumberGenerator))));
				}
				else if (chooseMaterial < 0.95) { //metal
					list[i++] = new Sphere(center, 0.2,
						new metal(
							vec3(0.5*(1 + unifRand(randomNumberGenerator)), 0.5*(1 + unifRand(randomNumberGenerator)), 0.5*(1 + unifRand(randomNumberGenerator))), 
							0.5*(1 + unifRand(randomNumberGenerator))
							)
						);
				}
				else { //glass
					list[i++] = new Sphere(center, 0.2, new dielectric(1.5));
				}
			}
		}
	}

	list[i++] = new Sphere(vec3(0, 1, 0), 1.0, new dielectric(1.5));
	list[i++] = new Sphere(vec3(-4, 1, 0), 1.0, new lambertian(vec3(0.4, 0.2, 0.1)));
	list[i++] = new Sphere(vec3(4, 1, 0), 1.0, new metal(vec3(0.7, 0.6, 0.5), 0.0));

	return new HitableList(list, i);
}