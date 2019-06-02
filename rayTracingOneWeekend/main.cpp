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
#include "xy_rect.h"
#include "box.h"
#include "material.h"
#include "constantMedium.h"
#include "hitableList.h"
#include "float.h"
#include "camera.h"
#include "color.h"
#include "bvhNode.h"

#include "debug.h"

#include "winDIBbitmap.h"

//https://github.com/nothings/stb
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

/* TODO:
	- drowan 20190120: More cleanly seperate and encapsulate functions and implement general OOP best practices. For now, just trying to get through the book.
	- drowan 20190121: I need to really refactor and clean up code style. unifRand is being pulled from material.
	- drowan 20190601: Look into this:
		https://eli.thegreenplace.net/2016/c11-threads-affinity-and-hyperthreading/
*/

/* 
* https://github.com/petershirley/raytracinginoneweekend
* http://jamie-wong.com/2016/07/15/ray-marching-signed-distance-functions/
* http://www.codinglabs.net/article_world_view_projection_matrix.aspx
* http://iquilezles.org/index.html
*/

Hitable *randomScene();
Hitable *cornellBox();

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

	int32_t resWidth = 600, resHeight = 600;
	uint8_t bytesPerPixel = (winDIBBmp.getBitsPerPixel() / 8);
	uint32_t antiAliasingSamples = 200;

	uint32_t tempImageBufferSizeInBytes = resWidth * resHeight * bytesPerPixel;

	std::unique_ptr<uint8_t> tempImageBuffer(new uint8_t[tempImageBufferSizeInBytes]);				

	//Setup camera
	vec3 lookFrom(0, 0, -10);
	vec3 lookAt(0, 10, 0);
	vec3 worldUp(0, 1, 0);
	float distToFocus = 10.0; //(lookFrom - lookAt).length();
	float aperture = 0.0;
	float aspectRatio = float(resWidth) / float(resHeight);
	float vFoV = 40.0;

	Camera mainCamera(lookFrom, lookAt, worldUp, vFoV, aspectRatio, aperture, distToFocus, 0.0, 1.0);

	mainCamera.setLookFrom(vec3(0, 0, -10));
	mainCamera.setLookAt(vec3(0, 0, 0));

	//cornell box
	mainCamera.setLookFrom(vec3(278, 278, -800));
	mainCamera.setLookAt(vec3(278, 278, 0));

	//world bundles all the hitables and provides a generic way to call hit recursively in color (it's hit calls all the objects hits)

	Hitable *world = cornellBox(); //randomScene();

	//world = cornellBox();
	
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
	//drowan 20190127: The code below is basically hardcoded to generate ~400 spheres. When I try to make the list smaller than this, it tries to access
	//out of bounds memory.
	//drowan 20190210: maybe use camera lookat to figure out the centerX and Y coords?
	int n = 110;
	Hitable **list = new Hitable*[n + 1];

	Texture *checker = new CheckerTexture(
		new ConstantTexture(vec3(0.2, 0.2, 0.8)), 
		new ConstantTexture(vec3(0.9,0.9,0.9))
	);

	Texture *perlin = new NoiseTexture(true, 8.0f);

	Texture *constant = new ConstantTexture(vec3(0.0, 1.0, 0.0));

	Material *emitterMat = new DiffuseLight(new ConstantTexture(vec3(4,4,4)));

	//read in an image for texture mapping
	int nx, ny, nn;
	unsigned char *textureData = stbi_load("./input_images/earth1300x1300.jpg", &nx, &ny, &nn, 0);
	//unsigned char *textureData = stbi_load("./input_images/red750x750.jpg", &nx, &ny, &nn, 0);

	Material *imageMat = new Lambertian(new ImageTexture(textureData, nx, ny));

	list[0] = new Sphere(vec3(0, -1000, 0), 1000, new Lambertian(perlin));

	int i = 1;

	const int xMod = 5, yMod = 5;

	int centerX = -xMod, centerY = -yMod;
