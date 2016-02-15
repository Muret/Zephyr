#ifndef _SSR_DEMO_H
#define _SSR_DEMO_H

#include "includes.h"
#include "Demo.h"

class SSR;
class FreeCameraController;

class SSRDemo : public DemoBase
{
public:
	SSRDemo();
	virtual ~SSRDemo() override;

	virtual void initialize() override;
	virtual void tick(float dt)  override;

	virtual void on_key_up(char key) override;


private:
	SSR *ssr_component;
	FreeCameraController *camera_controller_;
};

#endif