#include "MoveBrush.h"

#include <windows.h>
#include <GL/gl.h>
#include <iostream>

Radial::Radial(Vector2& p, float rad, float soft)
{
	pos = p;
	radius = rad;
	softness = soft;
}

Radial::~Radial() {}

float Radial::pushPoint(Vector2& p, Vector2& parentPos)
{
	//compute distance between point and radial
	Vector2 canvasPos = pos + parentPos;
	Vector2 dist = canvasPos - p;
	float d = dist.length();

	float y0 = radius/softness; 

	//find contribution of radial to movement of p
	float y = y0 - (y0*(d/radius));
	float a = y / radius;
	//float a = falloff - (falloff/radius)*d;
	if (a<0)
		return 0.0f;
	else if (a>1.0f)
		return 1.0f;
	else
		return a;
}

MoveBrush::MoveBrush()
{
}

MoveBrush::~MoveBrush() {}

void MoveBrush::pushPoints(float dx, float dy, vector<ControlPoint>& points)
{
	Vector2 d = Vector2(dx,dy);
	for (uint i=0; i<points.size(); ++i)
	{
		float ap = 0.0f; //percentage of movement transferred to the point
		//calculate influence of each radial
		for (uint j=0; j<radials.size(); ++j)
			ap += radials[j].pushPoint(points[i].p, pos);
		
		if (ap > 1.0f) ap = 1.0f;

		//move point
		points[i].p = points[i].p + d*ap;
	}
}

void MoveBrush::addRadial(Radial& rad)
{
	//keep the bounds of the brush accurate
	Vector2 r0 = Vector2(rad.pos.x-rad.radius, rad.pos.y-rad.radius);
	Vector2 r1 = Vector2(rad.pos.x+rad.radius, rad.pos.y+rad.radius);

	BoundingBox radBound = BoundingBox(r0, r1);
	bounds.merge(radBound);

	radials.push_back(rad);
}

void MoveBrush::setPosition(float x, float y, float gx, float gy)
{
	pos = Vector2(x,y);
	guiPos = Vector2(gx, gy);
}

void MoveBrush::drawCanvas(float scale)
{
	float c[3] = {1.0f, 0.0f, 0.0f};
	draw(guiPos, scale, c);
}

void MoveBrush::drawMenu(Vector2 pos)
{
	float c[3] = {0.0f, 0.0f, 0.0f};
	draw(pos, 1.0f, c);
}

void MoveBrush::scaleBrush(float scale, float maxWidth)
{
	//scale bounds, check if it doesn't become too large
	float width = bounds.pMax.x - bounds.pMin.x;
	float height = bounds.pMax.y - bounds.pMin.y;

	//std::cout<<"scaled: "<<width*scale<<","<<height*scale<<"\n";

	if (width*scale > maxWidth || height*scale > maxWidth || width*scale < 1.0f || height*scale < 1.0f)
		return;

	bounds = BoundingBox();

	//scale brush components
	for (uint i=0; i<radials.size(); ++i)
	{
		radials[i].radius *= scale;
		Vector2 r0 = Vector2(radials[i].pos.x-radials[i].radius, radials[i].pos.y-radials[i].radius);
		Vector2 r1 = Vector2(radials[i].pos.x+radials[i].radius, radials[i].pos.y+radials[i].radius);
		BoundingBox radBound = BoundingBox(r0, r1);
		bounds.merge(radBound);
	}
}

void MoveBrush::scaleSoftness(float softScale)
{
	for (uint i=0; i<radials.size(); ++i)
	{
		float newSoft = radials[i].softness*softScale;
		if (newSoft > 1.0f)
			newSoft = 1.0f;
		radials[i].softness = newSoft;
	}
}

void MoveBrush::draw(Vector2 pos, float scale, float c[])
{
	BoundingBox b = bounds;
	b.move(pos);

	for (uint i=0; i<radials.size(); ++i)
	{
		float x = radials[i].pos.x + pos.x;
		float y = radials[i].pos.y + pos.y;
		//float f = (radials[i].softness-1)/(radials[i].softness/radials[i].radius);
		float y0 = radials[i].radius / radials[i].softness;
		float f = -((radials[i].radius-y0)*radials[i].radius) / y0;
		float inner = f*scale;
		float radius = radials[i].radius*scale;
		float prevX = radius*cos(0.0f) + x;
		float prevY = radius*sin(0.0f) + y;

		int segs = 40;
		float coef = 2.0f*PI/(segs-1);

		float rads;
		glBegin(GL_TRIANGLE_STRIP);
		for (int n=0; n<segs; ++n)
		{
			rads = n*coef;

			//outer circle
			glColor4f(c[0],c[1],c[2],0.5);
			glVertex2f(inner*cos(rads) + x, inner*sin(rads) + y);

			glColor4f(c[0],c[1],c[2],0.5);
			glVertex2f(inner*cos(rads+coef) + x, inner*sin(rads+coef) + y);

			glColor4f(c[0],c[1],c[2],0);
			glVertex2f((radius)*cos(rads) + x, (radius)*sin(rads) + y);
      
			glColor4f(c[0],c[1],c[2],0);
			glVertex2f((radius)*cos(rads+coef) + x, (radius)*sin(rads+coef) + y);
		}
		glEnd();

		glBegin(GL_TRIANGLE_STRIP);
		for (int n=0; n<segs; ++n)
		{
			rads = n*coef;

			//inner circle     
			glColor4f(c[0],c[1],c[2],0.5);
			glVertex2f(x,y);

			glColor4f(c[0],c[1],c[2],0.5);
			glVertex2f(inner*cos(rads) + x, inner*sin(rads) + y);

			glColor4f(c[0],c[1],c[2],0.5);
			glVertex2f(inner*cos(rads+coef) + x, inner*sin(rads+coef) + y);
		}
		glEnd();

	}
}

void MoveBrush::drawOutline(float scale)
{
	float c[3] = {1.0f, 0.0f, 0.0f};
	Vector2 pos = guiPos;

	for (uint i=0; i<radials.size(); ++i)
	{
		float x = radials[i].pos.x + pos.x;
		float y = radials[i].pos.y + pos.y;
		//float f = (radials[i].softness-1)/(radials[i].softness/radials[i].radius);
		float y0 = radials[i].radius / radials[i].softness;
		float f = -((radials[i].radius-y0)*radials[i].radius) / y0;
		float inner = f*scale;
		float radius = radials[i].radius*scale;
		float prevX = radius*cos(0.0f) + x;
		float prevY = radius*sin(0.0f) + y;

		int segs = 40;
		float coef = 2.0f*PI/(segs-1);

		float rads;
		glColor4f(c[0],c[1],c[2],0.5);
		glBegin(GL_LINE_STRIP);
		for (int n=0; n<segs; ++n)
		{
			rads = n*coef;

			//outer circle

			glColor4f(c[0],c[1],c[2],1);
			glVertex2f((radius)*cos(rads) + x, (radius)*sin(rads) + y);
      
			glColor4f(c[0],c[1],c[2],1);
			glVertex2f((radius)*cos(rads+coef) + x, (radius)*sin(rads+coef) + y);
		}
		glEnd();
	}
}

