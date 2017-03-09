#ifndef _SSR_PLAINANIMATIONDEMO_H
#define _SSR_PLAINANIMATIONDEMO_H

#include "includes.h"
#include "Demo.h"

class FreeCameraController;
class Camera;

class PlaneAnimationDemo : public DemoBase
{
	struct AnimationCurvePoint
	{
		AnimationCurvePoint(D3DXVECTOR3 pos, D3DXVECTOR3 tang, D3DXVECTOR3 norm);

		D3DXVECTOR3 position_;
		D3DXVECTOR3 tangent_;
		D3DXVECTOR3 normal_;

		//D3DXVECTOR3 previous_tangent_;

		D3DXVECTOR4 quaternion_;
	};

public:
	

	PlaneAnimationDemo();
	virtual ~PlaneAnimationDemo() override;

	virtual void initialize() override;
	virtual void tick(float dt)  override;

	virtual void on_key_up(char key) override;

	static D3DXVECTOR4 rotation_to_quaternion(D3DMATRIX rot);

	static void rotate_vector_by_quaternion(const D3DXVECTOR3& v, const D3DXVECTOR4& q, D3DXVECTOR3& vprime)
	{
		// Extract the vector part of the quaternion
		D3DXVECTOR3 u(q.x, q.y, q.z);

		// Extract the scalar part of the quaternion
		float s = q.w;

		float dotuv = D3DXVec3Dot(&u, &v);
		float dotuu = D3DXVec3Dot(&u, &u);

		D3DXVECTOR3 crossuv;
		D3DXVec3Cross(&crossuv, &u, &v);

		// Do the math
		vprime = 2.0f * dotuv * u
			+ (s*s - dotuu) * v
			+ 2.0f * s * crossuv;
	}

private:
	Camera *camera_controller_;

	std::vector<AnimationCurvePoint> animation_curve_points_;

	int cur_index_;
	double cur_u_;

	Mesh *mesh_;

	bool move_;
	float dx_;

	std::map<int, map<double, double>> constant_speed_cache_;

	D3DXVECTOR3 get_current_position_from_curve(int cu, float du) const;
	D3DMATRIX get_current_frame_from_curve(int cu, float du) const;
};

#endif