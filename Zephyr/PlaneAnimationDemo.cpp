
#include "PlaneAnimationDemo.h"

#include "Renderer.h"
#include "ResourceManager.h"
#include "FreeCameraController.h"
#include "GUI.h"

PlaneAnimationDemo::PlaneAnimationDemo() : DemoBase("PlaneAnimationDemo")
{
}

PlaneAnimationDemo::~PlaneAnimationDemo()
{
}

void PlaneAnimationDemo::initialize()
{
	//Scene *scene = new Scene("PlaneAnimationDemo");// resource_manager.get_scene("Airbus A310");
	Scene *scene = resource_manager.get_scene("cornell_centered");
	renderer->set_scene_to_render(scene);

	move_ = true;
	dx_ = 1;

	camera_controller_ = new Camera; scene->get_camera("main_camera");
	
	//camera_controller_ = new FreeCameraController();
	
	camera_controller_->set_position(D3DXVECTOR4(-20, 40, -20, 1));
	camera_controller_->set_directions(D3DXVECTOR4(1, -2, 1, 0), D3DXVECTOR4(0, 1, 0, 0), D3DXVECTOR4(-1, 0, 1, 0));
	//camera_controller_->set_directions(D3DXVECTOR4(1, 0, 1, 0), D3DXVECTOR4(0, 1, 0, 0), D3DXVECTOR4(-1, 0, 1, 0));

	camera_controller_->set_position(D3DXVECTOR4(0, 40, 0, 1));
	camera_controller_->set_directions(D3DXVECTOR4(0, -1, 0, 0), D3DXVECTOR4(0, 0, 1, 0), D3DXVECTOR4(-1, 0, 0, 0));

	if (camera_controller_)
	{
		//camera_controller_->set_position(camera->get_position());
		//camera_controller_->set_near(camera->get_near());
		//camera_controller_->set_far(camera->get_far());
		//camera_controller_->set_fov(camera->get_fov());
	}

	renderer->set_camera_controller(camera_controller_);

	//init the curve
	{
		animation_curve_points_.push_back(AnimationCurvePoint(D3DXVECTOR3(5, 10, 0) ,  D3DXVECTOR3(0, -1, 1)  , D3DXVECTOR3(0, +1, 1)));
		animation_curve_points_.push_back(AnimationCurvePoint(D3DXVECTOR3(0, 7.5, 5),  D3DXVECTOR3(-1, -1, 0) , D3DXVECTOR3(-1, +1, 0)));
		animation_curve_points_.push_back(AnimationCurvePoint(D3DXVECTOR3(-5, 5, 0),   D3DXVECTOR3(0, -1,-1)  , D3DXVECTOR3(0, +1,-1)));
		animation_curve_points_.push_back(AnimationCurvePoint(D3DXVECTOR3(0, 2.5, -5), D3DXVECTOR3(1, -1, 0),	D3DXVECTOR3(1, +1, 0)));
		animation_curve_points_.push_back(AnimationCurvePoint(D3DXVECTOR3(5, 0, 0),	   D3DXVECTOR3(0, 0, 1),	D3DXVECTOR3(0, +1, 1)));

		//for (int i = 0; i < animation_curve_points_.size(); i++)
		//{
		//	D3DXVECTOR3 minus_tangent = animation_curve_points_[i].tangent_ * -1;
		//	float dot = D3DXVec3Dot(&animation_curve_points_[i].normal_, &minus_tangent);
		//	D3DXVECTOR3 prev_tang = minus_tangent - 2 * dot * animation_curve_points_[i].normal_;
		//	animation_curve_points_[i].previous_tangent_ = prev_tang;
		//}
	}

	mesh_ = resource_manager.get_mesh("plane");
	scene->add_mesh(mesh_);

	cur_index_ = 0;
	cur_u_ = 0;


	//init constant speed u -> x cache
	{
		float du = 0.001;
		int current_index = 0;
		float accumulated_pos = 0;

		for (int current_index = 0; current_index < animation_curve_points_.size() - 1; current_index++)
		{
			D3DXVECTOR3 last_pos = get_current_position_from_curve(current_index, 0);

			float current_u = 0;
			while (current_u < 1.0f)
			{
				D3DXVECTOR3 current_pos = get_current_position_from_curve(current_index, current_u);
				D3DXVECTOR3 distance = current_pos - last_pos;
				float dist = sqrt(D3DXVec3Dot(&distance, &distance));

				accumulated_pos += dist;
				constant_speed_cache_[current_index][current_u] = accumulated_pos;

				current_u += du;
				last_pos = current_pos;
 			}
		}

	}

}

