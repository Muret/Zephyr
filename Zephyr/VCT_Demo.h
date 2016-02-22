#ifndef _VCT_DEMO_H
#define _VCT_DEMO_H

#include "includes.h"
#include "Demo.h"

class GPUVCT;
class FreeCameraController;

class VCTDemo : public DemoBase
{
public:
	VCTDemo();
	virtual ~VCTDemo() override;

	virtual void initialize() override;
	virtual void tick(float dt)  override;

	virtual void on_key_up(char key) override;

private:
	GPUVCT *vct_;
	FreeCameraController *camera_controller_;

};

#endif