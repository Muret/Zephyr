
#ifndef _KEYCHAIN_H_
#define _KEYCHAIN_H_

#include "d11.h"

enum KeyType
{
	left_mouse_button,
	right_mouse_button,
	middle_mouse_button,
};

class KeyChain
{

public:
	KeyChain();
	
	void tick();
	void pre_tick();

	bool key(int key) const;

	D3DXVECTOR2 get_cur_mouse_position() const;
	D3DXVECTOR2 get_mouse_move() const;

	void handle_key_event(int key, int mode);

private:

	char keys[256];
	bool is_moving_camera;
	D3DXVECTOR2 last_mouse_position;
	D3DXVECTOR2 current_mouse_position;

};

extern KeyChain key_chain;

#endif