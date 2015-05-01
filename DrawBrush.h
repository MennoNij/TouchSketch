#pragma once

#include <string>
#include <sstream>
#include <iostream>

#include "Vector2.h"

enum varyBy { VARYNONE, VARYSIZE, VARYALPHA };

class DrawBrush {
public:
	float radius;
	float colour[3];
	//int varyAlphaWith;
	//int varySizeWith;
	int varyWithSpeed;
	int varyWithPressure;
	bool varPressure;
	bool varDyna;
	bool varAlpha;
	Vector2 guiPos;

	//dynadraw variables
	float px, py;    // current position of spring
	float vx, vy;    // current velocity
	float ppx, ppy;  // our previous position
	float k;         // bounciness, stiffness of spring
	float damping;   // friction
	float ductus;    // this constant relates stroke width to speed
	float max_th;    // maximum stroke thickness
	float mass;      // mass of simulated pen
	float maxVel;    // maximum measured velocity

	DrawBrush();
	DrawBrush(float rad, int varyPress, int varySpeed, float r, float g, float b);
	~DrawBrush();

	//samples the brush as a radius
	float sampleBrush(float x, float y, float dx, float dy, float pressure, float& sampleAlpha, float c[3]);
	void setBrushPos(float x, float y);
	void setBrushRadius(float rad);
	void calcDynaVars(float x, float y);
	float calcDynaContribution();
	void setPosition(float x, float y);
	void draw(Vector2 pos, float scale, float c[]);
	void drawMenu(Vector2 pos);
	void drawCanvas(float scale);

	void drawOutline(float scale);
};