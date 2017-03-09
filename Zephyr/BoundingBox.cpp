
#include "BoundingBox.h"


BoundingBox::BoundingBox()
{
	reset();
}

BoundingBox::~BoundingBox()
{
}

void BoundingBox::enlarge_bb_with_point(const D3DXVECTOR4 & position)
{
	min_.x = min(position.x, min_.x);
	min_.y = min(position.y, min_.y);
	min_.z = min(position.z, min_.z);

	max_.x = max(position.x, max_.x);
	max_.y = max(position.y, max_.y);
	max_.z = max(position.z, max_.z);
}

void BoundingBox::reset()
{
	min_ = D3DXVECTOR4(FLT_MAX, FLT_MAX, FLT_MAX , 1);
	max_ = D3DXVECTOR4(FLT_MIN, FLT_MIN, FLT_MIN , 1);
}

D3DXVECTOR4 BoundingBox::get_min() const
{
	return min_;
}

D3DXVECTOR4 BoundingBox::get_max() const
{
	return max_;
}

void BoundingBox::enlarge_bb_with_bb(const BoundingBox & bb)
{
	enlarge_bb_with_point(bb.get_max());
	enlarge_bb_with_point(bb.get_min());
}

float BoundingBox::get_sphere_radius() const
{
	D3DXVECTOR3 dist = max_ - min_;
	return max(max(dist.x, dist.y), dist.z);
}
