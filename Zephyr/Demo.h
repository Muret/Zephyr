#ifndef _DEMO_BASE_H
#define _DEMO_BASE_H


#include "includes.h"
#include "FBXSceneImporter.h"

void init_demo_scene();

class DemoBase
{
public:
	
	DemoBase(std::string name);
	virtual ~DemoBase();

	virtual void initialize() = 0;
	virtual void tick(float dt) = 0;

	virtual void on_key_up(char key) = 0;

private:
	std::string name_;

};



#endif