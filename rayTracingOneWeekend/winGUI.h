#pragma once
#include <Windows.h>
#include <iostream>

#include "common.h"
#include "debug.h"

HWND raytraceMSWindowHandle;

//HBITMAP *global_newBitmap = 0;
HBITMAP workingBitmap;

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

int guiWorkerProcedure(
	std::shared_ptr<WorkerThread> workerThreadStruct,
	uint32_t windowWidth,
	uint32_t windowHeight) {

	std::unique_lock<std::mutex> coutLock(globalCoutGuard);
	coutLock.unlock();

	//wait for exit
	std::unique_lock<std::mutex> exitLock(workerThreadStruct->exitMutex);
	exitLock.unlock();

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

				if (status == -1) {
					//TODO: something went wrong (i.e. invalid memory read for message??), so throw an error and exit
					DEBUG_MSG_L0(__func__, " An error occured when calling GetMessage()");
					return -1;
				}
				else {
					DispatchMessage(&msg);
				}				
			}			
		}

		DestroyWindow(raytraceMSWindowHandle);
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
		if (ScreenToClient(hwnd, &p)) {
			if (p.x >= 0 && p.y >= 0) {
				//std::cout << "\nMousepoint " << p.x << ", " << p.y << "\n";
			}
		}
	}

	switch (uMsg) {

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
		DEBUG_MSG_L0(__func__, "WM_PAINT");
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

		EndPaint(hwnd, &ps);
#endif
		return 0L;
	}

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
		DEBUG_MSG_L0(__func__, "WM_USER");

		//global_newBitmap = (HBITMAP*)lParam;

		workingBitmap = (HBITMAP)CopyImage((HBITMAP*)lParam, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);

		//https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-redrawwindow
		RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);

		return 0L;
	}

	case WM_USER + 1: {
		DEBUG_MSG_L0(__func__, "WM_USER + 1");
	}

	default:
	{
		//std::cout << "\nUnhandled WM message\n";

		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	}
}

