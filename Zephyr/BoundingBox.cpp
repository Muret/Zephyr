
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

D3DXVECTOR4 BoundingBox::get_center() const
{
	return (max_ + min_) * 0.5f;
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

void BoundingBox::get_points(vector<D3DXVECTOR4>& get_points) const
{
	D3DXVECTOR4 mid = (min_ + max_) * 0.5;
	D3DXVECTOR4 dif = (min_ - max_) * 0.5;

	get_points.push_back(mid + D3DXVECTOR4(dif.x *-1.0f, dif.y *-1.0f, dif.z *-1.0f, 0));
	get_points.push_back(mid + D3DXVECTOR4(dif.x *-1.0f, dif.y *+1.0f, dif.z *-1.0f, 0));
	get_points.push_back(mid + D3DXVECTOR4(dif.x *+1.0f, dif.y *-1.0f, dif.z *-1.0f, 0));
	get_points.push_back(mid + D3DXVECTOR4(dif.x *+1.0f, dif.y *+1.0f, dif.z *-1.0f, 0));

	get_points.push_back(mid + D3DXVECTOR4(dif.x *-1.0f, dif.y *-1.0f, dif.z *+1.0f, 0));
	get_points.push_back(mid + D3DXVECTOR4(dif.x *-1.0f, dif.y *+1.0f, dif.z *+1.0f, 0));
	get_points.push_back(mid + D3DXVECTOR4(dif.x *+1.0f, dif.y *-1.0f, dif.z *+1.0f, 0));
	get_points.push_back(mid + D3DXVECTOR4(dif.x *+1.0f, dif.y *+1.0f, dif.z *+1.0f, 0));
}

void BoundingBox::transform_by_matrix(const D3DXMATRIX & matrix)
{
	BoundingBox new_bb;

	vector<D3DXVECTOR4> points;
	get_points(points);

	for (int i = 0; i < points.size(); i++)
	{
		D3DXVECTOR4 new_pos;
		D3DXVec4Transform(&new_pos, &points[i], &matrix);

		new_bb.enlarge_bb_with_point(new_pos);
	}

	min_ = new_bb.min_;
	max_ = new_bb.max_;
}
