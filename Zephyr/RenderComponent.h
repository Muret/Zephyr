#ifndef __INCLUDE_RENDER_COMPONENT
#define __INCLUDE_RENDER_COMPONENT


class RenderComponent
{

public:
	RenderComponent();
	
	virtual void pre_render() = 0;
	virtual void post_render() = 0;
	virtual void post_gbuffer_render() = 0;

	virtual bool uses_postfx() const = 0;
private:


};





#endif