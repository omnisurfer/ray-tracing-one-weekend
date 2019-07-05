#include <iostream>
#include <string>
#include <fstream>
#include <stdlib.h>
#include <vector>
#include <random>
#include <chrono>
#include <list>
#include <iomanip>

#include <thread>
#include <mutex>
#include <condition_variable>

#include "defines.h"
#include "vec3.h"
#include "hitableList.h"
#include "float.h"
#include "camera.h"
#include "color.h"
#include "scenes.h"

//#include "ray.h"
//#include "sphere.h"
//#include "xy_rect.h"
//#include "box.h"
//#include "material.h"
//#include "constantMedium.h"
//#include "bvhNode.h"

#include <Windows.h>
#include <tchar.h>

#include "debug.h"

#include "winDIBbitmap.h"

//https://github.com/nothings/stb
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

/* Look into:
	- drowan 20190601: https://eli.thegreenplace.net/2016/c11-threads-affinity-and-hyperthreading/
	- drowan 20190607: Use OpenMPI???
*/

/* 
* https://github.com/petershirley/raytracinginoneweekend
* http://jamie-wong.com/2016/07/15/ray-marching-signed-distance-functions/
* http://www.codinglabs.net/article_world_view_projection_matrix.aspx
* http://iquilezles.org/index.html
*/

struct RenderProperties {
	uint32_t resWidthInPixels, resHeightInPixels;
	uint8_t bytesPerPixel;
	uint32_t antiAliasingSamplesPerPixel;
	uint32_t finalImageBufferSizeInBytes;
};

struct WorkerThread {
	uint32_t id;
	bool workIsDone;
	std::mutex workIsDoneMutex;
	std::condition_variable workIsDoneConditionVar;
	bool exit;
	std::mutex exitMutex;
	std::condition_variable exitConditionVar;
	bool start;	
	std::mutex startMutex;	
	std::condition_variable startConditionVar;
	std::thread handle;
};

struct WorkerImageBuffer {
	uint32_t sizeInBytes;
	uint32_t resWidthInPixels, resHeightInPixels;
	std::shared_ptr<uint8_t> buffer;
};

Hitable *randomScene();
Hitable *cornellBox();

HBITMAP hBitmap = NULL;

void configureScene(RenderProperties &renderProps);

void workerThreadFunction(
	std::shared_ptr<WorkerThread> workerThread, 
	std::shared_ptr<WorkerImageBuffer> workerImageBuffer, 	
	RenderProperties renderProps, 
	Camera sceneCamera, 
	Hitable *world,
	std::mutex *coutGuard
);

LRESULT CALLBACK WndProc(
	_In_ HWND hwnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
);

