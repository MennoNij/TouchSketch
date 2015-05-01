#pragma once

#include "Vector2.h"

struct TapWidget
{
	int diameter;
	int duration; //duration in miliseconds
	float visibility;
	unsigned long activationTime;
	Vector2 pos;
	bool active;

	TapWidget();
	~TapWidget();

	void enable(float x, float y, unsigned long actTime);
	void disable();
	void update(unsigned long time);
	bool activeWidget(float x, float y);

};