
#include "window.h"
#include "d11.h"
#include "includes.h"
#include "Camera.h"
#include "Demo.h"
#include "Camera.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "Utilities.h"

#include "SSR_Demo.h"

extern HINSTANCE g_hinstance;
extern int g_nCmdShow;

void main_render_loop();
string ExePath();

extern int frame_count;
extern int sort_time;

DemoBase *currentDemo;

//we all love main.cpp's
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	g_hinstance = hInstance;
	g_nCmdShow = iCmdshow;

	openWindow();

	frame_count = 0;
	int last_frame_count = 0;

	init_engine();
	
	renderer = new Renderer;
	
	resource_manager.init_resources("..\\Resources");

	demo_camera.init_camera();
	string path = ExePath();

	currentDemo = new SSRDemo();
	currentDemo->initialize();

	MSG msg = {};
	while (1)
	{
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) > 0) //Or use an if statement
		{
			switch (msg.message)
			{
			case WM_QUIT:
				exit(0);
				break;
			case WM_KEYDOWN:
				if (msg.wParam == VK_ESCAPE)
					exit(0);
				else
					demo_camera.handle_user_input_down((char)msg.wParam);
				break;
			case WM_KEYUP:
				demo_camera.handle_user_input_up((char)msg.wParam);
				break;
			case WM_TIMER:
				last_frame_count = frame_count;
				frame_count = 0;
				break;
			case WM_RBUTTONDOWN:
				demo_camera.startMovingCamera();
				break;
			case WM_RBUTTONUP:
				demo_camera.stopMovingCamera();
				break;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		demo_camera.tick_user_inputs();
		Utilities::tick();

		renderer->render_frame();

		frame_count++;
	}


	closeEngine();


	return 0;
}


string ExePath() {
	char buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	string::size_type pos = string(buffer).find_last_of("\\/");
	return string(buffer).substr(0, pos);
}