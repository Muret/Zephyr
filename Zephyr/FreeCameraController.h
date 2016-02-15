
#ifndef _FREECAMERACONTROLLER_H_
#define _FREECAMERACONTROLLER_H_

#include "Camera.h"

class FreeCameraController : public Camera
{

public:
	FreeCameraController();

	virtual void tick() override;

	void recompute_rotation();

	virtual void set_frame(const D3DXMATRIX& value) override;

private:
	float yaw_;
	float pitch_;

	static const D3DXVECTOR4 default_forward_;
	static const D3DXVECTOR4 default_up_;
	static const D3DXVECTOR4 default_right_;

};

#endif