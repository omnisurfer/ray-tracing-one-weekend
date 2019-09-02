#include <fstream>
#include "float.h"

#include "defines.h"
#include "vec3.h"
#include "hitableList.h"
#include "camera.h"
#include "color.h"
#include "scenes.h"
#include "common.h"
#include "winGUI.h"

#include "debug.h"

#include "winDIBbitmap.h"

//https://github.com/nothings/stb
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

/* 
	Look into:
	- drowan 20190601: https://eli.thegreenplace.net/2016/c11-threads-affinity-and-hyperthreading/
	- drowan 20190607: Use OpenMPI???
*/

/* 
* https://github.com/petershirley/raytracinginoneweekend
* http://jamie-wong.com/2016/07/15/ray-marching-signed-distance-functions/
* http://www.codinglabs.net/article_world_view_projection_matrix.aspx
* http://iquilezles.org/index.html
*/

void raytraceWorkerProcedure(
	std::shared_ptr<WorkerThread> workerThread,
	std::shared_ptr<WorkerImageBuffer> workerImageBuffer,
	RenderProperties renderProps,
	Camera sceneCamera,
	Hitable *world
);

void configureScene(RenderProperties &renderProps);

void bitBlitWorkerProcedure(
	std::shared_ptr<WorkerThread> workerThreadStruct,
	std::shared_ptr<WorkerImageBuffer> workerImageBufferStruct,
	RenderProperties renderProps
);

Hitable *randomScene();
Hitable *cornellBox();

/*
Start -> 
Render scene -> spawn or unpause worker threads > wait until done > pause worker threads
Blit to GUI -> spawn or unpause worker thread > wait until done > pause worker thread
Wait for input -> ?
Return to [Render scene]
*/

