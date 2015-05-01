#pragma once

#include <algorithm>

#include "Vector2.h"

#define INFINITY 3.402823466e+38F

using std::min;
using std::max;

class BoundingBox {
public:
	Vector2 pMin, pMax; //min and max coords of the bounds

  BoundingBox() {
    pMin = Vector2(INFINITY, INFINITY);
    pMax = Vector2(-INFINITY, -INFINITY);
  }
	BoundingBox(Vector2& p) {
		pMin = Vector2(p.x, p.y);
		pMax = Vector2(p.x, p.y);
  }
  BoundingBox(Vector2& p1, Vector2& p2) {
		pMin = Vector2(min(p1.x, p2.x),
									 min(p1.y, p2.y));
		pMax = Vector2(max(p1.x, p2.x),
									 max(p1.y, p2.y));
  }
  ~BoundingBox() { }

  float getBound(int axis);

  // check if two boxes overlap
	bool overlaps(BoundingBox &b)
	{
		bool x = (pMax.x >= b.pMin.x) && (pMin.x <= b.pMax.x);
		bool y = (pMax.y >= b.pMin.y) && (pMin.y <= b.pMax.y);

		return x && y;
	}

	bool overlaps(Vector2& p0, Vector2& p1)
	{
    /*return ((p0.x >= pMin.x && p0.x <= pMax.x &&
              p0.y >= pMin.y && p0.y <= pMax.y) ||
						 (p1.x >= pMin.x && p1.x <= pMax.x &&
              p1.y >= pMin.y && p1.y <= pMax.y));
		*/
		//compare line segment against each slab
		float p0t, p1t, fp0t = 0, fp1t = 1; 
		//check x
		if (p0.x < p1.x)
		{
			if (p0.x > pMax.x || p1.x < pMin.x)
				return false;
			float invdx = 1.0f / (p1.x - p0.x);
			p0t = (p0.x < pMin.x) ? (pMin.x - p0.x) * invdx : 0;
			p1t = (p1.x > pMax.x) ? (pMax.x - p0.x) * invdx : 1;
		}
		else
		{
			if (p1.x > pMax.x || p0.x < pMin.x)
				return false;
			float invdx = 1.0f / (p1.x - p0.x);
			p0t = (p0.x > pMax.x) ? (pMax.x - p0.x) * invdx : 0;
			p1t = (p1.x < pMin.x) ? (pMin.x - p0.x) * invdx : 1;
		}
		if (p0t > fp0t) fp0t = p0t;
		if (p1t < fp1t) fp1t = p1t;
		if (fp1t < fp0t) return false;

		//check y
		if (p0.y < p1.y)
		{
			if (p0.y > pMax.y || p1.y < pMin.y)
				return false;
			float invdy = 1.0f / (p1.y - p0.y);
			p0t = (p0.y < pMin.y) ? (pMin.y - p0.y) * invdy : 0;
			p1t = (p1.y > pMax.y) ? (pMax.y - p0.y) * invdy : 1;
		}
		else
		{
			if (p1.y > pMax.y || p0.y < pMin.y)
				return false;
			float invdy = 1.0f / (p1.y - p0.y);
			p0t = (p0.y > pMax.y) ? (pMax.y - p0.y) * invdy : 0;
			p1t = (p1.y < pMin.y) ? (pMin.y - p0.y) * invdy : 1;
		}
		if (p0t > fp0t) fp0t = p0t;
		if (p1t < fp1t) fp1t = p1t;
		if (fp1t < fp0t) return false;

		return true;
	}

  // check if the point exists inside the box
  bool inside(Vector2& p)
	{
    return (p.x >= pMin.x && p.x <= pMax.x &&
             p.y >= pMin.y && p.y <= pMax.y);
  }

	//check if the line is inside the box
  bool inside(Vector2& p0, Vector2& p1)
	{
    return (p0.x >= pMin.x && p0.x <= pMax.x &&
             p0.y >= pMin.y && p0.y <= pMax.y &&
						 p1.x >= pMin.x && p1.x <= pMax.x &&
             p1.y >= pMin.y && p1.y <= pMax.y);
  }

  // make the box bigger
	void expand(float delta)
	{
		pMin -= Vector2(delta, delta);
		pMax += Vector2(delta, delta);
	}

  // compute the volume of the box
	float volume()
	{
		Vector2 d = pMax - pMin;
		return d.x * d.y;
	}

  // get the direction of the longest extent of the box (x, y or z)
	int maximumExtent()
	{
		Vector2 diag = pMax - pMin;
		if (diag.x > diag.y)
			return 0; //x max
		else
			return 1; //y max
	}

	void move(Vector2& p)
	{
		pMin += p;
		pMax += p;
	}

  void merge(Vector2& p)
	{
		pMin.x = min(pMin.x, p.x);
		pMin.y = min(pMin.y, p.y);
		pMax.x = max(pMax.x, p.x);
		pMax.y = max(pMax.y, p.y);
	}

  void merge(BoundingBox& b2) {
		pMin.x = min(pMin.x, b2.pMin.x);
		pMin.y = min(pMin.y, b2.pMin.y);
		pMax.x = max(pMax.x, b2.pMax.x);
		pMax.y = max(pMax.y, b2.pMax.y);
	}

  /*// compute the bounding sphere of the box
  void boundingSphere(Point *center, float *rad) const {
    *center = pMin * 0.5f + pMax * 0.5f;
    *rad    = distance(*center, pMax);
  }*/

};
