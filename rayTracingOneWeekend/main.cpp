#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <vector>
#include <random>
#include <chrono>
#include <list>
#include <iomanip>

#include <thread>
#include <mutex>

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

#define DEFAULT_RENDER_WIDTH 100
#define DEFAULT_RENDER_HEIGHT 100
#define DEFAULT_RENDER_AA 1

/* TODO:
	- drowan 20190120: More cleanly seperate and encapsulate functions and implement general OOP best practices. For now, just trying to get through the book.
	- drowan 20190121: I need to really refactor and clean up code style. unifRand is being pulled from material.
	- drowan 20190601: Look into this:
		https://eli.thegreenplace.net/2016/c11-threads-affinity-and-hyperthreading/
	- drowan 20190607: Look into OpenMPI???
*/

/* 
* https://github.com/petershirley/raytracinginoneweekend
* http://jamie-wong.com/2016/07/15/ray-marching-signed-distance-functions/
* http://www.codinglabs.net/article_world_view_projection_matrix.aspx
* http://iquilezles.org/index.html
*/

Hitable *randomScene();
Hitable *cornellBox();

struct RenderProperties {
	uint32_t resWidthInPixels, resHeightInPixels;
	uint8_t bytesPerPixel;
	uint32_t antiAliasingSamplesPerPixel;
	uint32_t finalImageBufferSizeInBytes;
};

struct WorkerThreadStruct {
	uint32_t threadId;
	bool threadDone;
	std::thread threadHandle;
};

struct WorkerImageBufferStruct {
	uint32_t sizeInBytes;
	std::shared_ptr<uint8_t> buffer;
};