int main() {
	
	std::unique_lock<std::mutex> coutLock(globalCoutGuard);
	coutLock.unlock();

	DEBUG_MSG_L0(__func__, "");

	//Setup random number generator
	timeSeed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	std::seed_seq seedSequence{
			uint32_t(timeSeed & 0xffffffff),
			uint32_t(timeSeed >> 32)
	};

	randomNumberGenerator.seed(seedSequence);	
	
	uint32_t numOfRenderThreads = DEBUG_RUN_THREADS; // std::thread::hardware_concurrency();
	DEBUG_MSG_L0("\t", "used hardware threads: " << numOfRenderThreads << "\n");

	WINDIBBitmap winDIBBmp;
	RenderProperties renderProps;

	configureScene(renderProps);

	renderProps.bytesPerPixel = (winDIBBmp.getBitsPerPixel() / 8);
	renderProps.finalImageBufferSizeInBytes = renderProps.resWidthInPixels * renderProps.resHeightInPixels * renderProps.bytesPerPixel;

	//Setup camera
	vec3 lookFrom(0, 0, 0);
	vec3 lookAt(0, 1, 0);
	vec3 worldUp(0, 1, 0);
	float distToFocus = 1000;//(lookFrom - lookAt).length(); //1000
	float aperture = 2.0;
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
	mainCamera.setLookFrom(vec3(278, 278, -500));
	mainCamera.setLookAt(vec3(278, 278, 0));

	Hitable *world = cornellBox();
#endif

	// Each thread will have a handle to this shared buffer but will access the memory with a thread specific memory offset which will hopefully mitigate concurrent access issues.
	std::shared_ptr<WorkerImageBuffer> workerImageBufferStruct(new WorkerImageBuffer);

	//figure out how many rows each thread is going to work on
	workerImageBufferStruct->resHeightInPixels = renderProps.resHeightInPixels;
	workerImageBufferStruct->resWidthInPixels = renderProps.resWidthInPixels;
	workerImageBufferStruct->sizeInBytes = workerImageBufferStruct->resHeightInPixels * workerImageBufferStruct->resWidthInPixels * renderProps.bytesPerPixel;

	std::shared_ptr<uint8_t> _workingImageBuffer(new uint8_t[workerImageBufferStruct->sizeInBytes]);

	workerImageBufferStruct->buffer = std::move(_workingImageBuffer);

#pragma region Init_Threads	
	//gui
#if DISPLAY_WINDOW == 1

	std::shared_ptr<WorkerThread> guiWorkerThread(new WorkerThread);

	guiWorkerThread->id = 0;
	guiWorkerThread->workIsDone = false;
	guiWorkerThread->start = false;
	guiWorkerThread->continueWork = false;
	guiWorkerThread->exit = false;
	guiWorkerThread->handle = std::thread(guiWorkerProcedure, guiWorkerThread, renderProps.resWidthInPixels, renderProps.resHeightInPixels);
#endif
	//bitblit
#if DEBUG_BITBLIT == 1

	std::shared_ptr<WorkerThread> bitBlitWorkerThread(new WorkerThread);

	bitBlitWorkerThread->id = 0;
	bitBlitWorkerThread->workIsDone = false;
	bitBlitWorkerThread->start = false;
	bitBlitWorkerThread->continueWork = false;
	bitBlitWorkerThread->exit = false;
	bitBlitWorkerThread->handle = std::thread(bitBlitWorkerProcedure, bitBlitWorkerThread, workerImageBufferStruct, renderProps);
#endif

	//render
	std::vector<std::shared_ptr<WorkerImageBuffer>> workerImageBufferVector;
	std::vector<std::shared_ptr<WorkerThread>> workerThreadVector;
	std::shared_ptr<uint8_t> finalImageBuffer(new uint8_t[renderProps.finalImageBufferSizeInBytes]);

	for (int i = 0; i < numOfRenderThreads; i++) {

		std::shared_ptr<WorkerThread> workerThread(new WorkerThread);

		workerThread->id = i;
		workerThread->workIsDone = false;
		workerThread->start = false;
		workerThread->continueWork = false;
		workerThread->exit = false;
		workerThread->handle = std::thread(raytraceWorkerProcedure, workerThread, workerImageBufferStruct, renderProps, mainCamera, world);

		workerThreadVector.push_back(workerThread);
	}

#pragma endregion Init_Threads

#pragma region Start_Threads
	//gui
#if DISPLAY_WINDOW == 1
	std::unique_lock<std::mutex> startLock(guiWorkerThread->startMutex);
	guiWorkerThread->start = true;
	guiWorkerThread->startConditionVar.notify_all();
	startLock.unlock();

	//debug wait for the gui to start
	Sleep(4000);
#endif

	//render
	for (std::shared_ptr<WorkerThread> &thread : workerThreadVector) {
		std::unique_lock<std::mutex> startLock(thread->startMutex);
		thread->start = true;
		thread->startConditionVar.notify_all();
		startLock.unlock();
	}
	
	//bitblit
#if DEBUG_BITBLIT == 1
	std::unique_lock<std::mutex> startBitBlitLock(bitBlitWorkerThread->startMutex);
	bitBlitWorkerThread->start = true;
	bitBlitWorkerThread->startConditionVar.notify_all();
	startBitBlitLock.unlock();
#endif

#pragma endregion Start_Threads

#pragma region Manage_Threads

	for (int i = 0; i < 100; i++) {

		//check if render is done
		for (std::shared_ptr<WorkerThread> &thread : workerThreadVector) {

			std::unique_lock<std::mutex> doneLock(thread->workIsDoneMutex);			
			while (!thread->workIsDone) {
				thread->workIsDoneConditionVar.wait(doneLock);				
			}
			//reset the workIsDone indicator to acknowledge
			thread->workIsDone = false;
			doneLock.unlock();			
		}

		//bitblit
#if DEBUG_BITBLIT == 1		
		std::unique_lock<std::mutex> bitBlitContinueLock(bitBlitWorkerThread->continueWorkMutex);
		bitBlitWorkerThread->continueWork = true;
		bitBlitWorkerThread->continueWorkConditionVar.notify_all();
		bitBlitContinueLock.unlock();

		std::unique_lock<std::mutex> bitBlitDoneLock(bitBlitWorkerThread->workIsDoneMutex);
		while (!bitBlitWorkerThread->workIsDone) {
			bitBlitWorkerThread->workIsDoneConditionVar.wait(bitBlitDoneLock);
		}
		//reset the workdIsDone indicator to acknowledge
		bitBlitWorkerThread->workIsDone = false;
		bitBlitDoneLock.unlock();
#endif
		
		//start the render threads again
		for (std::shared_ptr<WorkerThread> &thread : workerThreadVector) {

			std::unique_lock<std::mutex> continueLock(thread->continueWorkMutex);
			thread->continueWork = true;
			thread->continueWorkConditionVar.notify_all();
			continueLock.unlock();
		}
	}	


#pragma endregion Manage_Threads

#pragma region Stop_Threads
	//gui
	//wait for the threads
	for (std::shared_ptr<WorkerThread> &thread : workerThreadVector) {

		std::unique_lock<std::mutex> doneLock(thread->workIsDoneMutex);
		while (!thread->workIsDone) {
			thread->workIsDoneConditionVar.wait(doneLock);			
		}		
		doneLock.unlock();		

		std::unique_lock<std::mutex> continueLock(thread->continueWorkMutex);
		thread->continueWork = false;
		thread->continueWorkConditionVar.notify_all();		
		continueLock.unlock();
	}

	//join threads
	for (std::shared_ptr<WorkerThread> &thread : workerThreadVector) {
		//signal to the thread to exit		

		std::unique_lock<std::mutex> threadExitLock(thread->exitMutex);
		thread->exit = true;
		thread->exitConditionVar.notify_all();
		threadExitLock.unlock();

		while(!thread->handle.joinable()) {
			//sit and spin
		}
		thread->handle.join();
	}

	//bitblit
#if DEBUG_BITBLIT == 1
	//exit the bitblit thread
	/*
	std::unique_lock<std::mutex> bitBlitDoneLock(bitBlitWorkerThread->workIsDoneMutex);
	while (!bitBlitWorkerThread->workIsDone) {
		bitBlitWorkerThread->workIsDoneConditionVar.wait(bitBlitDoneLock);
	}
	bitBlitDoneLock.unlock();
	/**/
	std::unique_lock<std::mutex> bitBlitContinueLock(bitBlitWorkerThread->continueWorkMutex);
	bitBlitWorkerThread->continueWork = false;
	bitBlitWorkerThread->continueWorkConditionVar.notify_all();
	bitBlitContinueLock.unlock();

	std::unique_lock<std::mutex> bitBlitExitLock(bitBlitWorkerThread->exitMutex);
	bitBlitWorkerThread->exit = true;
	bitBlitWorkerThread->exitConditionVar.notify_all();
	bitBlitExitLock.unlock();
	
	while (!bitBlitWorkerThread->handle.joinable()) {
		//sit and spin
	}
	bitBlitWorkerThread->handle.join();

#endif

#if DISPLAY_WINDOW == 1
	//exit the GUI
	std::unique_lock<std::mutex> guiWorkerExitLock(guiWorkerThread->exitMutex);
	guiWorkerThread->exit = true;
	guiWorkerThread->exitConditionVar.notify_all();
	guiWorkerExitLock.unlock();

	std::unique_lock<std::mutex> guiDoneLock(guiWorkerThread->workIsDoneMutex);
	while (!guiWorkerThread->workIsDone) {
		guiWorkerThread->workIsDoneConditionVar.wait(guiDoneLock);
	}
	guiDoneLock.unlock();

	while(!guiWorkerThread->handle.joinable()) {
		//sit and spin
	}
	guiWorkerThread->handle.join();
#endif

#pragma endregion Stop_Threads

#if OUTPUT_BMP_EN == 1
	std::cout << "Writing to bmp file...\n";

	winDIBBmp.writeBMPToFile(workerImageBufferStruct->buffer.get(), renderProps.finalImageBufferSizeInBytes, renderProps.resWidthInPixels, renderProps.resHeightInPixels, BMP_BITS_PER_PIXEL);

#endif	

	// drowan(20190607) BUG: For some reason if the rendered scene is small (10x10 pixels)
	std::cout << "Hit any key to exit...";
	//std::cout.flush();
	//std::cin.ignore(INT_MAX, '\n');
	std::cin.get();

	delete[] world;

	return 0;
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

void raytraceWorkerProcedure(
	std::shared_ptr<WorkerThread> workerThreadStruct,
	std::shared_ptr<WorkerImageBuffer> workerImageBufferStruct,
	RenderProperties renderProps,
	Camera sceneCamera,
	Hitable *world
) {

	std::unique_lock<std::mutex> exitLock(workerThreadStruct->exitMutex);
	exitLock.unlock();

	std::unique_lock<std::mutex> continueLock(workerThreadStruct->continueWorkMutex);
	continueLock.unlock();

	std::unique_lock<std::mutex> coutLock(globalCoutGuard);
	coutLock.unlock();

	std::unique_lock<std::mutex> startLock(workerThreadStruct->startMutex);
	while (!workerThreadStruct->start) {
		workerThreadStruct->startConditionVar.wait(startLock, [workerThreadStruct] {return workerThreadStruct->start == true; });
	}
	startLock.unlock();

	//DEBUG drowan(20190704): pretty sure this is not safe to have multiple threads accessing the canvas without a mutex
	HDC hdcRayTraceWindow;

	hdcRayTraceWindow = GetDC(raytraceMSWindowHandle);
		
	int numOfThreads = DEBUG_RUN_THREADS; //std::thread::hardware_concurrency();	
	
	DEBUG_MSG_L0(__func__, 
		"worker " << workerThreadStruct->id <<
		"\n\tHwnd: " << raytraceMSWindowHandle <<
		"\n\tThread ID: " << workerThreadStruct->id <<
		"\n\tLookat: " << sceneCamera.getLookAt() << 
		"\n\tWorld hitable address:  " << world <<
		"\n\tImage buffer address: " << &workerImageBufferStruct << 
		" @[0]: " << workerImageBufferStruct->buffer.get()[0] << " Size in bytes: " << workerImageBufferStruct->sizeInBytes		
	);

	uint32_t rowOffsetInPixels = 0;

#if RUN_RAY_TRACE == 1
	while (true) {

		/* # of Threads = 4
		T1 (n + t*i):		0, 4, 8
		T2 (n+1 + t*i):		1, 5, 9
		T3 (n+2 + t*i):		2, 6, 10
		T4 (n+3 + t*i):		3, 7, 11
	*/
		for (int row = workerImageBufferStruct->resHeightInPixels - 1; row >= 0; row--) {
			//for (int row = 0; row < workerImageBufferStruct->resHeightInPixels; row++) {
			for (int i = 0; i < workerImageBufferStruct->resWidthInPixels; i++) {

				int column = workerThreadStruct->id + numOfThreads * i;

				if (column < workerImageBufferStruct->resWidthInPixels) {
					vec3 outputColor(0, 0, 0);
					//loop to produce AA samples
					for (int sample = 0; sample < renderProps.antiAliasingSamplesPerPixel; sample++) {

						float u = (float)(column + unifRand(randomNumberGenerator)) / (float)workerImageBufferStruct->resWidthInPixels;
						float v = (float)(row + unifRand(randomNumberGenerator)) / (float)workerImageBufferStruct->resHeightInPixels;

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

					//Seems OK with multiple thread access. Or at least can't see any obvious issues.
					// Look into replacing this since it is pretty slow:
					// https://stackoverflow.com/questions/26005744/how-to-display-pixels-on-screen-directly-from-a-raw-array-of-rgb-values-faster-t
#if DISPLAY_WINDOW == 1 && DEBUG_SET_PIXEL == 1
				//SetPixel is really slow on my laptop. Maybe GPU bound as CPU only loads to ~40%. Without it, can reach 100%
				//For WinAPI look into Lockbits
				//SetPixel(hdcRayTraceWindow, column, renderProps.resHeightInPixels - row, RGB(ir, ig, ib));				
#endif

#if 1			
					uint32_t rowIndex = row * renderProps.resWidthInPixels * renderProps.bytesPerPixel;
					uint32_t columnIndex = (renderProps.resWidthInPixels * renderProps.bytesPerPixel) - column * renderProps.bytesPerPixel;
					uint32_t bufferIndex = workerImageBufferStruct->sizeInBytes - (rowIndex + columnIndex);
					workerImageBufferStruct->buffer.get()[bufferIndex] = ib;
					workerImageBufferStruct->buffer.get()[bufferIndex + 1] = ig;
					workerImageBufferStruct->buffer.get()[bufferIndex + 2] = ir;
					//alpha channel for now is just 0
					workerImageBufferStruct->buffer.get()[bufferIndex + 3] = 0;
#endif
				}
				else {
					break;
				}
			}
		}

		//indicate that ray tracing is complete	
		std::unique_lock<std::mutex> doneLock(workerThreadStruct->workIsDoneMutex);
		workerThreadStruct->workIsDone = true;
		workerThreadStruct->workIsDoneConditionVar.notify_all();
		doneLock.unlock();

		//check if we need to continue rendering
		continueLock.lock();
		DEBUG_MSG_L0(__func__, "worker " << workerThreadStruct->id << " waiting for continue notice");
		workerThreadStruct->continueWorkConditionVar.wait(continueLock);
		DEBUG_MSG_L0(__func__, "worker " << workerThreadStruct->id << " got continue notice...");
		if (workerThreadStruct->continueWork) {
			//continue
			continueLock.unlock();
		}
		else {
			continueLock.unlock();
			break;
		}
	}
#endif
	
	//check for exit
	exitLock.lock();
	DEBUG_MSG_L0(__func__, "worker " << workerThreadStruct->id << " waiting for exit notice");
	workerThreadStruct->exitConditionVar.wait(exitLock, [workerThreadStruct] {return workerThreadStruct->exit == true; });
	DEBUG_MSG_L0(__func__, "worker " << workerThreadStruct->id << " exiting...");

	DeleteDC(hdcRayTraceWindow);

	return;
}

void bitBlitWorkerProcedure(
	std::shared_ptr<WorkerThread> workerThreadStruct,
	const std::shared_ptr<WorkerImageBuffer> workerImageBufferStruct,
	const RenderProperties renderProps
) {
	std::unique_lock<std::mutex> coutLock(globalCoutGuard);
	coutLock.unlock();

	std::unique_lock<std::mutex> exitLock(workerThreadStruct->exitMutex);
	exitLock.unlock();

	std::unique_lock<std::mutex> continueLock(workerThreadStruct->continueWorkMutex);
	continueLock.unlock();

	std::unique_lock<std::mutex> doneLock(workerThreadStruct->workIsDoneMutex);
	doneLock.unlock();

	std::unique_lock<std::mutex> startLock(workerThreadStruct->startMutex);
	while (!workerThreadStruct->start) {
		workerThreadStruct->startConditionVar.wait(startLock, [workerThreadStruct] {return workerThreadStruct->start == true; });
	}
	startLock.unlock();	

	DEBUG_MSG_L0(__func__, "starting...");

	uint64_t counter = 0;

	HBITMAP newBitmap = 0;

	while (true) {

		counter++;

		//check if we need to continue blitting
		continueLock.lock();
		DEBUG_MSG_L0(__func__, "worker " << workerThreadStruct->id << " waiting for continue notice");
		workerThreadStruct->continueWorkConditionVar.wait(continueLock);
		DEBUG_MSG_L0(__func__, "worker " << workerThreadStruct->id << " continuing...");
		if (workerThreadStruct->continueWork) {
			//continue						
			continueLock.unlock();

			//check if need to delete the bitmap
			//helps a little. probably need to pass bytes and array length throuhg user LPARAM and leave
			//bitmap stuff to the gui thread
			if (newBitmap) {
				DeleteObject(newBitmap);
			}
		}
		else {
			continueLock.unlock();
			break;
		}

#if 1		
		//https://stackoverflow.com/questions/26011437/c-trouble-with-making-a-bitmap-from-scratch
		//Attempting to get bitmap working. Noticed that my bufferTest will create a valid
		//bitmap when I set the bit depth to 32 instead of 24.
		//May require that I have an "alpha" to get byte alignment correct	

		//2019-09-01: Check this out about color bitmaps:
		//https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-createbitmap
		newBitmap = CreateBitmap(
			renderProps.resWidthInPixels,
			renderProps.resHeightInPixels,
			1,
			renderProps.bytesPerPixel * 8,
			workerImageBufferStruct->buffer.get()
		);

		//DEBUG! After about 10K iterations, bitmap creation fails and the system stalls
		if (!newBitmap) {
			DEBUG_MSG_L0(__func__, "worker " << workerThreadStruct->id << " bitmap creation failed@ " << counter);
			DEBUG_MSG_L0(__func__, "last error: " << GetLastError());
			break;
		}

		PostMessage(raytraceMSWindowHandle, WM_USER, 0, (LPARAM)newBitmap);

#endif		
		//indicate that blitting is complete	
		doneLock.lock();
		workerThreadStruct->workIsDone = true;
		workerThreadStruct->workIsDoneConditionVar.notify_all();
		doneLock.unlock();		
	}
		
	exitLock.lock();
	/*
		https://en.cppreference.com/w/cpp/thread/condition_variable/wait
		https://stackoverflow.com/questions/30217956/error-variable-cannot-be-implicitly-captured-because-no-default-capture-mode-h
	*/	
	DEBUG_MSG_L0(__func__, "worker " << workerThreadStruct->id << " waiting for exit notice");	
	workerThreadStruct->exitConditionVar.wait(exitLock, [workerThreadStruct]{return workerThreadStruct->exit == true; });
	DEBUG_MSG_L0(__func__, "worker " << workerThreadStruct->id << " exiting...");
}