void PlaneAnimationDemo::tick(float dt)
{
	camera_controller_->tick();

	ImGui::Begin("PlaneAnimationDemo");

	float temp = cur_u_;

	ImGui::Checkbox("Do Animation", &move_);
	ImGui::InputFloat("Speed", &dx_);
	ImGui::InputFloat("Cur u", &temp);

	cur_u_ = temp;

	ImGui::End();

	if (move_)
	{
		cur_u_ += dx_;
		//if (cur_u_ > 1.0f)
		//{
		//	cur_u_ -= 1.0f;
		//	cur_index_ = (cur_index_ + 1) % (animation_curve_points_.size() - 1);
		//}

		float real_du = 0;
		int real_index = 0;
		bool found = false;
		float last_distance = 0;
		for (auto index_it = constant_speed_cache_.begin(); index_it != constant_speed_cache_.end() && !found; index_it++)
		{
			float last_u = 0;
			for (auto du_index = index_it->second.begin(); du_index != index_it->second.end(); du_index++)
			{
				if (du_index->second > cur_u_)
				{
					float dist = du_index->second - last_distance;
					float percent = (cur_u_ - last_distance) / dist;
					float u_dist = du_index->first - last_u;

					real_du = last_u + u_dist * percent;
					real_index = index_it->first;
					found = true;
					break;
				}

				last_distance = du_index->second;
				last_u = du_index->first;
			}

		}

		if (!found)
		{
			cur_u_ = 0;
		}

		D3DXMATRIX new_frame;
		new_frame = mesh_->get_frame();

		bool changed = false;

		ImGui::Begin("Matrix");
		ImGui::Columns(3);
		changed |= ImGui::InputFloat("m00", &new_frame.m[0][0]);
		changed |= ImGui::InputFloat("m01", &new_frame.m[0][1]);
		changed |= ImGui::InputFloat("m02", &new_frame.m[0][2]);
		ImGui::NextColumn();
		changed |= ImGui::InputFloat("m10", &new_frame.m[1][0]);
		changed |= ImGui::InputFloat("m11", &new_frame.m[1][1]);
		changed |= ImGui::InputFloat("m12", &new_frame.m[1][2]);
		ImGui::NextColumn();
		changed |= ImGui::InputFloat("m20", &new_frame.m[2][0]);
		changed |= ImGui::InputFloat("m21", &new_frame.m[2][1]);
		changed |= ImGui::InputFloat("m22", &new_frame.m[2][2]);
		ImGui::End();


		mesh_->set_frame(get_current_frame_from_curve(real_index, real_du));

		//if (changed)
		//{
		//	mesh_->set_frame(new_frame);
		//}
	}
}

void PlaneAnimationDemo::on_key_up(char key)
{

}

inline float SIGN(float x)
{
	return (x >= 0.0f) ? +1.0f : -1.0f; 
}

inline float NORM(float a, float b, float c, float d) 
{ 
	return sqrt(a * a + b * b + c * c + d * d); 
}


