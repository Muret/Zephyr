
#ifndef _CAMERA_H_
#define _CAMERA_H_

class Camera
{

public:
	Camera();

	void init_camera();
	
	void handle_user_input_down(char key);
	
	void handle_user_input_up(char key);
	
	void tick_user_inputs();
	
	void startMovingCamera();
	
	void stopMovingCamera();
	
	D3DXVECTOR3 get_position() const;
	D3DXVECTOR3 get_forward_vector() const;
	D3DXVECTOR3 get_up_vector() const;
	D3DXVECTOR3 get_right_vector() const;

	bool is_key_down(char key) const;
	D3DXMATRIX get_view_projection_matrix() const;
private:
	//camera parameters
	D3DXVECTOR3 camera_position;
	D3DXVECTOR3 view_direction;
	D3DXVECTOR3 right_vector;
	D3DXVECTOR3 up_vector;

	float y_camera_bearing;
	float x_camera_bearing;

	//interaction
	char keys[256];
	bool is_moving_camera;
	D3DXVECTOR2 last_mouse_position;

	D3DXVECTOR3 original_view_direction;
	D3DXVECTOR3 original_right_vector;
	D3DXVECTOR3 original_up_vector;
};

extern Camera demo_camera;

#endif