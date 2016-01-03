#ifndef _SSR_DEMO_H
#define _SSR_DEMO_H

#include "includes.h"
#include "Demo.h"

class SSR;

class SSRDemo : public DemoBase
{
public:
	SSRDemo();
	virtual ~SSRDemo() override;

	virtual void initialize() override;
	virtual void tick(float dt)  override;

private:
	SSR *ssr_component;
};

#endif