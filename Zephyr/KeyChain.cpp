
#include "KeyChain.h"

KeyChain key_chain;

KeyChain::KeyChain()
{
	memset(keys, 0, sizeof(char) * 256);
	
}

void KeyChain::tick()
{
	
}

void KeyChain::pre_tick()
{
	last_mouse_position = current_mouse_position;
}

bool KeyChain::key(int key) const
{
	ZEPHYR_ASSERT(key >= 0 && key < 256);
	return keys[key];
}

D3DXVECTOR2 KeyChain::get_cur_mouse_position() const
{
	return current_mouse_position;
}

D3DXVECTOR2 KeyChain::get_mouse_move() const
{
	return current_mouse_position - last_mouse_position;
}

void KeyChain::handle_key_event(int key, int mode)
{
	switch (key)
	{
		case WM_QUIT:
			exit(0);
			break;
		case WM_KEYDOWN:
			if (mode == VK_ESCAPE)
				exit(0);
			else
				keys[mode] = 1;
			break;
		case WM_KEYUP:
			keys[mode] = 0;
			break;
		case WM_RBUTTONDOWN:
			keys[KeyType::right_mouse_button] = 1;
			break;
		case WM_RBUTTONUP:
			keys[KeyType::right_mouse_button] = 0;
			break;
		case WM_LBUTTONDOWN:
			keys[KeyType::left_mouse_button] = 1;
			break;
		case WM_LBUTTONUP:
			keys[KeyType::left_mouse_button] = 0;
			break;
		case WM_MOUSEMOVE:
		{
			POINT  p;
			GetCursorPos(&p);
			current_mouse_position = D3DXVECTOR2(p.x, p.y);
			break;
		}
	}
}


