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

	Material *emitterMat = new DiffuseLight(new ConstantTexture(vec3(20 * 1, 20 * 0, 20 * 0)));

	//read in an image for texture mapping
	int nx, ny, nn;
	unsigned char *textureData = stbi_load("./input_images/earth1300x1300.jpg", &nx, &ny, &nn, 0);
	//unsigned char *textureData = stbi_load("./input_images/red750x750.jpg", &nx, &ny, &nn, 0);

	Material *imageMat = new Lambertian(new ImageTexture(textureData, nx, ny));

	list[0] = new Sphere(vec3(0, 1000, 0), 1000, new Lambertian(perlin));

	int i = 1;

	const int xMod = 5, yMod = 5;

	int centerX = -xMod, centerY = -yMod;
#if 1
	// drowan(20190607) TODO: figure out what magic number 3 was supposed to be (I forgot...)
	while (i < n - 3) {
		float chooseMaterial = unifRand(randomNumberGenerator);

		vec3 center(centerX + 0.9*unifRand(randomNumberGenerator), -0.2, centerY + 0.9*unifRand(randomNumberGenerator));

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
		list[i++] = new Sphere(vec3(0, -2.0, 20), 2.0, imageMat);
	}
	else {
		list[i++] = new Sphere(vec3(5, -1.0, 0), 1.0, new Dielectric(1.5));
	}
#if 1
	list[i++] = new Sphere(vec3(-4, -1.0, 0), 1.0, new Lambertian(perlin));
	list[i++] = new Sphere(vec3(4, -1.0, 0), 1.0, new Metal(vec3(0.7, 0.6, 0.5), 0.0));		
	//basic "sun"?
	list[i++] = new Sphere(vec3(0, -400, 10), 100.0, emitterMat);
#endif

	//std::cout << "n+1 = " << n << " i= " << i << "\n";
	return new BvhNode(list, i, 0.0, 1.0);
	//return new HitableList(list, i);
}

Hitable *randomScene_NED() {
	//drowan 20190210: maybe use camera lookat to figure out the centerX and Y coords?
	int n = 100;
	Hitable **list = new Hitable*[n + 1];

	Texture *checker = new CheckerTexture(
		new ConstantTexture(vec3(0.2, 0.2, 0.8)),
		new ConstantTexture(vec3(0.9, 0.9, 0.9))
	);

	Texture *perlin = new NoiseTexture(true, 8.0f);

	Texture *constant = new ConstantTexture(vec3(0.0, 1.0, 0.0));

	Material *emitterMat = new DiffuseLight(new ConstantTexture(vec3(20 * 1, 20 * 0, 20 * 0)));

	//read in an image for texture mapping
	int nx, ny, nn;
	unsigned char *textureData = stbi_load("./input_images/1_earth_8k.jpg", &nx, &ny, &nn, 0);
	//unsigned char *textureData = stbi_load("./input_images/earth1300x1300.jpg", &nx, &ny, &nn, 0);
	//unsigned char *textureData = stbi_load("./input_images/red750x750.jpg", &nx, &ny, &nn, 0);

	Material *imageMat = new Lambertian(new ImageTexture(textureData, nx, ny));

	float worldSphereRadius = 1000.0;
	float worldSphereRadiusOffset = -1 * worldSphereRadius;

	list[0] = new Sphere(vec3(0, 0, 0), worldSphereRadius, new Lambertian(perlin));

	int i = 1;

	const int xMod = 10, yMod = 10;

	int centerX = xMod, centerY = yMod;
#if 1
	// drowan(20190607) TODO: figure out what magic number 3 was supposed to be (I forgot...)
	while (i < n - 3) {
		float chooseMaterial = unifRand(randomNumberGenerator);

		vec3 center(centerX * unifRand(randomNumberGenerator), centerY * unifRand(randomNumberGenerator), worldSphereRadiusOffset - 1);

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
				list[i++] = new Sphere(center, 
					0.2,
					new Metal(
						vec3(0.5*(1 + unifRand(randomNumberGenerator)), 0.5*(1 + unifRand(randomNumberGenerator)), 0.5*(1 + unifRand(randomNumberGenerator))),
						0.5*(1 + unifRand(randomNumberGenerator))
					)
				);
			}
			else { //glass
				list[i++] = new Sphere(center, 0.5, new Dielectric(1.5));
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
		list[i++] = new Sphere(vec3(20.0, 0.0, worldSphereRadiusOffset - 25.0), 10.0, imageMat);
	}
	else {
		list[i++] = new Sphere(vec3(20.0, 0.0, worldSphereRadiusOffset - 25.0), 10.0, new Dielectric(1.5));
	}
#if 1
	list[i++] = new Sphere(vec3(100, -70, worldSphereRadiusOffset), 50.0, new Lambertian(perlin));
	list[i++] = new Sphere(vec3(100, 70, worldSphereRadiusOffset), 50.0, new Metal(vec3(0.7, 0.6, 0.5), 0.0));
	//basic "sun"?
	list[i++] = new Sphere(vec3(0, 0, worldSphereRadiusOffset - 500), 100.0, emitterMat);
#endif

	//std::cout << "n+1 = " << n << " i= " << i << "\n";
	return new BvhNode(list, i, 0.0, 1.0);
	//return new HitableList(list, i);
}

