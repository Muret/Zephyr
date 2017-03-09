
#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "d11.h"
#include "BoundingBox.h"

class Camera
{
public:
	Camera();

	void init_camera();
	virtual void tick();
	void validate_cur_frame_cache();
	
	const D3DXVECTOR4& get_position() const;
	const D3DXVECTOR4& get_forward_vector() const;
	const D3DXVECTOR4& get_up_vector() const;
	const D3DXVECTOR4& get_right_vector() const;

	const D3DXMATRIX& get_view_matrix() const;
	const D3DXMATRIX& get_view_projection_matrix() const;
	const D3DXMATRIX& get_projection_matrix() const;

	const D3DXMATRIX& get_inv_view_matrix() const;
	const D3DXMATRIX& get_inv_view_projection_matrix() const;
	const D3DXMATRIX& get_inv_projection_matrix() const;

	float get_near() const;
	float get_far() const;
	string get_name() const;
	float get_fov() const;

	virtual void set_position(const D3DXVECTOR4& pos);
	virtual void set_frame(const D3DXMATRIX &value);

	void set_directions(const D3DXVECTOR4 &view, const D3DXVECTOR4 &up, const D3DXVECTOR4 &right);

	void set_near(float near_v);
	void set_far(float far_v);
	void set_name(string name);

	const D3DXMATRIX& get_frame() const;
	void set_fov(float far_v);

	void set_is_ortho(bool v);

	void set_ortho_params(float left, float right, float top, float bottom, float near_v, float far_);

	void check_bb_in_frustum(const BoundingBox &bb);

protected:
	//camera parameters
	D3DXVECTOR4 camera_position_;
	D3DXVECTOR4 view_vector_;
	D3DXVECTOR4 right_vector_;
	D3DXVECTOR4 up_vector_;
	D3DXMATRIX camera_frame_;

	float near_, far_;
	float field_of_view_;

	//orthographic rendering params
	bool is_ortho_;
	float left_, right_, top_, bottom_;

	//current frame cache variables
	D3DXMATRIX cur_frame_view_matrix_;
	D3DXMATRIX cur_frame_projection_matrix_;
	D3DXMATRIX cur_frame_view_projection_matrix_;

	D3DXMATRIX cur_frame_inv_view_matrix_;
	D3DXMATRIX cur_frame_inv_projection_matrix_;
	D3DXMATRIX cur_frame_inv_view_projection_matrix_;

	string name_;
};

#endif