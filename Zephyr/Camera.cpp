
#include "d11.h"

#include "Camera.h"



void Camera::init_camera()
{

}


void Camera::tick()
{

}

Camera::Camera()
{
	camera_position_ = D3DXVECTOR4(0, 0, 0, 0);
	view_vector_ = D3DXVECTOR4(0, 0, 0, 0);
	right_vector_ = D3DXVECTOR4(0, 0, 0, 0);
	up_vector_ = D3DXVECTOR4(0, 0, 0, 0);

	near_ = 0;
	far_ = 0;

	is_ortho_ = false;
}

void Camera::validate_cur_frame_cache()
{
	D3DXVECTOR3 lookat = camera_position_ + view_vector_;
	D3DXVECTOR3 up_vector_v3 = up_vector_;
	D3DXVECTOR3 cam_position_v3 = camera_position_;

	D3DXMatrixLookAtRH(&cur_frame_view_matrix_, &cam_position_v3, &lookat, &up_vector_v3);

	if (is_ortho_)
	{
		D3DXMatrixOrthoRH(&cur_frame_inv_projection_matrix_, right_ - left_, top_ - bottom_, near_, far_);
	}
	else
	{
		D3DXMatrixPerspectiveFovRH(&cur_frame_projection_matrix_, (field_of_view_ / 180) * PI, float(g_screenWidth) / float(g_screenHeight), near_, far_);
	}
	
	cur_frame_view_projection_matrix_ = cur_frame_view_matrix_ * cur_frame_projection_matrix_;

	float determinant;
	D3DXMatrixInverse(&cur_frame_inv_view_matrix_, &determinant, &cur_frame_view_matrix_);
	D3DXMatrixInverse(&cur_frame_inv_projection_matrix_, &determinant, &cur_frame_projection_matrix_);
	D3DXMatrixInverse(&cur_frame_inv_view_projection_matrix_, &determinant, &cur_frame_view_projection_matrix_);
}

const D3DXVECTOR4& Camera::get_position() const
{
	return camera_position_;
}

const D3DXVECTOR4& Camera::get_forward_vector() const
{
	return view_vector_;
}

const D3DXVECTOR4& Camera::get_up_vector() const
{
	return up_vector_;
}

const D3DXVECTOR4& Camera::get_right_vector() const
{
	return right_vector_;
}

const D3DXMATRIX& Camera::get_view_matrix() const
{
	return cur_frame_view_matrix_;
}

const D3DXMATRIX& Camera::get_view_projection_matrix() const
{
	return cur_frame_view_projection_matrix_;
}

const D3DXMATRIX& Camera::get_projection_matrix() const
{
	return cur_frame_projection_matrix_;
}

const D3DXMATRIX& Camera::get_inv_view_matrix() const
{
	return cur_frame_inv_view_matrix_;
}

const D3DXMATRIX& Camera::get_inv_view_projection_matrix() const
{
	return cur_frame_inv_view_projection_matrix_;
}

const D3DXMATRIX& Camera::get_inv_projection_matrix() const
{
	return cur_frame_inv_projection_matrix_;
}

float Camera::get_near() const
{
	return near_;
}

float Camera::get_far() const
{
	return far_;
}

std::string Camera::get_name() const
{
	return name_;
}

float Camera::get_fov() const
{
	return field_of_view_;
}

void Camera::set_position(const D3DXVECTOR4 &p)
{
	camera_position_ = p;
}

void Camera::set_frame(const D3DXMATRIX &value)
{
	camera_frame_ = value;
}

void Camera::set_directions(const D3DXVECTOR4 &view, const D3DXVECTOR4 &up, const D3DXVECTOR4 &right)
{
	view_vector_ = view;
	up_vector_ = up;
	right_vector_ = right;
}

void Camera::set_near(float near_value)
{
	near_ = near_value;
}

void Camera::set_far(float far_value)
{
	far_ = far_value;
}

void Camera::set_name(string name)
{
	name_ = name;
}

const D3DXMATRIX& Camera::get_frame() const
{
	return camera_frame_;
}

void Camera::set_fov(float v)
{
	field_of_view_ = v;
}

void Camera::set_is_ortho(bool v)
{
	is_ortho_ = v;
}

void Camera::set_ortho_params(float left, float right, float top, float bottom, float near_v, float far_v)
{
	left_ = left;
	right_ = right;
	top_ = top;
	bottom_ = bottom;
	near_ = near_v;
	far_ = far_v;
}