int main() {

	DEBUG_MSG_L0(__func__, "");

	//Setup random number generator
	timeSeed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	std::seed_seq seedSequence{
			uint32_t(timeSeed & 0xffffffff),
			uint32_t(timeSeed >> 32)
	};

	randomNumberGenerator.seed(seedSequence);

	WINDIBBitmap winDIBBmp;
	RenderProperties renderProps;
	
	uint32_t numOfThreads = DEBUG_RUN_THREADS; // std::thread::hardware_concurrency();
	std::cout << "Used hardware threads: " << numOfThreads << "\n";

	configureScene(renderProps);

	renderProps.bytesPerPixel = (winDIBBmp.getBitsPerPixel() / 8);
	renderProps.finalImageBufferSizeInBytes = renderProps.resWidthInPixels * renderProps.resHeightInPixels * renderProps.bytesPerPixel;

	//Setup camera
	vec3 lookFrom(0, 0, 0);
	vec3 lookAt(0, 1, 0);
	vec3 worldUp(0, 1, 0);
	float distToFocus = (lookFrom - lookAt).length(); //10
	float aperture = 0.0;
	float aspectRatio = float(renderProps.resWidthInPixels) / float(renderProps.resHeightInPixels);
	float vFoV = 60.0;

	Camera mainCamera(lookFrom, lookAt, worldUp, vFoV, aspectRatio, aperture, distToFocus, 0.0, 1.0);

	// TODO: drowan(20190607) - should I make a way to select this programatically?
#if OUTPUT_RANDOM_SCENE == 1
	//random scene
	mainCamera.setLookFrom(vec3(3, 3, -10));
	mainCamera.setLookAt(vec3(0, 0, 0));

	//world bundles all the hitables and provides a generic way to call hit recursively in color (it's hit calls all the objects hits)
	Hitable *world = randomScene();
#else
	//cornell box
	mainCamera.setLookFrom(vec3(278, 278, -425));
	mainCamera.setLookAt(vec3(278, 278, 0));

	Hitable *world = cornellBox();
#endif	

	std::vector<std::shared_ptr<WorkerImageBuffer>> workerImageBufferVector;
	std::vector<std::shared_ptr<WorkerThread>> workerThreadVector;
	std::shared_ptr<uint8_t> finalImageBuffer(new uint8_t[renderProps.finalImageBufferSizeInBytes]);

	//drowan(20190607): maybe look into this: https://stackoverflow.com/questions/9332263/synchronizing-std-cout-output-multi-thread	
	std::mutex coutGuard;
	
	// drowan(20190630): Some threads finish sooner than others. May be worth looking into a dispatch approach that keeps all resources busy but balances against memory operations...
	//create the worker threads
	for (int i = 0; i < numOfThreads; i++) {
		
		std::shared_ptr<WorkerImageBuffer> workerImageBufferStruct(new WorkerImageBuffer);

		//figure out how many rows each thread is going to work on
		workerImageBufferStruct->resHeightInPixels = static_cast<uint32_t>(renderProps.resHeightInPixels / numOfThreads);
		workerImageBufferStruct->resWidthInPixels = renderProps.resWidthInPixels;

		//if the last thread, get any leftover rows
		if (i == numOfThreads - 1) {
			workerImageBufferStruct->resHeightInPixels += renderProps.resHeightInPixels%numOfThreads;
		}
		
		workerImageBufferStruct->sizeInBytes = workerImageBufferStruct->resHeightInPixels * workerImageBufferStruct->resWidthInPixels * renderProps.bytesPerPixel;

		std::shared_ptr<uint8_t> _workingImageBuffer(new uint8_t[workerImageBufferStruct->sizeInBytes]);
		
		workerImageBufferStruct->buffer = std::move(_workingImageBuffer);
		workerImageBufferVector.push_back(workerImageBufferStruct);
						
		std::shared_ptr<WorkerThread> workerThread(new WorkerThread);
		
		// drowan(20190616): Possible race condition with some of the variables not being assigned until after the thread starts.		
		workerThread->id = i;
		workerThread->workIsDone = false;
		workerThread->handle = std::thread(workerThreadFunction, workerThread, workerImageBufferStruct, renderProps, mainCamera, world, &coutGuard);

		workerThreadVector.push_back(workerThread);			
	}

	//start the threads
	for (std::shared_ptr<WorkerThread> &thread : workerThreadVector) {
		std::unique_lock<std::mutex> startLock(thread->startMutex);
		thread->start = true;
		thread->startConditionVar.notify_all();
		startLock.unlock();
	}
	
	// join threads	
	for (std::shared_ptr<WorkerThread> &thread : workerThreadVector) {
		
		std::unique_lock<std::mutex> doneLock(thread->workIsDoneMutex);
		while (!thread->workIsDone) {
			thread->workIsDoneConditionVar.wait(doneLock);
		}

		//signal to the thread to exit
		std::unique_lock<std::mutex> threadExitLock(thread->exitMutex);
		thread->exit = true;
		thread->exitConditionVar.notify_all();
		threadExitLock.unlock();

		if (thread->handle.joinable()) {
			thread->handle.join();
		}
	}
	
#if OUTPUT_BMP == 1
	std::cout << "Writing to bmp file...\n";

	uint32_t finalBufferIndex = 0;
	// copy contents from worker Buffers into final Image buffer	
	for (std::shared_ptr<WorkerImageBuffer> &workerImageBuffer : workerImageBufferVector) {
		
		//get the buffer size from renderprops.		
		for (int i = 0; i < workerImageBuffer->sizeInBytes;  i++) {
			if (finalBufferIndex < renderProps.finalImageBufferSizeInBytes) {
				finalImageBuffer.get()[finalBufferIndex] = workerImageBuffer->buffer.get()[i];
				finalBufferIndex++;
			}
		}
	}

	winDIBBmp.writeBMPToFile(finalImageBuffer.get(), renderProps.finalImageBufferSizeInBytes, renderProps.resWidthInPixels, renderProps.resHeightInPixels, BMP_BITS_PER_PIXEL);
#endif

#if DISPLAY_WINDOW == 1

	//make a MS Window
	const char* const myClass = "myclass";

	/*
	- https://stackoverflow.com/questions/1748470/how-to-draw-image-on-a-window
	*/
	WNDCLASSEX wndClassEx;

	wndClassEx.cbSize = sizeof(WNDCLASSEX);
	wndClassEx.style = CS_HREDRAW | CS_VREDRAW;
	wndClassEx.lpfnWndProc = WndProc;
	wndClassEx.cbClsExtra = 0;
	wndClassEx.cbWndExtra = 0;
	wndClassEx.hInstance = GetModuleHandle(0);
	wndClassEx.hIcon = LoadIcon(0, IDI_APPLICATION);
	wndClassEx.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClassEx.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wndClassEx.lpszMenuName = NULL;
	wndClassEx.lpszClassName = myClass;
	wndClassEx.hIconSm = LoadIcon(wndClassEx.hInstance, IDI_APPLICATION);

	if (RegisterClassEx(&wndClassEx)) {

		HWND window = CreateWindowEx(
			0,
			myClass,
			"Ray Trace In One Weekend",
			WS_OVERLAPPEDWINDOW,
			800,
			600,
			renderProps.resWidthInPixels,
			renderProps.resHeightInPixels,
			0,
			0,
			GetModuleHandle(0),
			0
		);

		if (window) {
			ShowWindow(window, SW_SHOWDEFAULT);

			MSG msg;
			bool status;
			while (status = GetMessage(&msg, 0, 0, 0) != 0) {
				if (status == -1) {
					//TODO: something went wrong (i.e. invalid memory read for message??), so through an error and exit
					std::cout << "An error occured when calling GetMessage()\n";
					return -1;
				}
				else {
					DispatchMessage(&msg);
				}
			}
		}
	}

#endif

	delete[] world;
		
	// putting a \n triggers cin...?
	// drowan(20190607) BUG: For some reason if the rendered scene is small (10x10 pixels) 
	// the cin.get() is completely bypassed unless I put a cin.ignore(). Is the thread exiting
	// weird relative to the main thread???
	std::cout << "Hit any key to exit...";
	//std::cout.flush();
	//std::cin.ignore(INT_MAX, '\n');
	std::cin.get();

	return 0;
}

