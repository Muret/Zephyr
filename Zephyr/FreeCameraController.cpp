
#include "FreeCameraController.h"
#include "KeyChain.h"

const D3DXVECTOR4 FreeCameraController::default_forward_ = D3DXVECTOR4(0,0,-1,0);
const D3DXVECTOR4 FreeCameraController::default_up_ = D3DXVECTOR4(0, 1, 0,0);
const D3DXVECTOR4 FreeCameraController::default_right_ = D3DXVECTOR4(1, 0, 0 ,0);

FreeCameraController::FreeCameraController()
{
	yaw_ = 0.0f;
	pitch_ = 0.0f;
}

void FreeCameraController::tick()
{
	float multiplier = 1.0f;

	if (key_chain.key(KeyType::right_mouse_button))
	{
		D3DXVECTOR2 delta = key_chain.get_mouse_move();
		pitch_ += delta.y * 0.001;
		yaw_ -= delta.x * 0.001;
		
		while(yaw_ > PI)
		{
			yaw_ -= PI;
		}

		while (pitch_ > PI)
		{
			pitch_ -= PI;
		}
	}

	recompute_rotation();


	if (key_chain.key('W'))
	{
		camera_position_ += 0.06 * view_vector_ * multiplier;
	}

	if (key_chain.key('S'))
	{
		camera_position_ -= 0.06 * view_vector_ * multiplier;
	}

	if (key_chain.key('D'))
	{
		camera_position_ += 0.06 * right_vector_ * multiplier;
	}

	if (key_chain.key('A'))
	{
		camera_position_ -= 0.06 * right_vector_ * multiplier;
	}

	if (key_chain.key('Q'))
	{
		camera_position_ += 0.06 * up_vector_ * multiplier;
	}

	if (key_chain.key('E'))
	{
		camera_position_ -= 0.06 * up_vector_ * multiplier;
	}

	validate_cur_frame_cache();
}

void FreeCameraController::recompute_rotation()
{
	D3DXVECTOR3 view_temp;
	D3DXVECTOR3 view_temp2;
	D3DXVECTOR3 right_temp;
	D3DXVECTOR3 up_temp;

	D3DXVECTOR3 right_vector_v3 = right_vector_;
	D3DXVECTOR3 up_vector_v3 = up_vector_;
	D3DXVECTOR3 view_vector_v3 = view_vector_;

	D3DXVECTOR3 default_right_v3 = default_right_;
	D3DXVECTOR3 default_forward_v3 = default_forward_;
	D3DXVECTOR3 default_up_v3 = default_up_;

	D3DXMATRIX m_1;
	D3DXMATRIX m_2;

	D3DXMatrixRotationAxis(&m_2, &default_up_v3, yaw_);
	D3DXVec3TransformCoord(&view_temp2, &default_forward_v3, &m_2);
	D3DXVec3TransformCoord(&right_temp, &default_right_v3, &m_2);

	D3DXMatrixRotationAxis(&m_1, &right_temp, -pitch_);
	D3DXVec3TransformCoord(&view_temp, &view_temp2, &m_1);
	D3DXVec3TransformCoord(&up_temp, &default_up_v3, &m_1);

	D3DXVec3Normalize(&up_vector_v3, &up_temp);
	D3DXVec3Normalize(&view_vector_v3, &view_temp);
	D3DXVec3Normalize(&right_vector_v3, &right_temp);

	right_vector_ = D3DXVECTOR4(right_vector_v3.x, right_vector_v3.y, right_vector_v3.z, 1);
	up_vector_ = D3DXVECTOR4(up_vector_v3.x, up_vector_v3.y, up_vector_v3.z, 1);
	view_vector_ = D3DXVECTOR4(view_vector_v3.x, view_vector_v3.y, view_vector_v3.z, 1);
}

void FreeCameraController::set_frame(const D3DXMATRIX& value)
{
	camera_position_ = D3DXVECTOR4(value.m[3][0], value.m[3][1], value.m[3][2], 1);

	{
		D3DXVECTOR4 view_direction = D3DXVECTOR4(value.m[0][0], value.m[0][1], value.m[0][2], 0);
		view_direction.y = 0.0f;
		D3DXVec4Normalize(&view_direction, &view_direction);

		yaw_ = acos(D3DXVec4Dot(&view_direction, &default_forward_));
	}

	{
		D3DXVECTOR4 view_direction = D3DXVECTOR4(value.m[0][0], value.m[0][1], value.m[0][2], 0);
		D3DXVECTOR4 temp = view_direction;
		view_direction.y = 0.0f;

		D3DXVec4Normalize(&view_direction, &view_direction);
		D3DXVec4Normalize(&temp, &temp);

		pitch_ = acos(D3DXVec4Dot(&temp, &view_direction));
	}

	recompute_rotation();
}
