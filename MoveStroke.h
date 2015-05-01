#pragma once

#include <string>
#include <sstream>
#include <iostream>
#include <vector>

#include "Stroke.h"
#include "BoundingBox.h"

using std::vector;

#define uint unsigned int
#define NUMMOVEDIR 20

class MoveStroke
{
public:
	vector<Vector2> p; //points of the stroke
	vector<Vector2> rotP; //rotated point set
	vector<float> r;
	//vector<Vector2> n; //outward pointing normals of the stroke

	GLfloat* vertices;

	vector<Vector2> debugP0;
	vector<Vector2> debugP1;
	vector<Vector2> debugP2;

	Vector2 pos;
	Vector2 guiPos;
	Vector2 dir[NUMMOVEDIR];

	BoundingBox bounds;

	MoveStroke();
	MoveStroke(Stroke* stroke);
	MoveStroke(vector<GLfloat>& stroke);

	~MoveStroke();

	void setPosition(float x, float y, float gx, float gy);
	void setDirection(float nx, float ny);

	void pushPoints(float dx, float dy, vector<ControlPoint>& points);

	bool triangleContribution(Vector2& p, uint indx);

	void draw(float zoom);
};