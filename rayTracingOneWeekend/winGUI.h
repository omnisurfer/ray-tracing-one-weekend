#pragma once
#include <Windows.h>
#include <iostream>

#include "common.h"
#include "debug.h"

HWND raytraceMSWindowHandle;
HBITMAP workingBitmap;

int globalMouseCurrentX = 0, globalMouseCurrentY = 0;
int globalMouseLastX = 0, globalMouseLastY = 0;
int globalMouseDeltaX = 0, globalMouseDeltaY = 0;
std::mutex globalMouseXYInputMutex;

int globalWindowHeight = 0, globalWindowWidth = 0;

bool globalGuiIsRunning = false;
std::mutex globalGuiIsRunningMutex;

GUIControlInputs _guiControlInputs;

//more sophisticated FPS method:
//https://stackoverflow.com/questions/28530798/how-to-make-a-basic-fps-counter
clock_t currentFrameTimeSample;
clock_t lastFrameTimeSample;

int guiWorkerProcedure(
	std::shared_ptr<WorkerThread> workerThreadStruct,
	uint32_t windowWidth,
	uint32_t windowHeight);

LRESULT CALLBACK WndProc(
	_In_ HWND hwnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
);

int getMouseCoord(int &x, int &y) {
	
	std::lock_guard<std::mutex> lock(globalMouseXYInputMutex);		
	x = globalMouseCurrentX - globalWindowWidth / 2;
	y = globalMouseCurrentY - globalWindowHeight / 2;

	return 0;
}

bool getGUIControlInputs(GUIControlInputs &guiControlInputs) {

	guiControlInputs = _guiControlInputs;

	return true;
}

bool checkIfGuiIsRunning() {

	bool lockState;

	std::lock_guard<std::mutex> guiRunningLock(globalGuiIsRunningMutex);
	lockState = globalGuiIsRunning;	

	return lockState;
}

