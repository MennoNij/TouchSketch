#ifndef MYAPPLICATIONMANAGER_H
#define MYAPPLICATIONMANAGER_H

#pragma once

#include <LDFToolkit/DefaultManager.h>

#include <LDFToolkit/Strategies/Drawers/GradientBorderStrategy.h>
#include <LDFToolkit/Strategies/Drawers/GradientRoundBorderStrategy.h>

#include <string>
#include <sstream>
#include <iostream>

#include "Font.h"

class InputHandler;
class Canvas;
class VisComponentGL;
//class Font;

using std::string;

class MyApplicationManager : public DefaultManager
{
public:
	Canvas* canvas;
	InputHandler* input;

	VisComponentGL* tapWidget;
	GradientRoundBorderStrategy* tapEdge0;
	GradientRoundBorderStrategy* tapEdge1;

	unsigned int pressType;
	unsigned int dragType;
	unsigned int releaseType;

	float curX;
	float curY;

	uint borderCircleTex;
	uint smallToBigTex;

	Font* debugFont;
	Font* screenFont;

	MyApplicationManager(FrameTimer* timer = NULL);
	~MyApplicationManager(void);

	void processEvent(LargeDisplayEvent* evt, int button = -1,  float press = 1.0f, VisComponent* target = NULL);
	void processKeyPress(string key, int modifier);
	void processTimedEvents();
	void renderAll(bool enablePicking = false);
	
	void resize(unsigned int width, unsigned int height);

	void renderScreenBorder();
	void renderMenu();
	void renderTapWidget();

	void writeCommandLog();

protected:
	void initComponents();

	template<class T> std::string toString(T value) {
	   std::ostringstream oss;
	   oss << value;
	   return oss.str();
	}

};

#endif  // MYAPPLICATIONMANAGER_H
