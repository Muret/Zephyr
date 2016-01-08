
#include "window.h"

extern HMODULE g_hinstance;
extern HWND g_hwnd;
extern int g_screenWidth, g_screenHeight;
extern int g_nCmdShow;

void openWindow()
{
	WNDCLASSEX wc;
	//DEVMODE dmScreenSettings;
	int posX, posY;


	// Get the instance of this application.
	g_hinstance = GetModuleHandle(NULL);

	// clear out the window class for use
	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	// fill in the struct with the needed information
	//Step 1: Registering the Window Class
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = g_hinstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "Demo";
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	// Register the window class.
	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, "Window Registration Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	// Determine the resolution of the clientss desktop screen.
	g_screenWidth = GetSystemMetrics(SM_CXSCREEN);
	g_screenHeight = GetSystemMetrics(SM_CYSCREEN);


	// If windowed then set it to 800x600 resolution.
	g_screenWidth = 1024;
	g_screenHeight = 1024;

	// Place the window in the middle of the screen.
	posX = 500;
	posY = 100;
	

	// Create the window with the screen settings and get the handle to it.
	g_hwnd = CreateWindowEx(	0,
								"Demo",    // name of the window class
								"Muret Demo",   // title of the window
								WS_OVERLAPPEDWINDOW,    // window style
								posX,    // x-position of the window
								posY,    // y-position of the window
								g_screenWidth,    // width of the window
								g_screenHeight,    // height of the window
								NULL,    // we have no parent window, NULL
								NULL,    // we aren't using menus, NULL
								g_hinstance,    // application handles
								NULL);    // used with multiple windows, NULL

	int TimmerID = SetTimer( g_hwnd , 0 , 1000 , NULL);

	int lastError = GetLastError();
	

	// Bring the window up on the screen and set it as main focus.
	ShowWindow(g_hwnd, g_nCmdShow);

	// Hide the mouse cursor.
	//ShowCursor(false);

	return;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	switch (umessage)
	{
		// Check if the window is being destroyed.
	case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}

		// Check if the window is being closed.
	case WM_CLOSE:
		{
			PostQuitMessage(0);
			return 0;
		}

	case WM_NCCREATE:
		{
			return true;
		}

		// All other messages pass to the message handler in the system class.
	}
	return DefWindowProc(hwnd, umessage, wparam, lparam);
}