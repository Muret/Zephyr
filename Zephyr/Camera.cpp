
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
	const float multiplier = 5;

	if (is_moving_camera)
	{
		POINT  p;
		GetCursorPos(&p);
		y_camera_bearing -= (p.y - last_mouse_position.y) * 0.002 * multiplier;
		if (y_camera_bearing > PI)
		{
			y_camera_bearing -= PI;
		}
		x_camera_bearing -= (p.x - last_mouse_position.x) * 0.002 * multiplier;

		last_mouse_position = D3DXVECTOR2(p.x, p.y);
	}



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
		camera_position += 0.06 * view_direction * multiplier;
	}

	if (keys['S'])
	{
		camera_position -= 0.06 * view_direction * multiplier;
	}

	if (keys['D'])
	{
		camera_position += 0.11 * right_vector * multiplier;
	}

	if (keys['A'])
	{
		camera_position -= 0.11 * right_vector * multiplier;
	}

	if (keys['Q'])
	{
		camera_position += 0.11 * up_vector * multiplier;
	}

	if (keys['E'])
	{
		camera_position -= 0.11 * up_vector * multiplier;
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
	original_view_direction		= D3DXVECTOR3(0, 0, -1);
	original_right_vector		= D3DXVECTOR3(1, 0, 0);
	original_up_vector			= D3DXVECTOR3(0, 1, 0);

	camera_position = D3DXVECTOR3(0, 10, 80);
	view_direction = original_view_direction;
	right_vector = original_right_vector;
	up_vector = original_up_vector;
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

bool Camera::is_key_down(char key) const
{
	return keys[key];
}

D3DXMATRIX Camera::get_view_projection_matrix() const
{
	D3DXMATRIX view, projection;
	D3DXVECTOR3 lookat = camera_position + view_direction;
	D3DXMatrixLookAtRH(&view, &camera_position, &lookat, &up_vector);

	D3DXMatrixPerspectiveFovRH(&projection, PI / 6.0f, float(g_screenWidth) / float(g_screenHeight), 0.1f, 10000.0f);

	float determinant;
	return view * projection;
}