// going to try and pass a buffer per thread and combine afterwards so as to avoid memory contention when using a mutex which may slow things down...
//WorkerThreadProperties *workerThreadProperties, , WorkerImageBuffer *workerImageBuffer
void workerThreadFunction(std::shared_ptr<WorkerThreadStruct> workerThreadStruct, std::shared_ptr<WorkerImageBufferStruct> workerImageBufferStruct, std::mutex *coutGuard, RenderProperties renderProps, Camera sceneCamera, Hitable *world) {
	int numOfThreads = std::thread::hardware_concurrency();

	std::unique_lock<std::mutex> coutLock(*coutGuard);	
	std::cout << "\nHello from thread with ID: " << workerThreadStruct.get()->threadId << "\n";
	std::cout << "Lookat: " << sceneCamera.getLookAt() << "\n";
	std::cout << "World hitable address:  " << world << "\n";

	std::cout << "Image buffer address: " << &workerImageBufferStruct << " @[0]: " << workerImageBufferStruct->buffer.get()[0] << " Size: " << workerImageBufferStruct->sizeInBytes << "\n";	
	std::cout << "Raytracing starting...\n";
	coutLock.unlock();

	// drowan(20190613): Need to think of how the chunks will be made as the cores change. For now, splitting into stripes 
	// based on height with the same width
	uint32_t chunkResHeightInPixels = static_cast<uint32_t> (renderProps.resHeightInPixels / numOfThreads);	
	uint32_t rowOffsetInPixels = 0;
	uint32_t chunkResWidthInPixels = static_cast<uint32_t> (renderProps.resWidthInPixels);
	uint32_t chunkResHeigthInPixelsOffset = 0;

	// give last thread any extra rows that did not divide cleanly between the threads.
	/*

	|_______| n * 0
	|_______| n * 1
	|_______| n * 3
	|		| n * 4
	|       |
	|_______|	

	*/

#if 1
	if (workerThreadStruct->threadId == numOfThreads - 1) {
		rowOffsetInPixels = (workerThreadStruct->threadId * chunkResHeightInPixels);
		chunkResHeightInPixels += renderProps.resHeightInPixels%numOfThreads;
	}
	else {
		rowOffsetInPixels = (workerThreadStruct->threadId * chunkResHeightInPixels);
	}

	for (int row = chunkResHeightInPixels - 1; row >= 0; row--) {
		// std::cout << "Row " << renderProps.resHeightInPixels - row - (rowOffsetInPixels) << " (u, v): ";
		
		for (int column = 0; column < chunkResWidthInPixels; column++) {

			// calculate u and v. This is a naive implementation that does not account for non-square or even dimensions
			float u = (float)column / (float)chunkResWidthInPixels;									
			float v = ((float)row + rowOffsetInPixels) / (float)renderProps.resHeightInPixels;
			
			if (column % 1000 == 0 || column == 0) {
				// std::cout << "(" << std::setprecision(2) << u << "," << v << ") ";
				// std::cout << ". ";
			}

			vec3 outputColor(0, 0, 0);
			//loop to produce AA samples
			for (int sample = 0; sample < renderProps.antiAliasingSamplesPerPixel; sample++) {
				float u = float(column + unifRand(randomNumberGenerator)) / float(renderProps.resWidthInPixels);
				float v = float(row + unifRand(randomNumberGenerator)) / float(renderProps.resHeightInPixels);

				//A, the origin of the ray (camera)
				//rayCast stores a ray projected from the camera as it points into the scene that is swept across the uv "picture" frame.
				ray rayCast = sceneCamera.getRay(u, v);

				//NOTE: not sure about magic number 2.0 in relation with my tweaks to the viewport frame
				vec3 pointAt = rayCast.pointAtParameter(2.0);
				outputColor += color(rayCast, world, 0);
			}

			outputColor /= float(renderProps.antiAliasingSamplesPerPixel);
			outputColor = vec3(sqrt(outputColor[0]), sqrt(outputColor[1]), sqrt(outputColor[2]));
			// drowan(20190602): This seems to perform a modulo remap of the value. 362 becomes 106 maybe remap to 255? Does not seem to work right.
			// Probably related to me outputing to bitmap instead of the ppm format...
			uint8_t ir = 0;
			uint8_t ig = 0;
			uint8_t ib = 0;

			uint16_t irO = uint16_t(255.99 * outputColor[0]);
			uint16_t igO = uint16_t(255.99 * outputColor[1]);
			uint16_t ibO = uint16_t(255.99 * outputColor[2]);

			// cap the values to 255 max
			(irO > 255) ? ir = 255 : ir = uint8_t(irO);
			(igO > 255) ? ig = 255 : ig = uint8_t(igO);
			(ibO > 255) ? ib = 255 : ib = uint8_t(ibO);

			//also store values into tempBuffer			
			workerImageBufferStruct->buffer.get()[row*renderProps.resWidthInPixels * renderProps.bytesPerPixel + (column * renderProps.bytesPerPixel)] = ib;
			workerImageBufferStruct->buffer.get()[row*renderProps.resWidthInPixels * renderProps.bytesPerPixel + (column * renderProps.bytesPerPixel) + 1] = ig;
			workerImageBufferStruct->buffer.get()[row*renderProps.resWidthInPixels * renderProps.bytesPerPixel + (column * renderProps.bytesPerPixel) + 2] = ir;
		}
		//std::cout << "\n";
		//coutLock.unlock();
	}
#endif
	// original single thread loop
#if 0
	//main raytracing loops, the movement across u and v "drive" the render (i.e. when to stop)
	for (int row = renderProps.resHeightInPixels - 1; row >= 0; row--) {
		if (row % 10 == 0 || row == renderProps.resHeightInPixels - 1)
			std::cout << "\nRow " << row << " ";

		int columnProgress = 0;
		//loop to move ray across width of frame
		for (int column = 0; column < renderProps.resWidthInPixels; column++, columnProgress++) {
			if (columnProgress % 1000 == 0 || columnProgress == 0)
				std::cout << ". ";

			vec3 outputColor(0, 0, 0);
			//loop to produce AA samples
			for (int sample = 0; sample < renderProps.antiAliasingSamplesPerPixel; sample++) {
				float u = float(column + unifRand(randomNumberGenerator)) / float(renderProps.resWidthInPixels);
				float v = float(row + unifRand(randomNumberGenerator)) / float(renderProps.resHeightInPixels);

				//A, the origin of the ray (camera)
				//rayCast stores a ray projected from the camera as it points into the scene that is swept across the uv "picture" frame.
				ray rayCast = sceneCamera.getRay(u, v);

				//NOTE: not sure about magic number 2.0 in relation with my tweaks to the viewport frame
				vec3 pointAt = rayCast.pointAtParameter(2.0);
				outputColor += color(rayCast, world, 0);
			}

			outputColor /= float(renderProps.antiAliasingSamplesPerPixel);
			outputColor = vec3(sqrt(outputColor[0]), sqrt(outputColor[1]), sqrt(outputColor[2]));
			// drowan(20190602): This seems to perform a modulo remap of the value. 362 becomes 106 maybe remap to 255? Does not seem to work right.
			// Probably related to me outputing to bitmap instead of the ppm format...
			uint8_t ir = 0;
			uint8_t ig = 0;
			uint8_t ib = 0;

			uint16_t irO = uint16_t(255.99 * outputColor[0]);
			uint16_t igO = uint16_t(255.99 * outputColor[1]);
			uint16_t ibO = uint16_t(255.99 * outputColor[2]);

			// cap the values to 255 max
			(irO > 255) ? ir = 255 : ir = uint8_t(irO);
			(igO > 255) ? ig = 255 : ig = uint8_t(igO);
			(ibO > 255) ? ib = 255 : ib = uint8_t(ibO);

			//also store values into tempBuffer
			workerImageBuffer.get()[row*renderProps.resWidthInPixels * renderProps.bytesPerPixel + (column * renderProps.bytesPerPixel)] = ib;
			workerImageBuffer.get()[row*renderProps.resWidthInPixels * renderProps.bytesPerPixel + (column * renderProps.bytesPerPixel) + 1] = ig;
			workerImageBuffer.get()[row*renderProps.resWidthInPixels * renderProps.bytesPerPixel + (column * renderProps.bytesPerPixel) + 2] = ir;
		}
	}	
#endif

	coutLock.lock();
	std::cout << "\nRaytracing worker " << workerThreadStruct->threadId << " finished!\n";
	coutLock.unlock();

	//indicate that ray tracing is complete
	// TODO: wrap this in a mutex
	workerThreadStruct->threadDone = true;

	return;
}

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

	RenderProperties renderProps;

	renderProps.resHeightInPixels = DEFAULT_RENDER_WIDTH;
	renderProps.resWidthInPixels = DEFAULT_RENDER_HEIGHT;
	renderProps.antiAliasingSamplesPerPixel = DEFAULT_RENDER_AA;
	renderProps.bytesPerPixel = (winDIBBmp.getBitsPerPixel() / 8);

	uint32_t numOfThreads = 7; // std::thread::hardware_concurrency();
	std::cout << "You have " << numOfThreads << " hardware threads.\n";

	//ask for image dimensions
	std::cout << "Enter render width: ";
	//std::cin >> renderProps.resWidthInPixels;

	//drowan(20190607) BUG: If the width and height is set to 1x1, I get heap corruption in the BMP writer. 
	//For now, going to use a minimum value to step around this issue until I can fix it.
	if (std::cin.fail()) {
		std::cin.clear();
		std::cin.ignore(INT_MAX, '\n');
		renderProps.resWidthInPixels = DEFAULT_RENDER_WIDTH;
		std::cout << "Invalid input, using default: " << renderProps.resWidthInPixels << '\n';
	}
	else {
		if (renderProps.resWidthInPixels < DEFAULT_RENDER_WIDTH) {
			renderProps.resWidthInPixels = DEFAULT_RENDER_WIDTH;
			std::cout << "Minimum width set: " << DEFAULT_RENDER_WIDTH << "\n";
		}
	}

	std::cout << "Enter render height: ";
	//std::cin >> renderProps.resHeightInPixels;

	if (std::cin.fail()) {
		std::cin.clear();
		std::cin.ignore(INT_MAX, '\n');
		renderProps.resHeightInPixels = DEFAULT_RENDER_HEIGHT;
		std::cout << "Invalid input, using default: " << renderProps.resHeightInPixels << '\n';
	}
	else {
		if (renderProps.resHeightInPixels < DEFAULT_RENDER_HEIGHT) {
			renderProps.resHeightInPixels = DEFAULT_RENDER_HEIGHT;
			std::cout << "Minimum height set: " << DEFAULT_RENDER_HEIGHT << "\n";
		}
	}

	std::cout << "Enter number of aliasing samples (also helps increase photon count): ";
	//std::cin >> renderProps.antiAliasingSamplesPerPixel;

	if (std::cin.fail()) {
		std::cin.clear();
		std::cin.ignore(INT_MAX, '\n');
		renderProps.antiAliasingSamplesPerPixel = DEFAULT_RENDER_AA;
		std::cout << "Invalid input, using default: " << renderProps.antiAliasingSamplesPerPixel << '\n';
	}
	else {
		if (renderProps.antiAliasingSamplesPerPixel < DEFAULT_RENDER_AA) {
			renderProps.antiAliasingSamplesPerPixel = DEFAULT_RENDER_AA;
			std::cout << "Minimum AA set: " << DEFAULT_RENDER_AA << "\n";
		}
	}
	
	//Setup camera
	vec3 lookFrom(0, 0, -10);
	vec3 lookAt(0, 10, 0);
	vec3 worldUp(0, 1, 0);
	float distToFocus = 10.0; //(lookFrom - lookAt).length();
	float aperture = 0.0;
	float aspectRatio = float(renderProps.resWidthInPixels) / float(renderProps.resHeightInPixels);
	float vFoV = 40.0;

	Camera mainCamera(lookFrom, lookAt, worldUp, vFoV, aspectRatio, aperture, distToFocus, 0.0, 1.0);

	// drowan(20190607) TOOD: make a way to select this programatically?
