#pragma once

#include <vector>

#include "Vector2.h"
#include "BoundingBox.h"
#include "Stroke.h"

#define uint unsigned int

using std::vector;

struct StrokeExtent
{
	Stroke* strokeID;
	uint minCP, maxCP;
	vector<ControlPoint> points;

	StrokeExtent(Stroke* id, uint min, uint max)
	{
		strokeID = id;
		minCP = min;
		maxCP = max;
	}
	~StrokeExtent() {}
};

struct Radial
{
	Vector2 pos;
	float radius;
	float softness;

	Radial(Vector2& p, float rad, float soft);
	~Radial();

	float pushPoint(Vector2& p, Vector2& parentPos);
};
/*
struct Triangle
{

};
*/
class MoveBrush
{
public:
	Vector2 pos;
	Vector2 guiPos;
	BoundingBox bounds;
	vector<Radial> radials;

	MoveBrush();
	~MoveBrush();

	//calculates new locations of the points after being pushed by the brush
	void pushPoints(float dx, float dy, vector<ControlPoint>& points);

	void addRadial(Radial& rad);

	void setPosition(float x, float y, float gx, float gy);
	void scaleBrush(float scale, float maxWidth);
	void scaleSoftness(float softScale);
	void drawCanvas(float scale);
	void drawMenu(Vector2 pos);
	void draw(Vector2 pos, float scale, float c[]);
	void drawOutline(float scale);
};