D3DXVECTOR4 PlaneAnimationDemo::rotation_to_quaternion(D3DMATRIX rot)
{
	float q0, q1, q2, q3;

	float r11 = rot.m[0][0];
	float r12 = rot.m[0][1];
	float r13 = rot.m[0][2];
	float r21 = rot.m[1][0];
	float r22 = rot.m[1][1];
	float r23 = rot.m[1][2];
	float r31 = rot.m[2][0];
	float r32 = rot.m[2][1];
	float r33 = rot.m[2][2];

	q0 = (r11 + r22 + r33 + 1.0f) / 4.0f;
	q1 = (r11 - r22 - r33 + 1.0f) / 4.0f;
	q2 = (-r11 + r22 - r33 + 1.0f) / 4.0f;
	q3 = (-r11 - r22 + r33 + 1.0f) / 4.0f;

	if (q0 < 0.0f)
		q0 = 0.0f;

	if (q1 < 0.0f)
		q1 = 0.0f;

	if (q2 < 0.0f)
		q2 = 0.0f;

	if (q3 < 0.0f)
		q3 = 0.0f;

	q0 = sqrt(q0);
	q1 = sqrt(q1);
	q2 = sqrt(q2);
	q3 = sqrt(q3);

	if (q0 >= q1 && q0 >= q2 && q0 >= q3)
	{
		q0 *= +1.0f;
		q1 *= SIGN(r32 - r23);
		q2 *= SIGN(r13 - r31);
		q3 *= SIGN(r21 - r12);
	}
	else if (q1 >= q0 && q1 >= q2 && q1 >= q3)
	{
		q0 *= SIGN(r32 - r23);
		q1 *= +1.0f;
		q2 *= SIGN(r21 + r12);
		q3 *= SIGN(r13 + r31);
	}
	else if (q2 >= q0 && q2 >= q1 && q2 >= q3)
	{
		q0 *= SIGN(r13 - r31);
		q1 *= SIGN(r21 + r12);
		q2 *= +1.0f;
		q3 *= SIGN(r32 + r23);
	}
	else if (q3 >= q0 && q3 >= q1 && q3 >= q2)
	{
		q0 *= SIGN(r21 - r12);
		q1 *= SIGN(r31 + r13);
		q2 *= SIGN(r32 + r23);
		q3 *= +1.0f;
	}
	else 
	{
		int a = 5;
	}

	float r = NORM(q0, q1, q2, q3);
	q0 /= r;
	q1 /= r;
	q2 /= r;
	q3 /= r;

	return D3DXVECTOR4(q0, q1, q2 , q3);
}

D3DXVECTOR3 PlaneAnimationDemo::get_current_position_from_curve(int cu, float du) const
{
	D3DXVECTOR3 v0 = animation_curve_points_[cu].position_;
	D3DXVECTOR3 v1 = animation_curve_points_[cu + 1].position_;

	// *10 is for better curve
	D3DXVECTOR3 m0 = animation_curve_points_[cu].tangent_ * 10;
	D3DXVECTOR3 m1 = animation_curve_points_[cu + 1].tangent_ * 10;

	float u_square = du * du;
	float u_cube = u_square * du;

	return (2 * u_cube - 3 * u_square + 1) * v0 + (u_cube - 2 * u_square + du) * m0 + (-2 * u_cube + 3 * u_square) * v1 + (u_cube - u_square) * m1;
}