#if 0
	//random scene
	mainCamera.setLookFrom(vec3(3, 3, -10));
	mainCamera.setLookAt(vec3(0, 0, 0));

	//world bundles all the hitables and provides a generic way to call hit recursively in color (it's hit calls all the objects hits)
	Hitable *world = randomScene();
#else
	//cornell box
	mainCamera.setLookFrom(vec3(278, 278, -800));
	mainCamera.setLookAt(vec3(278, 278, 0));

	Hitable *world = cornellBox();
#endif

	renderProps.finalImageBufferSizeInBytes = renderProps.resWidthInPixels * renderProps.resHeightInPixels * renderProps.bytesPerPixel;

	std::cout << "Buffer size in bytes: " << renderProps.finalImageBufferSizeInBytes << "\n";	

	std::vector<std::shared_ptr<WorkerImageBufferStruct>> workerImageBufferStructVector;
	std::vector<std::shared_ptr<WorkerThreadStruct>> workerThreadStructVector;

	std::shared_ptr<uint8_t> finalImageBuffer(new uint8_t[renderProps.finalImageBufferSizeInBytes]);

	//drowan(20190607): maybe look into this: https://stackoverflow.com/questions/9332263/synchronizing-std-cout-output-multi-thread
	// maybe combine these into a single structure?
	//std::vector<std::thread> workerThreadVector;
	std::mutex coutGuard;
	
	for (int i = 0; i < numOfThreads; i++) {
		
		std::shared_ptr<WorkerImageBufferStruct> workerImageBufferStruct(new WorkerImageBufferStruct);
		
		workerImageBufferStruct->sizeInBytes = static_cast<uint32_t>(renderProps.finalImageBufferSizeInBytes / numOfThreads);
		
		//the last thread may get a different amount of bytes to process
		if (i == numOfThreads - 1) {
			workerImageBufferStruct->sizeInBytes = renderProps.finalImageBufferSizeInBytes - ((workerImageBufferStruct->sizeInBytes) * (numOfThreads - 1));
		}

		std::shared_ptr<uint8_t> _workingImageBuffer(new uint8_t[workerImageBufferStruct->sizeInBytes]);

		for (int x = 0; x < workerImageBufferStruct->sizeInBytes; x++) {
			_workingImageBuffer.get()[x] = 0x41 + x + i;
		}
		
		workerImageBufferStruct->buffer = std::move(_workingImageBuffer);
		workerImageBufferStructVector.push_back(workerImageBufferStruct);
						
		std::shared_ptr<WorkerThreadStruct> workerThreadStruct(new WorkerThreadStruct);
		
		// drowan(20190616): Possible race condition with some of the variables not being assigned until after the thread starts.
		std::thread workerThread;

		workerThreadStruct.get()->threadId = i;
		workerThreadStruct.get()->threadDone = false;
		workerThreadStruct.get()->threadHandle = std::move(workerThread);

		workerThreadStruct.get()->threadHandle = std::thread(workerThreadFunction, workerThreadStruct, workerImageBufferStruct, &coutGuard, renderProps, mainCamera, world);

		workerThreadStructVector.push_back(workerThreadStruct);		
	}
	
	// join threads		
	for (std::shared_ptr<WorkerThreadStruct> &thread : workerThreadStructVector) {
		
		while (!thread->threadDone) {
			//std::cout << "waiting...\n";
		}

		if (thread->threadHandle.joinable()) {
			thread->threadHandle.join();
		}
	}

	std::cout << "Writing to bmp file...\n";
