
#ifndef _GUI_H_
#define _GUI_H_

#include "d11.h"
#include "imgui.h"

class GUI
{

public:
	GUI();
	
	void init(void* hwnd, ID3D11Device* device, ID3D11DeviceContext* device_context);
	void start_frame();
	void render_frame();
	
	bool handle_key_event(UINT msg, WPARAM wParam, LPARAM lParam);

private:

};

extern GUI gui;

#endif