Hitable *cornellBox() {
	Hitable **list = new Hitable*[100];
	int i = 0;

	Material *white = new Lambertian(new ConstantTexture(vec3(0.73, 0.73, 0.73)));
	Material *red = new Lambertian(new ConstantTexture(vec3(0.65, 0.05, 0.05)));
	Material *green = new Lambertian(new ConstantTexture(vec3(0.12, 0.95, 0.15)));
	Material *blue = new Lambertian(new ConstantTexture(vec3(0.12, 0.12, 0.45)));	
	Material *yellow = new Lambertian(new ConstantTexture(vec3(255/255, 255/255, 0/255)));

	Material *light = new DiffuseLight(new ConstantTexture(vec3(3, 3, 3)));
	Material *redLight = new DiffuseLight(new ConstantTexture(vec3(4, 0, 0)));
	Material *greenLight = new DiffuseLight(new ConstantTexture(vec3(0, 4, 0)));
	Material *blueLight = new DiffuseLight(new ConstantTexture(vec3(0, 0, 4)));

#if 1
	int planeWidth = 500, planeHeight = 500, planeAxisDepthOffset = 500;
	int xCoord = 0, yCoord = 0;

	int x0 = (xCoord - (xCoord / 2));
	int x0Off = (planeAxisDepthOffset / 4);

	//light panel
	/**/
	list[i++] = new XZRectangle(
		x0 - x0Off - 50,
		((planeWidth / 2 + xCoord) - (xCoord / 2)) - (planeAxisDepthOffset / 4) + 50,
		(yCoord - (yCoord / 2)) + (planeAxisDepthOffset / 4) - 50,
		((planeHeight / 2 + yCoord) - (yCoord / 2)) + (planeAxisDepthOffset / 4) + 50,
		(-planeAxisDepthOffset / 2) + 1,
		light
	);
	/**/

#endif
#if 1
	//light panel - does not seem to effect the left panel??
	/*
	list[i++] = new XZRectangle(
		x0 - (planeAxisDepthOffset / 4),
		((planeWidth / 2 + xCoord) - (xCoord / 2)) - (planeAxisDepthOffset / 4),
		(yCoord - (yCoord / 2)),
		((planeHeight / 2 + yCoord) - (yCoord / 2)),
		(planeAxisDepthOffset / 2) - 10,
		blueLight
	);
	/**/

	//top panel	
	list[i++] = new XZRectangle(
		x0 - (planeAxisDepthOffset / 2),
		((planeWidth + xCoord) - (xCoord / 2)) - (planeAxisDepthOffset / 2),
		(yCoord - (yCoord / 2)),
		((planeHeight + yCoord) - (yCoord / 2)),
		-(planeAxisDepthOffset / 2),
		white
	);	
	
	//left panel	
	list[i++] = new YZRectangle(
		(xCoord - (xCoord / 2)) - (planeAxisDepthOffset / 2),
		((planeWidth + xCoord) - (xCoord / 2)) - (planeAxisDepthOffset / 2),
		(yCoord - (yCoord / 2)),
		((planeHeight + yCoord) - (yCoord / 2)),
		-planeAxisDepthOffset / 2,
		red
	);
	
	//back panel
	list[i++] = new Translate(
			new FlipNormals(new XYRectangle(
		xCoord - (planeWidth / 2),
		(planeWidth + xCoord) - (planeWidth / 2),
		yCoord - (planeHeight / 2),
		(planeHeight + yCoord) - (planeHeight / 2),
		planeAxisDepthOffset,
		white
	)), vec3(0,0,0));

	//right panel
	list[i++] = new FlipNormals(new YZRectangle(
		(xCoord - (xCoord / 2)) - (planeAxisDepthOffset / 2),
		((planeWidth + xCoord) - (xCoord / 2)) - (planeAxisDepthOffset / 2),
		(yCoord - (yCoord / 2)),
		((planeHeight + yCoord) - (yCoord / 2)),
		planeAxisDepthOffset / 2,
		blue
	));
	
	//bottom panel
	list[i++] = new FlipNormals(new XZRectangle(
		(xCoord - (xCoord / 2)) - (planeAxisDepthOffset / 2),
		((planeWidth + xCoord) - (xCoord / 2)) - (planeAxisDepthOffset / 2),
		(yCoord - (yCoord / 2)),
		((planeHeight + yCoord) - (yCoord / 2)),
		(planeAxisDepthOffset / 2),
		white
	));
#endif

	//add boxes
#if 0
	list[i++] = new Box(vec3(50, 100, 100), vec3(200, 200, 200), blue);
	list[i++] = new Box(vec3(265, 0, 295), vec3(430, 330, 460), white);
#else

	/*
	list[i++] = new Translate(
		new RotateY(new Box(vec3(0, 0, 0), vec3(160, 160, 160), blue), 18.0),
		vec3(0, 80, 100)
	);
	/*
	// make a smoke box
	Hitable *box = new Translate(
		new RotateY(new Box(vec3(0, 0, 0), vec3(160, 300, 160), green), -25),
		vec3(-100, -50, 200)
	);	
	
	list[i++] = new ConstantMedium(box, 0.01, new ConstantTexture(vec3(0.2, 0.9, 0.4)));

	/**/

	Material *innerSpehereMat = new Lambertian(new ConstantTexture(vec3(0.9, 0.05, 0.10)));

	Hitable *outerSphere = new Sphere(vec3(-150, 150, 200), 100.0, new Dielectric(1.5));
	Hitable *innerSphere = new Sphere(vec3(-150, 150, 200), 99.0, innerSpehereMat);

	list[i++] = new ConstantMedium(innerSphere, 0.001, new ConstantTexture(vec3(0.8, 0.2, 0.1)));
	list[i++] = innerSphere;
	list[i++] = outerSphere;

#endif
	return new HitableList(list, i);
}

