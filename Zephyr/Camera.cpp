
#include "d11.h"

#include "Camera.h"



void Camera::init_camera()
{

}


void Camera::tick()
{
	validate_cur_frame_cache();
}

Camera::Camera()
{
	camera_position_ = D3DXVECTOR4(0, 0, 0, 0);
	view_vector_ = D3DXVECTOR4(0, 0, 0, 0);
	right_vector_ = D3DXVECTOR4(0, 0, 0, 0);
	up_vector_ = D3DXVECTOR4(0, 0, 0, 0);

	near_ = 0.01;
	far_ = 1000;

	is_ortho_ = false;

	field_of_view_ = 60;
}

void Camera::validate_cur_frame_cache()
{
	D3DXVECTOR3 lookat = camera_position_ + view_vector_;
	D3DXVECTOR3 up_vector_v3 = up_vector_;
	D3DXVECTOR3 cam_position_v3 = camera_position_;

	D3DXMatrixLookAtRH(&cur_frame_view_matrix_, &cam_position_v3, &lookat, &up_vector_v3);

	if (is_ortho_)
	{
		D3DXMatrixOrthoRH(&cur_frame_projection_matrix_, right_ - left_, top_ - bottom_, near_, far_);
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
	D3DXVECTOR3 l_view, l_up, l_right;
	D3DXVECTOR3 l_view2, l_up2, l_right2;

	D3DXVec4Normalize(&view_vector_, &view);
	D3DXVec4Normalize(&up_vector_, &up);
	D3DXVec4Normalize(&right_vector_, &right);

	//l_view.x = view_vector_.x;	
	//l_view.y = view_vector_.y;
	//l_view.z = view_vector_.z;
	//
	//l_up.x = up_vector_.x;
	//l_up.y = up_vector_.y;
	//l_up.z = up_vector_.z;
	//
	//l_right.x = right_vector_.x;
	//l_right.y = right_vector_.y;
	//l_right.z = right_vector_.z;
	//
	//D3DXVec3Cross(&l_up2, &l_right, &l_view);
	//D3DXVec3Normalize(&l_up2, &l_up2);
	//
	//D3DXVec3Cross(&l_right2, &l_view, &l_up2);
	//D3DXVec3Normalize(&l_right2, &l_right2);
	//
	//up_vector_ = D3DXVECTOR4(l_up2.x, l_up2.y, l_up2.z, 0);
	//right_vector_ = D3DXVECTOR4(l_right2.x, l_right2.y, l_right2.z, 0);
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

void Camera::check_bb_in_frustum(const BoundingBox & bb)
{
	float aspect_ratio = float(g_screenWidth) / float(g_screenHeight);
	float half_angle_rad = (field_of_view_ / 180.0f) * PI * 0.5f;

	D3DXVECTOR4 near_plane_center = camera_position_ + view_vector_ * near_;
	float near_plane_up_length = tan(half_angle_rad) * near_;
	float near_plane_right_length = aspect_ratio * near_plane_up_length;

	D3DXVECTOR4 near_plane_right_up = near_plane_center + up_vector_ * near_plane_up_length + right_vector_ * near_plane_right_length;
	D3DXVECTOR4 near_plane_right_down = near_plane_center - up_vector_ * near_plane_up_length + right_vector_ * near_plane_right_length;
	D3DXVECTOR4 near_plane_left_up = near_plane_center + up_vector_ * near_plane_up_length - right_vector_ * near_plane_right_length;
	D3DXVECTOR4 near_plane_left_down = near_plane_center - up_vector_ * near_plane_up_length - right_vector_ * near_plane_right_length;

	D3DXVECTOR4 far_plane_center = camera_position_ + view_vector_ * far_;
	float far_plane_up_length = near_plane_up_length * far_ / near_;
	float far_plane_right_length = near_plane_right_length * far_ / near_;

	D3DXVECTOR4 far_plane_right_up = far_plane_center + up_vector_ * far_plane_up_length + right_vector_ * far_plane_right_length;
	D3DXVECTOR4 far_plane_right_down = far_plane_center - up_vector_ * far_plane_up_length + right_vector_ * far_plane_right_length;
	D3DXVECTOR4 far_plane_left_up = far_plane_center + up_vector_ * far_plane_up_length - right_vector_ * far_plane_right_length;
	D3DXVECTOR4 far_plane_left_down = far_plane_center - up_vector_ * far_plane_up_length - right_vector_ * far_plane_right_length;



}

