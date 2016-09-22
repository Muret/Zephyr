#ifndef _GPU_BASED_PIPLINE_DEMO_H
#define _GPU_BASED_PIPLINE_DEMO_H

#include "includes.h"
#include "Demo.h"
#include "GPUBasedPipeline.h"
#include "GUI.h"

class FreeCameraController;

class GPUBasedPipelineDemo : public DemoBase
{
public:
	GPUBasedPipelineDemo();
	virtual ~GPUBasedPipelineDemo() override;

	virtual void initialize() override;
	virtual void tick(float dt)  override;

	virtual void on_key_up(char key) override;

private:

	FreeCameraController *camera_controller_;
	GPUBasedPipeline::GPUBasedRenderer *renderer_;
	void init_state();

	bool show_test_window;
	bool show_another_window;
	ImVec4 clear_col;

};

#endif