Hitable *cornellBox_NED() {
	Hitable **list = new Hitable*[100];
	int i = 0;

	Material *white = new Lambertian(new ConstantTexture(vec3(0.73, 0.73, 0.73)));
	Material *red = new Lambertian(new ConstantTexture(vec3(0.65, 0.05, 0.05)));
	Material *green = new Lambertian(new ConstantTexture(vec3(0.12, 0.95, 0.15)));
	Material *blue = new Lambertian(new ConstantTexture(vec3(0.12, 0.12, 0.45)));
	Material *yellow = new Lambertian(new ConstantTexture(vec3(255 / 255, 255 / 255, 0 / 255)));

	Material *light = new DiffuseLight(new ConstantTexture(vec3(3, 3, 3)));
	Material *redLight = new DiffuseLight(new ConstantTexture(vec3(4, 0, 0)));
	Material *greenLight = new DiffuseLight(new ConstantTexture(vec3(0, 4, 0)));
	Material *blueLight = new DiffuseLight(new ConstantTexture(vec3(0, 0, 4)));

	int planeWidth = 1000, planeHeight = 1000;
	
	//light panel	
	list[i++] = new Translate(
		new XYRectangle(
			0,
			planeWidth/2,
			0, 
			planeHeight/2,
			0,
			light
		), vec3(-planeWidth / 4, -planeWidth / 4, (-planeHeight / 2) + 1)
	);

	//top panel	
	list[i++] = new Translate(
		new XYRectangle(
			0,
			planeWidth,
			0,
			planeHeight,
			0,
			white
		), vec3(-planeWidth / 2, -planeWidth / 2, -planeHeight / 2)
	);

	//bottom panel	
	list[i++] = new Translate(
		new FlipNormals(new XYRectangle(
			0,
			planeWidth,
			0,
			planeHeight,
			0,
			white
		)), vec3(-planeWidth / 2, -planeWidth / 2, planeHeight / 2)
	);

	//left panel	
	list[i++] = new Translate(
		new XZRectangle(
			0,
			planeWidth,
			0,
			planeHeight,
			0,
			red
		), vec3(-planeWidth / 2, -planeWidth / 2, -planeHeight / 2)
	);

	//back panel
	list[i++] = new Translate(
		new FlipNormals(new YZRectangle( 
			0, 
			planeWidth, 
			0, 
			planeHeight, 
			0,
			white
		)), vec3(planeWidth / 2,-planeWidth / 2,-planeHeight / 2)
	);	

	//right panel	
	list[i++] = new Translate(
		new FlipNormals(new XZRectangle(
			0,
			planeWidth,
			0,
			planeHeight,
			0,
			blue
		)), vec3(-planeWidth / 2, planeWidth / 2, -planeHeight / 2)
	);

	//add boxes
//#if 0
	list[i++] = new Translate(
		new Box(vec3(0, 0, 0), vec3(200, 200, 300), blue), 
		vec3(0, -300, 0)
	);
	list[i++] = new Translate(
		new Box(vec3(0, 0, 0), vec3(200, 200, 300), red), 
		vec3(0, 150, 0)
	);
//#else

	/**/
	list[i++] = new Translate(
		new RotateY(new Box(vec3(0, 0, 0), vec3(160, 160, 160), blue), 18.0),
		vec3(0, 0, -200)
	);
	/*
	// make a smoke box
	Hitable *box = new Translate(
		new RotateY(new Box(vec3(0, 0, 0), vec3(160, 300, 160), green), -25),
		vec3(-100, -50, 200)
	);

	list[i++] = new ConstantMedium(box, 0.01, new ConstantTexture(vec3(0.2, 0.9, 0.4)));

	/**/

	Material *innerSpehereMat = new Lambertian(new ConstantTexture(vec3(0.1, 0.7, 0.20)));

	Hitable *outerSphere = new Sphere(vec3(0, 0, 150), 100.0, new Dielectric(1.5));
	Hitable *innerSphere = new Sphere(vec3(0, 0, 150), 99.0, innerSpehereMat);

	list[i++] = new ConstantMedium(innerSphere, 0.001, new ConstantTexture(vec3(0.1, 0.7, 0.2)));
	list[i++] = innerSphere;
	list[i++] = outerSphere;

//#endif
	return new HitableList(list, i);
}