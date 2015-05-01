#include "DrawBrush.h"

#include <cmath>
#include <algorithm>

#include <windows.h>
#include <GL/gl.h>

#include "Vector2.h"

DrawBrush::DrawBrush()
{
	radius = 1.0f;
	//varySizeWith = PRESSURE;
	//varyAlphaWith = CONSTANT;
	varyWithPressure = VARYALPHA;
	varyWithSpeed = VARYNONE;
	colour[0] = 0.0f;
	colour[1] = 0.0f;
	colour[2] = 0.0f;
}

DrawBrush::DrawBrush(float rad, int varyPress, int varySpeed, float r, float g, float b)
{
	radius = rad;
	//varySizeWith = varySize;
	//varyAlphaWith = varyAlph;
	varyWithPressure = varyPress;
	varyWithSpeed = varySpeed;

	colour[0] = r;
	colour[1] = g;
	colour[2] = b;

	//dyna stuff
	k = 0.06;
	damping = 0.68;
	ductus = 1.0;
	max_th = 10.0;
	mass = 1;
	maxVel = 100.0;
}

DrawBrush::~DrawBrush()
{
}

float DrawBrush::sampleBrush(float x, float y, float dx, float dy, float pressure, float& sampleAlpha, float c[3])
{
	float sampleSize = 0.0f;

	calcDynaVars(x, y);

	sampleSize = radius;
	sampleAlpha = 1.0f;

	if (varyWithPressure == VARYSIZE)
		sampleSize = radius*pressure;
	else if (varyWithPressure == VARYALPHA)
		sampleAlpha = myMin(pressure, 1.0f);

	if (varyWithSpeed == VARYSIZE)
		sampleSize = radius*calcDynaContribution();
	else if (varyWithSpeed == VARYALPHA)
		sampleAlpha = myMin(1.0f*calcDynaContribution(), 1.0f);

	c[0] = colour[0]; c[1] = colour[1]; c[2] = colour[2];
	return sampleSize;
}

void DrawBrush::setBrushPos(float x, float y)
{
	px = x;
	py = y;
	guiPos = Vector2(x, y);
}

void DrawBrush::setBrushRadius(float rad)
{
	radius = rad;
}

void DrawBrush::calcDynaVars(float x, float y)
{
	float dy = py - x;   // Compute displacement from the cursor
	float dx = px - y;
	
	float fx = -k * dx;       // Hooke's law, Force = - k * displacement
	float fy = -k * dy;
	
	float ay = fy / mass;     // Acceleration, computed from F = ma
	float ax = fx / mass;
	
	vx = vx + ax;             // Integrate once to get the next
	vy = vy + ay;             // velocity from the acceleration
	vx = vx * damping;        // Apply damping, which is a force
	vy = vy * damping;        // negatively proportional to velocity
	px = px + vx;             // Integrate a second time to get the
	py = py + vy;             // next position from the velocity
}

float DrawBrush::calcDynaContribution()
{
	//still doesn't work properly...
	float vh = sqrt(vx*vx + vy*vy);              // Compute the (Pythagorean) velocity,

	float th = max_th - myMin(vh*ductus, max_th);  // which we use (scaled, clamped and
	
	//given max velocity, at what % is vh*ductus?
	float vhPerc = (vh*ductus)/maxVel;
	float perc = 1-myMin(1.0f, vhPerc);
	float contrib = myMax(0.01f, perc);
	
	std::cout<<"th: "<<th<<" max_th: "<<max_th<<" vh: "<<vh<<" vh*ductus: "<<vh*ductus<<" vhPerc: "<<vhPerc<<" perc: "<<perc<<"\n";	
	th = myMax(1.0f, th);                           // inverted) in computing...



	return contrib;
}

void DrawBrush::setPosition(float x, float y)
{
	guiPos = Vector2(x, y);
}

void DrawBrush::draw(Vector2 pos, float scale, float c[])
{
	float r = radius * scale;
	float x = pos.x;
	float y = pos.y;

	float prevX = r*cos(0.0f) + x;
	float prevY = r*sin(0.0f) + y;

	int segs = 40;
	float coef = 2.0f*PI/(segs-1);

	float rads;
	glBegin(GL_TRIANGLE_STRIP);
	for (int n=0; n<segs; ++n)
	{
		rads = n*coef;

		//inner circle     
		glColor4fv(c);
		glVertex2f(x,y);
		glVertex2f(r*cos(rads) + x, r*sin(rads) + y);
		glVertex2f(r*cos(rads+coef) + x, r*sin(rads+coef) + y);
	}
	glEnd();
}

void DrawBrush::drawMenu(Vector2 pos)
{
	float c[4] = {0.0f, 0.0f, 0.0f, 1.0f};
	draw(pos, 1.0f, c);
}

void DrawBrush::drawCanvas(float scale)
{
	float c[4] = {1.0f, 0.0f, 0.0f, 0.7f};
	draw(guiPos, scale, c);
}

void DrawBrush::drawOutline(float scale)
{
	float c[4] = {1.0f, 0.0f, 0.0f, 0.7f};
	Vector2 pos = guiPos;

	float r = radius * scale;
	float x = pos.x;
	float y = pos.y;

	float prevX = r*cos(0.0f) + x;
	float prevY = r*sin(0.0f) + y;

	int segs = 40;
	float coef = 2.0f*PI/(segs-1);

	float rads;
	glBegin(GL_LINE_STRIP);
	for (int n=0; n<segs; ++n)
	{
		rads = n*coef;

		//inner circle     
		glColor4fv(c);
		//glVertex2f(x,y);
		glVertex2f(r*cos(rads) + x, r*sin(rads) + y);
		glVertex2f(r*cos(rads+coef) + x, r*sin(rads+coef) + y);
	}
	glEnd();
}