#if 1
	while (i < n - 3) {
		float chooseMaterial = unifRand(randomNumberGenerator);
		
		vec3 center(centerX + 0.9*unifRand(randomNumberGenerator), 0.2, centerY + 0.9*unifRand(randomNumberGenerator));

		if ((center - vec3(4, 0.2, 0)).length() > 0.9) {
			if (chooseMaterial < 0.8) { //diffuse
				list[i++] = new MovingSphere(center,
					center + vec3(0, 0.5*unifRand(randomNumberGenerator), 0),
					0.0,
					1.0,
					0.2,
					new Lambertian(checker)
				);
			}
			else if (chooseMaterial < 0.95) { //metal
				list[i++] = new Sphere(center, 0.2,
					new Metal(
						vec3(0.5*(1 + unifRand(randomNumberGenerator)), 0.5*(1 + unifRand(randomNumberGenerator)), 0.5*(1 + unifRand(randomNumberGenerator))),
						0.5*(1 + unifRand(randomNumberGenerator))
					)
				);
			}
			else { //glass
				list[i++] = new Sphere(center, 0.2, new Dielectric(1.5));
			}
		}

		if (centerX < xMod) {
			centerX++;
		}
		else {
			centerX = -xMod;
			centerY++;
		}

		if (centerY > yMod) {
			centerY = -yMod;
		}

		//std::cout << __func__ << "cX: " << centerX << " cY: " << centerY << "\n";
	}
#endif
	if (textureData != NULL) {
		list[i++] = new Sphere(vec3(0, 2, 0), 2.0, imageMat);
	}
	else {
		list[i++] = new Sphere(vec3(0, 1, 0), 1.0, new Dielectric(1.5));
	}
#if 1
	list[i++] = new Sphere(vec3(-4, 1, 0), 1.0, new Lambertian(perlin));
	list[i++] = new Sphere(vec3(4, 1, 0), 1.0, new Metal(vec3(0.7, 0.6, 0.5), 0.0));

	list[i++] = new XYRectangle(0, 2, 0, 2, -6, emitterMat);
#endif
	
	std::cout << "n+1 = " << n << " i= " << i << "\n";
	return new BvhNode(list, i, 0.0, 1.0);
	//return new HitableList(list, i);
}

Hitable *cornellBox() {
	Hitable **list = new Hitable*[8];
	int i = 0;

	Material *red = new Lambertian(new ConstantTexture(vec3(0.65, 0.05, 0.05)));
	Material *white = new Lambertian(new ConstantTexture(vec3(0.73, 0.73, 0.73)));
	Material *green = new Lambertian(new ConstantTexture(vec3(0.12, 0.45, 0.15)));
	Material *light = new DiffuseLight(new ConstantTexture(vec3(7, 7, 7)));
	Material *blue = new Lambertian(new ConstantTexture(vec3(0.12, 0.12, 0.45)));


	list[i++] = new FlipNormals(new YZRectangle(0, 555, 0, 555, 555, green));	
#if 1
	list[i++] = new YZRectangle(0, 555, 0, 555, 0, red);
	list[i++] = new XZRectangle(113, 443, 127, 432, 554, light);	
	list[i++] = new FlipNormals(new XZRectangle(0, 555, 0, 555, 555, white));
	list[i++] = new XZRectangle(0, 555, 0, 555, 0, white);
	list[i++] = new FlipNormals(new XYRectangle(0, 555, 0, 555, 555, white));
#endif

	//add boxes
#if 0
	list[i++] = new Box(vec3(100, 100, 100), vec3(200, 200, 200), blue);
	list[i++] = new Box(vec3(265, 0, 295), vec3(430, 330, 460), white);
#else

	list[i++] = new Translate(
		new RotateY(new Box(vec3(0, 0, 0), vec3(165, 165, 165), red), -18.0), 
		vec3(130,0,65)
	);

	Hitable *box = new Translate(
		new RotateY(new Box(vec3(0, 0, 0), vec3(165, 330, 165), blue), 15.0),
		vec3(265,0,295)
	);
		
	// make a smoke box
	list[i++] = new ConstantMedium(box, 0.01, new ConstantTexture(vec3(0.2, 0.6, 0.3)));

#endif
	return new HitableList(list, i);
}