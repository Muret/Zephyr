#ifndef INCLUDE_BOUNDINGBOX_
#define INCLUDE_BOUNDINGBOX_

#include "includes.h"

class BoundingBox
{
public:

	BoundingBox();
	~BoundingBox();

	void enlarge_bb_with_point(const D3DXVECTOR4 &position);
	void reset();

	D3DXVECTOR4 get_min() const;
	D3DXVECTOR4 get_max() const;

	void enlarge_bb_with_bb(const BoundingBox &bb);

private:

	D3DXVECTOR4 min_;
	D3DXVECTOR4 max_;
};

#endif