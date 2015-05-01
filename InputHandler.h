#pragma once

#include <vector>
#include <string>

#include "tinyxml.h"

#include "Vector2.h"
#include "Menu.h"
#include "TapWidget.h"

class Canvas;
class DrawBrush;

enum commandList { useBorder, useBrush, useMoveBrush, useMoveStroke, useSelection, useSelScale, useSelMove, useSelRot,
										useTap, useTapDel, useTapSelect, useTapMenu, useBorRot, useBorMove, useBorScale };

/*
Handle User and GUI input
*/
#define ACTIVESIZE 8
enum activeIndex { DEVICEBUTTON, MODEBUTTON, GUIBUTTON, SELECTIONWIDGET, MENUPRESSED, INPUTMODE, MENUACTION, CANVASWIDGET };
enum menuAction { JUSTPRESSED, BRUSHSIZE, BRUSHSMOOTH, MOVEBRUSHSIZE, MOVESOFT, CHANGEPRESSURE, CHANGESPEED };
#define INTERACTIONSIZE 6
enum interactionIndex { iBORDER, iSELECTION, TYPEBORDER, TYPESELECTION, TYPEMENU, iTAPWIDGET  };

enum inputButtons { DRAW, EDIT, CLEAR, RESETCANVAS };
enum inputTypes { PRIMARY, SECONDARY };
enum widgetTypes { CANVAS, BORDER, MENU, SELECTWIDGET, TAPWIDGET };
enum borderInteractType { BOR_MOVE, BOR_ROTATE, BOR_ZOOM, BOR_ZOOMIN, BOR_ZOOMOUT, BOR_BL, BOR_BR, BOR_TR, BOR_TL, };
enum selectInteractType { SEL_MOVE, SEL_ROTATE, SEL_SCALE };

using std::vector;
using std::string;

class InputHandler
{
public:
	Canvas* canvas;
	int active[ACTIVESIZE]; //values for various widgets & modes, defined in enum activeIndex
	int interaction[INTERACTIONSIZE];

	bool alreadyPressed; //due to sensitivity, some input devices send several 'press' events at once
	bool alreadyReleased; //the same goes for release events

	//float prevX1[5]; float prevY1[5]; //store last 5 positions
	//float prevX2[5]; float prevY2[5];

	//bool borderInteraction; //is the border currently being interacted with
	float borderWidth; //width of the screen border where the canvas is inactive (used by rotation and moving)
	float cornerCircleRadius;

	float prevX1, prevY1; //previous x,y coords of pointer 1
	float prevX2, prevY2; //(optional) previous x,y coords of pointer 2
	float pressX1, pressY1; //used to distinguish a 'tap' from a real 'press'

	unsigned long pressTime;
	int tapTime; //tap sensitivity in miliseconds
	
	float screenWidth, screenHeight; //dimensions of the screen/canvas

	Menu guiMenu;

	TapWidget tapWidget;

	int currentSketch;

	unsigned long startTime;
	
	vector<int> commandList;
	vector<float> commandTime;
	vector<float> angleList;

	unsigned long currentTime;

	Vector2 cursorPos;

	Vector2 strokeStartPos;
	float strokeStartTime;

	InputHandler(Canvas* canv);
	~InputHandler();
	
	//## User input routines
	
	//button press event from input device
	void onPress(float x, float y, int button, float pressure = 1.0f);
	//dragging (moving with a pressed button) event from the input device
	void onDragging(float x, float y, float pressure = 1.0f);
	//button release event from user input
	void onRelease(float x, float y, int button);

	//## GUI routines
	void buttonPushed(int id);
	void keyPressed(string& key, int modifier);
	//screen border interactions (rotate, move)
	void doMenuInteraction(float x, float y, float dx, float dy);
	void doBorderInteraction(float x, float y, float dx, float dy);
	void doSelectionInteraction(float x, float y, float dx, float dy);
	//What is the designation (type of widget) of the screen location clicked
	int determineWidgetType(float x, float y);
	void updateTapWidget();
	
	//## Other stuff
	void setInputMode(int mode) { active[INPUTMODE] = mode; }
	void setScreenSize(unsigned int width, unsigned int height) { screenWidth = (float)width; screenHeight = (float)height; }
	void setCursorPos(float x, float y);

	void logCommand(int command);
	void logAngles(float x, float y);
	void writeCommandLog();
};