#if 0
	uint32_t finalBufferIndex = 0;
	// copy contents from worker Buffers into final Image buffer
	for (WorkerImageBuffer &workerImageBuffer : workerImageBufferVector) {
		
		//get the buffer size from renderprops.		
		for (int i = 0; i < workerImageBuffer.sizeInBytes;  i++) {
			finalImageBuffer.get()[finalBufferIndex] = workerImageBuffer.buffer.get()[i];
		}
	}

	winDIBBmp.writeBMPToFile(finalImageBuffer.get(), renderProps.finalImageBufferSizeInBytes, renderProps.resWidthInPixels, renderProps.resHeightInPixels, BMP_BITS_PER_PIXEL);
#endif
	delete[] world;

	DEBUG_MSG_L0("colorCallCount: ", colorCallCount);

	//DEBUG pause
	// putting a \n triggers cin...?
	// drowan(20190607) BUG: For some reason if the rendered scene is small (10x10 pixels) 
	// the cin.get() is completely bypassed unless I put a cin.ignore(). Is the thread exiting
	// weird relative to the main thread???
	std::cout << "Hit any key to exit...";
	std::cin.ignore();
	std::cin.get();

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
	Material *light = new DiffuseLight(new ConstantTexture(vec3(4, 4, 4)));
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
	//list[i++] = box;

#endif
	return new HitableList(list, i);
}