int guiWorkerProcedure(
	std::shared_ptr<WorkerThread> workerThreadStruct,
	uint32_t windowWidth,
	uint32_t windowHeight) {

	std::unique_lock<std::mutex> coutLock(globalCoutGuard);
	coutLock.unlock();
		
	std::unique_lock<std::mutex> exitLock(workerThreadStruct->exitMutex);
	exitLock.unlock();

	std::unique_lock<std::mutex> guiRunningLock(globalGuiIsRunningMutex);
	globalGuiIsRunning = true;
	guiRunningLock.unlock();

	//set the clock samples to the same time
	currentFrameTimeSample = clock();
	lastFrameTimeSample = currentFrameTimeSample;

	//wait to be told to run
	std::unique_lock<std::mutex> startLock(workerThreadStruct->startMutex);
	while (!workerThreadStruct->start) {
		workerThreadStruct->startConditionVar.wait(startLock, [workerThreadStruct] {return workerThreadStruct->start == true; });
	}
	/*
- https://stackoverflow.com/questions/1748470/how-to-draw-image-on-a-window
*/
	WNDCLASSEX wndClassEx;

	//make a MS Window
	const char* const myClass = "raytrace_MSwindow";

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

		//http://www.directxtutorial.com/Lesson.aspx?lessonid=11-1-4
		//figure out how big to make the whole window
		RECT rect;
		rect = { 0, 0, (LONG)windowWidth, (LONG)windowHeight };		

		globalWindowHeight = windowHeight;
		globalWindowWidth = windowWidth;

		BOOL result = AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

		if (result) {
			DEBUG_MSG_L0(__func__, " X Adj: " << rect.right << " Y Adj: " << rect.bottom);
		}

		//maybe use lParam to pass a handle to the image buffer?
		raytraceMSWindowHandle = CreateWindowEx(
			0,
			myClass,
			"Ray Trace In One Weekend",
			WS_OVERLAPPEDWINDOW,
			100,
			100,
			rect.right - rect.left,
			rect.bottom - rect.top,
			0,
			0,
			GetModuleHandle(0),
			0
		);

		/*
			How to get mouse inputs from the GUI
			https://docs.microsoft.com/en-us/windows/win32/inputdev/using-mouse-input
		*/
		
		if (raytraceMSWindowHandle) {
			ShowWindow(raytraceMSWindowHandle, SW_SHOWDEFAULT);

			MSG msg;
			bool status;			
			while (status = GetMessage(&msg, 0, 0, 0) != 0) {
				// drowan: the example I found promoted bool to an int...?
				if (status == -1) {
					//TODO: something went wrong (i.e. invalid memory read for message??), so throw an error and exit
					DEBUG_MSG_L0(__func__, " An error occured when calling GetMessage()");
					return -1;
				}
				else {
					DispatchMessage(&msg);
				}				
			}
			DEBUG_MSG_L0(__func__, "Exiting Window message loop...");
		}

		DestroyWindow(raytraceMSWindowHandle);

		guiRunningLock.lock();
		globalGuiIsRunning = false;
		guiRunningLock.unlock();
	}
	
	std::unique_lock<std::mutex> doneLock(workerThreadStruct->workIsDoneMutex);
	workerThreadStruct->workIsDone = true;
	workerThreadStruct->workIsDoneConditionVar.notify_all();
	doneLock.unlock();

	DEBUG_MSG_L0(__func__, "worker " << workerThreadStruct->id << " done");

	exitLock.lock();
	DEBUG_MSG_L0(__func__, "worker " << workerThreadStruct->id << " waiting for exit notice");
	workerThreadStruct->exitConditionVar.wait(exitLock, [workerThreadStruct] {return workerThreadStruct->exit == true; });
	DEBUG_MSG_L0(__func__, "worker " << workerThreadStruct->id << " exiting...");

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
	
	std::unique_lock<std::mutex> coutLock(globalCoutGuard);
	coutLock.unlock();	

	/*
	- https://docs.microsoft.com/en-us/windows/win32/gdi/using-brushes
	- https://docs.microsoft.com/en-us/windows/win32/gdi/drawing-a-custom-window-background
	*/
	
	//TODO drowan(20190704): Reading the cursor here is probably not best practice. Look into how to do this.
	POINT p;

	if (GetCursorPos(&p)) {
		if (ScreenToClient(hwnd, &p));
	}

	switch (uMsg) {

		/**/
		case WM_KEYDOWN: {			
			
			switch (wParam) {

				case 'W': {
					_guiControlInputs.forwardAsserted = true;
					break;
				}

				case 'S': {
					_guiControlInputs.reverseAsserted = true;
					break;
				}

				case 'A': {
					_guiControlInputs.leftAsserted = true;
					break;
				}

				case 'D': {
					_guiControlInputs.rightAsserted = true;
					break;
				}

				case VK_ESCAPE: {
					_guiControlInputs.escAsserted = true;
					break;
				}

				default: {
					break;
				}

			}

			return 0L;
		}

		case WM_KEYUP: {

			switch (wParam) {

				case 'W': {
					_guiControlInputs.forwardAsserted = false;
					break;
				}

				case 'S': {
					_guiControlInputs.reverseAsserted = false;
					break;
				}

				case 'A': {
					_guiControlInputs.leftAsserted = false;
					break;
				}

				case 'D': {
					_guiControlInputs.rightAsserted = false;
					break;
				}

				case VK_ESCAPE: {
					_guiControlInputs.escAsserted = false;
					break;
				}

				default: {
					break;
				}

			}

			return 0L;
		}
		/**/
		case WM_SYSKEYDOWN: {
		
			switch (wParam) {

				default: {
					break;
				}

			}
			return 0L;
		}

		case WM_SYSKEYUP: {

			switch (wParam) {

				default: {
					break;
				}

			}
			return 0L;
		}

		case WM_MOUSEMOVE: {
					
			if (p.x >= 0 && p.y >= 0) {
				//std::cout << "\nMousepoint " << p.x << ", " << p.y << "\n";
			}
			
			std::lock_guard<std::mutex> lock(globalMouseXYInputMutex);
			globalMouseCurrentX = p.x;
			globalMouseCurrentY = p.y;
			
			globalMouseDeltaX = p.x - globalMouseLastX;							
			globalMouseDeltaY = p.y - globalMouseLastY;

			//std::cout << "\nGlobX: " << globalMouseDeltaX << "\n";
			//std::cout << "\nGlobY: " << globalMouseDeltaY << "\n";

			globalMouseLastX = p.x;
			globalMouseLastY = p.y;			

			return 0L;
		}

		/*
		- Maybe use this to pass a pointer to the image buffer
		https://stackoverflow.com/questions/19761167/winapi-c-how-to-pass-data-from-window-to-window-without-globals
		*/
		case WM_CREATE:
		{
			DEBUG_MSG_L0(__func__, "WM_CREATE");
			/*
			if (global_newBitmap == NULL) {
				std::cout << "Loading background...\n";
				global_newBitmap = (HBITMAP*)LoadImage(NULL, ".\\high_photon_cornellbox_bw.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
			}
			/**/
			return 0L;
		}

		case WM_ERASEBKGND:
		{
			DEBUG_MSG_L0(__func__, "WM_ERASEBKGND");
	#if 1
			RECT rctBrush;
			HBRUSH hBrushWhite, hBrushGray;

			hBrushWhite = (HBRUSH)GetStockObject(WHITE_BRUSH);
			hBrushGray = (HBRUSH)GetStockObject(GRAY_BRUSH);

			HDC hdcRaytraceWindow;
			hdcRaytraceWindow = GetDC(raytraceMSWindowHandle);

			GetClientRect(hwnd, &rctBrush);
			SetMapMode(hdcRaytraceWindow, MM_ANISOTROPIC);
			SetWindowExtEx(hdcRaytraceWindow, 100, 100, NULL);
			SetViewportExtEx(hdcRaytraceWindow, rctBrush.right, rctBrush.bottom, NULL);
			FillRect(hdcRaytraceWindow, &rctBrush, hBrushWhite);

			int x = 0;
			int y = 0;

			for (int i = 0; i < 13; i++)
			{
				x = (i * 40) % 100;
				y = ((i * 40) / 100) * 20;
				SetRect(&rctBrush, x, y, x + 20, y + 20);
				FillRect(hdcRaytraceWindow, &rctBrush, hBrushGray);
			}

			DeleteDC(hdcRaytraceWindow);
			return 0L;
	#endif

	#if 0
			PAINTSTRUCT ps;
			HDC hdcClientWindow;
			BITMAP bitmap;
			HDC hdcBlitWindow;
			HGDIOBJ currentBitmap;
			HBITMAP newBitmap;

			std::cout << "Loading background...\n";
			newBitmap = (HBITMAP)LoadImage(NULL, ".\\high_photon_cornellbox_bw.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

			hdcClientWindow = BeginPaint(hwnd, &ps);

			//create a "clone" of the current hdcWindow that will have the "new" bitmap painted to it
			hdcBlitWindow = CreateCompatibleDC(hdcClientWindow);
			//put the newBitmap into the hdcBlitWindow context, returns a handle the device context
			currentBitmap = SelectObject(hdcBlitWindow, newBitmap);

			//get the properites of the newBitmap
			GetObject(newBitmap, sizeof(bitmap), &bitmap);

			//Bit blit the hdcBlitWindow to the hdcClientWindow
			BitBlt(hdcClientWindow, 0, 0, bitmap.bmWidth, bitmap.bmHeight, hdcBlitWindow, 0, 0, SRCCOPY);

			//free memory associated with the "old" newBitmap aka, currentBitmap
			SelectObject(hdcBlitWindow, currentBitmap);

			DeleteDC(hdcBlitWindow);

			EndPaint(hwnd, &ps);
	#endif
		}

		case WM_PAINT:
		{			
	#if 1			
			PAINTSTRUCT ps;
			HDC hdcClientWindow;
			HDC hdcBlitWindow;

			BITMAP bitmap;
			HGDIOBJ currentBitmap;

			hdcClientWindow = BeginPaint(hwnd, &ps);

			//create a "clone" of the current hdcWindow that will have the "new" bitmap painted to it
			hdcBlitWindow = CreateCompatibleDC(hdcClientWindow);
			//put the newBitmap into the hdcBlitWindow context, returns a handle the device context
			//currentBitmap = SelectObject(hdcBlitWindow, global_newBitmap);
			currentBitmap = SelectObject(hdcBlitWindow, workingBitmap);

			//get the properites of the newBitmap
			//GetObject(global_newBitmap, sizeof(bitmap), &bitmap);
			GetObject(workingBitmap, sizeof(bitmap), &bitmap);

			//Bit blit the hdcBlitWindow to the hdcClientWindow
			BitBlt(hdcClientWindow, 0, 0, bitmap.bmWidth, bitmap.bmHeight, hdcBlitWindow, 0, 0, SRCCOPY);

			//free memory associated with the "old" newBitmap aka, currentBitmap
			SelectObject(hdcBlitWindow, currentBitmap);

			DeleteDC(hdcBlitWindow);

			//draw some text
			//https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-drawtext
	#if DISPLAY_FPS == 1
			//SetBkColor(hdcClientWindow, RGB(0, 0, 255));
			SetBkMode(hdcClientWindow, TRANSPARENT);
			SetTextColor(hdcClientWindow, RGB(255, 0, 0));
			
			//Create a basic FPS counter
			lastFrameTimeSample = currentFrameTimeSample;
			currentFrameTimeSample = clock();			

			clock_t deltaTime = currentFrameTimeSample - lastFrameTimeSample;

			float clocksPerSec = (float)deltaTime / CLOCKS_PER_SEC;
			int instantaneousFPS = (int)1 / clocksPerSec;

			//make sure it doesn't take more than four char positions
			instantaneousFPS = instantaneousFPS % 1000;

			//std::cout << "frame time (in seconds): " << clocksPerSec << "\n";
			//std::cout << "FPS (instantaneous): " << instantaneousFPS << "\n";

			RECT fpsRect;
			fpsRect = { 0, 0, 100, 100 };

			char fpsText[100] = "FPS: ";					

			//convert int to string
			char fpsIntText[4];
			sprintf_s(fpsIntText, 4, "%d", instantaneousFPS);			

			strcat_s(fpsText, 100, fpsIntText);						
			
			DrawText(hdcClientWindow, (LPCSTR)&fpsText, -1, &fpsRect, DT_CENTER);		
	#endif

			EndPaint(hwnd, &ps);
	#endif
			return 0L;
		}

		/* Ignored so default action taken (destorys window)
		 * https://docs.microsoft.com/en-us/windows/win32/learnwin32/closing-the-window
		case WM_CLOSE:
		{
			DEBUG_MSG_L0(__func__, "WM_CLOSE");
			return 0L;
		}
		*/

		case WM_DESTROY:
		{
			DEBUG_MSG_L0(__func__, "WM_DESTROY");

			PostQuitMessage(0);

			return 0L;
		}

		case WM_LBUTTONDOWN:
		{
			DEBUG_MSG_L0(__func__, "WM_LBUTTONDOWN: " << LOWORD(lParam) << "," << HIWORD(lParam) << "\n");

	#if 1
			HDC hdcRaytraceWindow;
			hdcRaytraceWindow = GetDC(raytraceMSWindowHandle);

			for (int x = 0; x < 10; x++) {
				for (int y = 0; y < 10; y++) {
					SetPixel(hdcRaytraceWindow, x + p.x, y + p.y, RGB(255, 0, 0));
				}
			}

			DeleteDC(hdcRaytraceWindow);
	#endif

			//ask to redraw the window
			//RedrawWindow(hwnd, NULL, NULL, RDW_INTERNALPAINT);
			RedrawWindow(hwnd, NULL, NULL, RDW_NOERASE);

			std::lock_guard<std::mutex> lock(globalMouseXYInputMutex);
			globalMouseCurrentX = p.x;
			globalMouseCurrentY = p.y;

			return 0L;
		}

		case WM_LBUTTONDBLCLK: {
			DEBUG_MSG_L0(__func__, "WM_LBUTTODBLCLK " << LOWORD(lParam) << "," << HIWORD(lParam) << "\n");

			return 0L;
		}

		case WM_SIZE: {
			DEBUG_MSG_L0(__func__, "WM_SIZE");
	#if 0
			PAINTSTRUCT ps;
			HDC hdcClientWindow;

			hdcClientWindow = GetDC(raytraceMSWindowHandle);

			//figure out how big to make the whole window
			RECT rect;
			LONG windowWidth = 0, windowHeight = 0;
			rect = { 0, 0, windowWidth, windowHeight };

			bool result = GetClientRect(raytraceMSWindowHandle, &rect);

			std::cout << "Calling resize...\n";

			hdcClientWindow = BeginPaint(hwnd, &ps);
		
			//Bit blit the hdcClientWindow with itself
			BitBlt(hdcClientWindow, 0, 0, windowWidth, windowHeight, hdcClientWindow, 0, 0, SRCCOPY);

			EndPaint(hwnd, &ps);

			DeleteDC(hdcClientWindow);
	#endif
			return 0L;
		}

		case WM_USER: {
			//DEBUG_MSG_L0(__func__, "WM_USER");

			//This code gets the current rectangle coords from the window space and then figures out where those points
			//are relative to desktop coord space. This modified coord space gets passed onto the ClipCursor call to
			//basically trap the cursor within the window. Not using raw input intentionally for now.
			/*https://stackoverflow.com/questions/36779161/trap-cursor-in-window*/
			RECT rect;
			GetClientRect(hwnd, &rect);

			POINT ul;
			ul.x = rect.left;
			ul.y = rect.top;

			POINT lr;
			lr.x = rect.right;
			lr.y = rect.bottom;

			MapWindowPoints(hwnd, nullptr, &ul, 1);
			MapWindowPoints(hwnd, nullptr, &lr, 1);

			rect.left = ul.x;
			rect.top = ul.y;

			rect.right = lr.x;
			rect.bottom = lr.y;

	#if CAPTURE_MOUSE == 1
			ClipCursor(&rect);

			SetCursorPos(rect.right - (rect.right - rect.left)/2, rect.top - (rect.top - rect.bottom)/2);
	#endif
			//drowan_20190916: Freeing the used memory seems to get around the CopyImage call failing. 
			//Seems to be OK handling a null workingBitmap when first called.
			DeleteObject(workingBitmap);

			workingBitmap = (HBITMAP)CopyImage((HBITMAP*)lParam, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);

			//https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-redrawwindow
			RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);

			return 0L;
		}

		case WM_USER + 1: {
			DEBUG_MSG_L0(__func__, "WM_USER + 1");
			return 0L;
		}

		default:
		{
			//DEBUG_MSG_L0(__func__, "3, umsg: " << uMsg);

			return DefWindowProc(hwnd, uMsg, wParam, lParam);
		}
	}	
}