D3DMATRIX PlaneAnimationDemo::get_current_frame_from_curve(int cu, float du) const
{
	D3DXVECTOR3 pos = get_current_position_from_curve(cu, du);

	D3DXVECTOR3 m0 = animation_curve_points_[cu].tangent_;
	D3DXVECTOR3 m1 = animation_curve_points_[cu + 1].tangent_;

	D3DXVECTOR3 n0 = animation_curve_points_[cu].normal_;
	D3DXVECTOR3 n1 = animation_curve_points_[cu + 1].normal_;

	//linear interpolation for the rotation
	D3DXVECTOR3 current_tangent = m0 * (1.0f - du) + m1 * du;
	D3DXVECTOR3 current_normal = n0 * (1.0f - du) + n1 * du;
	D3DXVec3Normalize(&current_tangent, &current_tangent);
	D3DXVec3Normalize(&current_normal, &current_normal);

	D3DXVECTOR3 current_right;
	D3DXVec3Cross(&current_right, &current_tangent, &current_normal);
	D3DXVec3Normalize(&current_right, &current_right);

	float scale = 1;

	D3DXVECTOR4 pos4 = D3DXVECTOR4(pos.x, pos.y, pos.z, 1);

	D3DXMATRIX new_frame;
	D3DXMatrixIdentity(&new_frame);
	//new_frame.m[1][0] =  current_tangent.x * scale;
	//new_frame.m[1][1] =  current_tangent.y * scale;
	//new_frame.m[1][2] =  current_tangent.z * scale;
	//
	//new_frame.m[2][0] = current_normal.x * scale;
	//new_frame.m[2][1] = current_normal.y * scale;
	//new_frame.m[2][2] = current_normal.z * scale;
	//			
	//new_frame.m[0][0] = current_right.x * scale;
	//new_frame.m[0][1] = current_right.y * scale;
	//new_frame.m[0][2] = current_right.z * scale;

	//ImGui::Begin("Matrix");
	//ImGui::Columns(3);
	//ImGui::InputFloat("m00", &new_frame.m[0][0]);
	//ImGui::InputFloat("m01", &new_frame.m[0][1]);
	//ImGui::InputFloat("m02", &new_frame.m[0][2]);
	//ImGui::NextColumn();
	//ImGui::InputFloat("m10", &new_frame.m[1][0]);
	//ImGui::InputFloat("m11", &new_frame.m[1][1]);
	//ImGui::InputFloat("m12", &new_frame.m[1][2]);
	//ImGui::NextColumn();
	//ImGui::InputFloat("m20", &new_frame.m[2][0]);
	//ImGui::InputFloat("m21", &new_frame.m[2][1]);
	//ImGui::InputFloat("m22", &new_frame.m[2][2]);
	//ImGui::End();

	//
	//D3DXMATRIX inverse_frame; float det;
	//D3DXMatrixInverse(&inverse_frame, &det, &new_frame);
	//D3DXVec4Transform(&pos4, &pos4, &inverse_frame);

	//new_frame.m[3][0] = pos4.x;
	//new_frame.m[3][1] = pos4.y;
	//new_frame.m[3][2] = pos4.z;

	D3DXVECTOR4 cur_quaternion = animation_curve_points_[cu].quaternion_ * (1.0f - du) + animation_curve_points_[cu + 1].quaternion_ * (du);

	D3DXVECTOR3 v1, v2, v3;
	rotate_vector_by_quaternion(D3DXVECTOR3(1, 0, 0), cur_quaternion, v1);
	rotate_vector_by_quaternion(D3DXVECTOR3(0, 1, 0), cur_quaternion, v2);
	rotate_vector_by_quaternion(D3DXVECTOR3(0, 0, 1), cur_quaternion, v3);

	D3DXVec3Normalize(&v1, &v1);
	D3DXVec3Normalize(&v2, &v2);
	D3DXVec3Normalize(&v3, &v3);

	new_frame.m[1][0] =  v1.x * scale;
	new_frame.m[1][1] =  v1.y * scale;
	new_frame.m[1][2] =  v1.z * scale;
	
	new_frame.m[2][0] = v2.x * scale;
	new_frame.m[2][1] = v2.y * scale;
	new_frame.m[2][2] = v2.z * scale;
				
	new_frame.m[0][0] = v3.x * scale;
	new_frame.m[0][1] = v3.y * scale;
	new_frame.m[0][2] = v3.z * scale;

	new_frame.m[3][0] = pos4.x;
	new_frame.m[3][1] = pos4.y;
	new_frame.m[3][2] = pos4.z;

	return new_frame;
}

PlaneAnimationDemo::AnimationCurvePoint::AnimationCurvePoint(D3DXVECTOR3 pos, D3DXVECTOR3 tang, D3DXVECTOR3 norm) : position_(pos), tangent_(tang), normal_(norm)
{
	D3DXVec3Normalize(&tangent_, &tang);
	D3DXVec3Normalize(&normal_, &norm);

	D3DXVECTOR3 current_right;
	D3DXVec3Cross(&current_right, &tangent_, &normal_);
	D3DXVec3Normalize(&current_right, &current_right);


	D3DXMATRIX rotation_frame;
	D3DXMatrixIdentity(&rotation_frame);
	rotation_frame.m[0][0] = tangent_.x;
	rotation_frame.m[0][1] = tangent_.y;
	rotation_frame.m[0][2] = tangent_.z;
	
	rotation_frame.m[1][0] = normal_.x;
	rotation_frame.m[1][1] = normal_.y;
	rotation_frame.m[1][2] = normal_.z;
				
	rotation_frame.m[2][0] = current_right.x;
	rotation_frame.m[2][1] = current_right.y;
	rotation_frame.m[2][2] = current_right.z;

	quaternion_ = PlaneAnimationDemo::rotation_to_quaternion(rotation_frame);
}