/*
- https://docs.microsoft.com/en-us/cpp/windows/walkthrough-creating-windows-desktop-applications-cpp?view=vs-2019
- https://www.gamedev.net/forums/topic/608057-how-do-you-create-a-win32-window-from-console-application/
- http://blog.airesoft.co.uk/2010/10/a-negative-experience-in-getting-messages/
- https://stackoverflow.com/questions/22819003/how-to-set-the-colors-of-a-windows-pixels-with-windows-api-c-once-created
- https://stackoverflow.com/questions/1748470/how-to-draw-image-on-a-window
- https://stackoverflow.com/questions/6423729/get-current-cursor-position
*/
LRESULT CALLBACK WndProc(
	_In_ HWND hwnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
) {
	//TODO drowan(20190704): Reading the cursor here is probably not best practice. Look into how to do this.
	POINT p;

	if (GetCursorPos(&p)) {
		if (ScreenToClient(hwnd, &p)) {
			if (p.x >= 0 && p.y >= 0) {
				std::cout << "\nMousepoint " << p.x << ", " << p.y << "\n";
			}
		}
	}
	
	switch (uMsg) {

		case WM_CREATE:		
			hBitmap = (HBITMAP)LoadImage(NULL, ".\\test.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
			return 0L;

		case WM_PAINT:
			
			PAINTSTRUCT ps;
			HDC hdc;
			BITMAP bitmap;
			HDC hdcMem;
			HGDIOBJ oldBitmap;

			hdc = BeginPaint(hwnd, &ps);

			hdcMem = CreateCompatibleDC(hdc);
			oldBitmap = SelectObject(hdcMem, hBitmap);

			GetObject(hBitmap, sizeof(bitmap), &bitmap);
			BitBlt(hdc, 0, 0, bitmap.bmWidth, bitmap.bmHeight, hdcMem, 0, 0, SRCCOPY);

			SelectObject(hdcMem, oldBitmap);
			DeleteDC(hdcMem);

			EndPaint(hwnd, &ps);
			
#if 0
			//try setting some pixels
			HDC hdcPixel;

			hdcPixel = GetDC(hwnd);
			for (int x = 0; x < 10; x++) {
				for (int y = 0; y < 10; y++) {
					SetPixel(hdcPixel, x + p.x, y + p.y, RGB(255, 0, 0));
				}
			}

			DeleteDC(hdcPixel);
#endif
			return 0L;

		case WM_DESTROY:
			std::cout << "\nClosing window...\n";

			PostQuitMessage(0);

			return 0L;

		case WM_LBUTTONDOWN:
			std::cout << "\nLeft Mouse Button Down " << LOWORD(lParam) << "," << HIWORD(lParam) << "\n";									

			//ask to redraw the window
			RedrawWindow(hwnd, NULL, NULL, RDW_INTERNALPAINT);

		case WM_LBUTTONDBLCLK:
			std::cout << "\nLeft Mouse Button Click " << LOWORD(lParam) << "," << HIWORD(lParam) << "\n";			

		default:
			//std::cout << "\nUnhandled WM message\n";

			return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}

void configureScene(RenderProperties &renderProps) {

	renderProps.resHeightInPixels = DEFAULT_RENDER_HEIGHT;
	renderProps.resWidthInPixels = DEFAULT_RENDER_WIDTH;
	renderProps.antiAliasingSamplesPerPixel = DEFAULT_RENDER_AA;

#if BYPASS_SCENE_CONFIG == 0
	//ask for image dimensions
	std::cout << "Enter render width: ";	
	std::cout.flush();
	std::cin.clear();
	//std::cin.ignore(INT_MAX, '\n');

	if (std::cin.peek() == '\n') {
		std::cout << "Invalid input, using default: " << renderProps.resWidthInPixels << '\n';
	}	
	else {
		std::cin >> renderProps.resWidthInPixels;

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
	}

	std::cout << "Enter render height: ";
	std::cout.flush();
	std::cin.clear();
	std::cin.ignore(INT_MAX, '\n');

	if (std::cin.peek() == '\n') {
		std::cout << "Invalid input, using default: " << renderProps.resHeightInPixels << '\n';
	}
	else {
		std::cin >> renderProps.resHeightInPixels;

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
	}

	std::cout << "Enter number of anti-aliasing samples (also helps increase photon count): ";
	std::cout.flush();
	std::cin.clear();
	std::cin.ignore(INT_MAX, '\n');

	if (std::cin.peek() == '\n') {
		std::cout << "Invalid input, using default: " << renderProps.antiAliasingSamplesPerPixel << '\n';
	}
	else {
		std::cin >> renderProps.antiAliasingSamplesPerPixel;

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
	}
#endif
}

// going to try and pass a buffer per thread and combine afterwards so as to avoid memory contention when using a mutex which may slow things down...
void workerThreadFunction(
	std::shared_ptr<WorkerThread> workerThreadStruct, 
	std::shared_ptr<WorkerImageBuffer> workerImageBufferStruct, 	
	RenderProperties renderProps, 
	Camera sceneCamera, 
	Hitable *world,
	std::mutex *coutGuard) {

	//DEBUG drowan(20190704): pretty sure this is not safe to have multiple threads accessing the canvas without a mutex
	HDC hdcPixel;

	hdcPixel = GetDC(0);

	int numOfThreads = DEBUG_RUN_THREADS; //std::thread::hardware_concurrency();

	std::unique_lock<std::mutex> coutLock(*coutGuard);

	std::cout << "\nThread ID: " << workerThreadStruct->id  << "\n";
	std::cout << "Lookat: " << sceneCamera.getLookAt() << "\n";
	std::cout << "World hitable address:  " << world << "\n";
	std::cout << "Image buffer address: " << &workerImageBufferStruct << " @[0]: " << workerImageBufferStruct->buffer.get()[0] << " Size in bytes: " << workerImageBufferStruct->sizeInBytes << "\n";
	std::cout << "Waiting for start...\n";

	coutLock.unlock();

	std::unique_lock<std::mutex> startLock(workerThreadStruct->startMutex);
	while (!workerThreadStruct->start) {
		workerThreadStruct->startConditionVar.wait(startLock);
	}

	coutLock.lock();
	std::cout << "Thread " << workerThreadStruct->id << " starting...\n";
	coutLock.unlock();

	uint32_t rowOffsetInPixels = 0;

#if RUN_RAY_TRACE == 1
	if (workerThreadStruct->id == numOfThreads - 1) {
		rowOffsetInPixels = static_cast<uint32_t>(workerThreadStruct->id * (renderProps.resHeightInPixels / numOfThreads));
		rowOffsetInPixels += renderProps.resHeightInPixels%numOfThreads;
	}
	else {
		rowOffsetInPixels = workerThreadStruct->id * workerImageBufferStruct->resHeightInPixels;
	}

	for (int row = workerImageBufferStruct->resHeightInPixels - 1; row >= 0; row--) {
		for (int column = 0; column < workerImageBufferStruct->resWidthInPixels; column++) {

			vec3 outputColor(0, 0, 0);
			//loop to produce AA samples
			for (int sample = 0; sample < renderProps.antiAliasingSamplesPerPixel; sample++) {

				float u = (float)(column + unifRand(randomNumberGenerator)) / (float)workerImageBufferStruct->resWidthInPixels;
				float v = ((float)row + rowOffsetInPixels + unifRand(randomNumberGenerator)) / (float)renderProps.resHeightInPixels;

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

			//DEBUG drowan(20190704): render to window expirment. This is rendering to the "desktop" because I don't have the window handle in here. I need to
			//put the MS Windows message loop thread in it's own thread.
			SetPixel(hdcPixel, row, column, RGB(ir, ig, ib));

#if 1
			//also store values into tempBuffer
			uint32_t bufferIndex = row * renderProps.resWidthInPixels * renderProps.bytesPerPixel + (column * renderProps.bytesPerPixel);
			workerImageBufferStruct->buffer.get()[bufferIndex] = ib;
			workerImageBufferStruct->buffer.get()[bufferIndex + 1] = ig;
			workerImageBufferStruct->buffer.get()[bufferIndex + 2] = ir;
#endif
		}
	}
#endif

	//indicate that ray tracing is complete	
	std::unique_lock<std::mutex> doneLock(workerThreadStruct->workIsDoneMutex);
	workerThreadStruct->workIsDone = true;
	workerThreadStruct->workIsDoneConditionVar.notify_all();
	doneLock.unlock();

	coutLock.lock();
	std::cout << "\nRaytracing worker " << workerThreadStruct->id << " finished!\n";
	coutLock.unlock();

	//wait for exit
	std::unique_lock<std::mutex> exitLock(workerThreadStruct->exitMutex);
	while (!workerThreadStruct->exit) {
		workerThreadStruct->exitConditionVar.wait(exitLock);
	}	

	return;
}