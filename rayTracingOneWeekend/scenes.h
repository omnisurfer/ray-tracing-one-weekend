#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <stdlib.h>

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


Hitable *randomScene() {
	//drowan 20190210: maybe use camera lookat to figure out the centerX and Y coords?
	int n = 100;
	Hitable **list = new Hitable*[n + 1];

	Texture *checker = new CheckerTexture(
		new ConstantTexture(vec3(0.2, 0.2, 0.8)),
		new ConstantTexture(vec3(0.9, 0.9, 0.9))
	);

	Texture *perlin = new NoiseTexture(true, 8.0f);

	Texture *constant = new ConstantTexture(vec3(0.0, 1.0, 0.0));

	Material *emitterMat = new DiffuseLight(new ConstantTexture(vec3(4, 4, 4)));

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
	// drowan(20190607) TODO: figure out what magic number 3 was supposed to be (I forgot...)
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

	//std::cout << "n+1 = " << n << " i= " << i << "\n";
	return new BvhNode(list, i, 0.0, 1.0);
	//return new HitableList(list, i);
}

Hitable *cornellBox() {
	Hitable **list = new Hitable*[100];
	int i = 0;

	Material *red = new Lambertian(new ConstantTexture(vec3(0.65, 0.05, 0.05)));
	Material *white = new Lambertian(new ConstantTexture(vec3(0.73, 0.73, 0.73)));
	Material *green = new Lambertian(new ConstantTexture(vec3(0.12, 0.45, 0.15)));
	Material *light = new DiffuseLight(new ConstantTexture(vec3(4, 4, 4)));
	Material *blue = new Lambertian(new ConstantTexture(vec3(0.12, 0.12, 0.45)));

#if 1
	list[i++] = new FlipNormals(new YZRectangle(0, 555, 0, 555, 555, green));
	list[i++] = new YZRectangle(0, 555, 0, 555, 0, red);
	list[i++] = new XZRectangle(113, 443, 127, 432, 554, light);
	list[i++] = new FlipNormals(new XZRectangle(0, 555, 0, 555, 555, white));
	list[i++] = new XZRectangle(0, 555, 0, 555, 0, white);
	list[i++] = new FlipNormals(new XYRectangle(0, 555, 0, 555, 555, white));
#endif

	//add boxes
#if 0
	list[i++] = new Box(vec3(50, 100, 100), vec3(200, 200, 200), blue);
	list[i++] = new Box(vec3(265, 0, 295), vec3(430, 330, 460), white);
#else

	list[i++] = new Translate(
		new RotateY(new Box(vec3(0, 0, 0), vec3(165, 165, 165), red), -18.0),
		vec3(130, 0, 65)
	);

	Hitable *box = new Translate(
		new RotateY(new Box(vec3(0, 0, 0), vec3(165, 330, 165), blue), 15.0),
		vec3(265, 0, 295)
	);

	// make a smoke box
	//list[i++] = new ConstantMedium(box, 0.01, new ConstantTexture(vec3(0.2, 0.6, 0.3)));

	Material *innerSpehereMat = new Lambertian(new ConstantTexture(vec3(0.1, 0.05, 0.60)));

	Hitable *outerSphere = new Sphere(vec3(350, 120, 350), 150.0, new Dielectric(1.5));
	Hitable *innerSphere = new Sphere(vec3(350, 120, 350), 149.0, innerSpehereMat);

	//list[i++] = new ConstantMedium(innerSphere, 0.001, new ConstantTexture(vec3(0.4, 0.6, 0.4)));
	list[i++] = innerSphere;
	list[i++] = outerSphere;

#endif
	return new HitableList(list, i);
}