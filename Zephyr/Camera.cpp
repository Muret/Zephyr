
#include "d11.h"

#include "Camera.h"

void Camera::init_camera()
{
	memset(keys, 0, sizeof(char)* 256);
	x_camera_bearing = 0;
	y_camera_bearing = 0;
}

void Camera::handle_user_input_down(char key)
{
	keys[key] = true;
}

void Camera::handle_user_input_up(char key)
{
	keys[key] = false;
}

void Camera::tick_user_inputs()
{
	if (is_moving_camera)
	{
		POINT  p;
		GetCursorPos(&p);
		y_camera_bearing -= (p.y - last_mouse_position.y) * 0.006;
		if (y_camera_bearing > PI)
		{
			y_camera_bearing -= PI;
		}
		x_camera_bearing -= (p.x - last_mouse_position.x) * 0.006;

		last_mouse_position = D3DXVECTOR2(p.x, p.y);
	}

	D3DXVECTOR3 original_view_direction(0, 1, 0);
	D3DXVECTOR3 original_right_vector(1, 0, 0);
	D3DXVECTOR3 original_up_vector(0, 0, 1);

	D3DXVECTOR3 view_temp;
	D3DXVECTOR3 view_temp2;
	D3DXVECTOR3 right_temp;
	D3DXVECTOR3 up_temp;

	D3DXMATRIX m_1;
	D3DXMATRIX m_2;

	D3DXMatrixRotationAxis(&m_2, &original_up_vector, x_camera_bearing);
	D3DXVec3TransformCoord(&view_temp2, &original_view_direction, &m_2);
	D3DXVec3TransformCoord(&right_temp, &original_right_vector, &m_2);

	D3DXMatrixRotationAxis(&m_1, &right_vector, y_camera_bearing);
	D3DXVec3TransformCoord(&view_temp, &view_temp2, &m_1);
	D3DXVec3TransformCoord(&up_temp, &original_up_vector, &m_1);

	D3DXVec3Normalize(&up_vector, &up_temp);
	D3DXVec3Normalize(&view_direction, &view_temp);
	D3DXVec3Normalize(&right_vector, &right_temp);

	if (keys['W'])
	{
		camera_position += 0.006 * view_direction;
	}

	if (keys['S'])
	{
		camera_position -= 0.006 * view_direction;
	}

	if (keys['D'])
	{
		camera_position += 0.011 * right_vector;
	}

	if (keys['A'])
	{
		camera_position -= 0.011 * right_vector;
	}

	if (keys['Q'])
	{
		camera_position += 0.011 * up_vector;
	}

	if (keys['E'])
	{
		camera_position -= 0.011 * up_vector;
	}
}

void Camera::startMovingCamera()
{
	if (!is_moving_camera)
	{
		POINT  p;
		GetCursorPos(&p);
		last_mouse_position = D3DXVECTOR2(p.x, p.y);

		is_moving_camera = true;

	}
}

void Camera::stopMovingCamera()
{
	is_moving_camera = false;
}

Camera::Camera()
{
	camera_position = D3DXVECTOR3(0, -20.0, 0);
	view_direction = D3DXVECTOR3(0, 1, 0);
	right_vector = D3DXVECTOR3(1, 0, 0);
	up_vector = D3DXVECTOR3(0, 0, 1);
}

D3DXVECTOR3 Camera::get_position() const
{
	return camera_position;
}

D3DXVECTOR3 Camera::get_forward_vector() const 
{
	return view_direction;
}

D3DXVECTOR3 Camera::get_up_vector() const
{
	return up_vector;
}

D3DXVECTOR3 Camera::get_right_vector() const
{
	return right_vector;
}


