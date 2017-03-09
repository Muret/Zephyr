
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
#include "VCT_Demo.h"
#include "CatmullClark_Demo.h"
#include "KeyChain.h"
#include "GPUBasedPipelineDemo.h"
#include "WorldMapDemo.h"
#include "PlaneAnimationDemo.h"

#include "GUI.h"

extern HINSTANCE g_hinstance;
extern int g_nCmdShow;

void main_render_loop();
string ExePath();

extern int frame_count;
extern int sort_time;

DemoBase *current_demo;

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

	string path = ExePath();

	current_demo = new SSRDemo();
	current_demo->initialize();

	MSG msg = {};
	while (1)
	{
		key_chain.pre_tick();

		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) > 0)
		{
			if (gui.handle_key_event(msg.message, msg.wParam, msg.lParam) == false)
			{
				key_chain.handle_key_event(msg.message, msg.wParam);
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		gui.start_frame();

		key_chain.tick();

		//ImGui::ShowTestWindow();

		Utilities::tick();

		current_demo->tick(0);

		renderer->render_frame();

		frame_count++;
	}


	closeEngine();


	return 0;
}


string ExePath() 
{
	char buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	string::size_type pos = string(buffer).find_last_of("\\/");
	return string(buffer).substr(0, pos);
}