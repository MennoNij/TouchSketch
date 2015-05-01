#pragma once

#include <string>

#include "Vector2.h"
#include "Font.h"

class VisComponentGL;
class InputHandler;

#define uint unsigned int
#define NUM_PUSH_BUTTONS 4
#define NUMDRAWPROPS 2
#define NUMEDITPROPS 2
#define NUMDRAWVAR 2

enum inProperties { propZOOM, propBRUSHSIZE, propBRUSHSMOOTH,
										 propMOVEBRUSHSIZE, propMOVESOFT,
										 propPRESSUREVAR, propSPEEDVAR };

struct PushButton
{
	int id;
	Vector2 pos;
	uint tex[3];
	bool pressed;
	float radius;

	PushButton();
	PushButton(int d, float rad, uint texU, uint texP, uint texD);
	~PushButton();

	void setPosition(float x, float y);
	void draw(int state);
};

class Menu 
{
public:
	InputHandler* input;

	Vector2 menuPos; //absolute position of the menu
	Vector2 defaultMenuPos; //where the menu always returns to
	float menuButtonSize; //size of the buttons
	float propertiesCircleRadius; //size of the property circle
	int showInProperties; //what type of information to show in the properties circle
	float mainMenuAngles[NUM_PUSH_BUTTONS]; //positions of the main menu buttons expressed in angles
	float drawMenuAngles[NUMDRAWPROPS]; //positions for draw submenu
	uint drawMenuTex[NUMDRAWPROPS];
	float editMenuAngles[NUMEDITPROPS]; //positions for edit submenu
	uint editMenuTex[NUMEDITPROPS];
	float penVarAngles[NUMDRAWVAR];
	//uint penVarTex[NUMDRAWVAR];

	uint varTex[3];

	//graphics
	PushButton pushButtons[NUM_PUSH_BUTTONS];
	PushButton penVar[NUMDRAWVAR];

	uint menuPropTex;
	uint brushSizeTex;
	uint softnessTex;
	uint smoothnessTex;

	Font* menuFont;

	Menu();
	Menu(Vector2 pos, float butSize, float propSize, InputHandler* inp);
	~Menu();

	void setPushButtons(uint tex[]);
	void setMenuTextures(uint propTex, uint bsizeTex, uint softTex, uint smoothTex);
	void setVarTextures(uint tex[]);
	void setMenuPos(Vector2 pos);
	void setDefaultPos();

	int activeWidget(Vector2 pos);
	void draw();

	void renderRegularBrush();
	void renderMoveBrush();
	void renderSmoothness();
	void renderVars(int var);
};