#include "Menu.h"

#include <sstream>
#include <iostream>
#include <vector>

#include <windows.h>
#include <GL/gl.h>
#include <LDFToolkit/VisComponentGL.h>

#include "ButtonStrategy.h"
#include "InputHandler.h"
#include "Canvas.h"
#include "Font.h"


PushButton::PushButton()
{
	id = 0;
	tex[0] = 0;
	tex[1] = 0;
	tex[2] = 0;
	pressed = false;
	radius = 0.0f;
}

PushButton::PushButton(int d, float rad, uint texUp, uint texPress, uint texDown)
{
	id = d;
	tex[0] = texUp;
	tex[1] = texPress;
	tex[2] = texDown;
	pressed = false;
	radius = rad;
}

PushButton::~PushButton()
{
}

void PushButton::setPosition(float x, float y)
{
	pos = Vector2(x, y);
}

void PushButton::draw(int state)
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex[state]);

	//render properties circle
	glBegin(GL_QUADS);
	glTexCoord2d(0.0,1.0); glVertex2f(pos.x-radius, pos.y-radius); //BL
	glTexCoord2d(0.0,0.0); glVertex2f(pos.x-radius, pos.y+radius); //TL
	glTexCoord2d(1.0,0.0); glVertex2f(pos.x+radius, pos.y+radius); //TR
	glTexCoord2d(1.0,1.0); glVertex2f(pos.x+radius, pos.y-radius); //BR
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

Menu::Menu()
{
}

Menu::Menu(Vector2 pos, float butSize, float propSize, InputHandler* inp)
{
	input = inp;

	menuPos = pos;
	defaultMenuPos = pos;

	menuButtonSize = butSize;
	propertiesCircleRadius = propSize;
	showInProperties = propZOOM;

	for (int i=0; i<NUM_PUSH_BUTTONS; ++i)
		pushButtons[i] = PushButton();

	mainMenuAngles[0] = 0.0f; mainMenuAngles[1] = -45.0f; mainMenuAngles[2] = 105.0f; mainMenuAngles[3] = 60.0f;
	drawMenuAngles[0] = -85.0f; drawMenuAngles[1] = -30.0f;
	editMenuAngles[0] = -85.0f; editMenuAngles[1] = -30.0f;
	penVarAngles[0] = -150.0f; penVarAngles[1] = -35.0f;

	menuPropTex = 0;
	brushSizeTex = 0;
}

Menu::~Menu()
{
}

void Menu::setPushButtons(uint tex[])
{
	for (int i=0; i<NUM_PUSH_BUTTONS; ++i)
		pushButtons[i] = PushButton(i, menuButtonSize*0.5f, tex[i*3], tex[i*3+1], tex[i*3+2]);
}

void Menu::setMenuTextures(uint propTex, uint bsizeTex, uint softTex, uint smoothTex)
{
	menuFont = new Font(std::string("images/unanimo.ttf"), 50);
	//std::cout<<"prop "<<propTex<<"\n";
	menuPropTex = propTex;
	brushSizeTex = bsizeTex;
	softnessTex = softTex;
	smoothnessTex = smoothTex;

	drawMenuTex[0] = brushSizeTex;
	drawMenuTex[1] = smoothnessTex;
	
	editMenuTex[0] = brushSizeTex;
	editMenuTex[1] = softnessTex;

}

void Menu::setVarTextures(uint tex[])
{
	varTex[0] = tex[0]; //none
	varTex[1] = tex[1]; //size
	varTex[2] = tex[2]; //alpha
	//std::cout<<"0,1 "<<tex[0]<<" "<<tex[1]<<"\n";
	for (int i=0; i<NUMDRAWVAR; ++i) //pressure, speed
	{
		//std::cout<<"but "<<tex[3+i*2]<<" "<<tex[3+i*2+1]<<"\n";
		penVar[i] = PushButton(NUM_PUSH_BUTTONS+i, menuButtonSize*0.4f, tex[3+i*2], tex[3+i*2+1], tex[3+i*2]);
		//penVar[i] = PushButton(NUM_PUSH_BUTTONS+i, menuButtonSize*0.4f, 0, 0, 0);
	}
	//std::cout<<"done\n";
}

void Menu::setMenuPos(Vector2 pos)
{
	menuPos = pos;
}

void Menu::setDefaultPos()
{
	menuPos = defaultMenuPos;
}

int Menu::activeWidget(Vector2 pos)
{
	Vector2 propDist = pos - menuPos;
	if (propDist.length() < propertiesCircleRadius)
		return JUSTPRESSED;

	float buttonDist = propertiesCircleRadius+menuButtonSize*0.5f;
	Vector2 buttonLoc = Vector2(-buttonDist, 0.0f);

	for (int i=0; i<NUM_PUSH_BUTTONS; ++i)
	{
		buttonLoc.rotateAroundZero(mainMenuAngles[i]*TO_RADIANS);
		Vector2 bDist = pos - (buttonLoc+menuPos);
		if (bDist.length() < menuButtonSize*0.5f)
		{
			//activate button
			input->buttonPushed(i);
			return JUSTPRESSED;
		}
	}

	//location of submenu drag buttons

	buttonLoc = Vector2(-(propertiesCircleRadius+menuButtonSize*0.52f), 0.0f);	
	
	if (input->active[INPUTMODE] == DRAW)
	{
		for (int i=0; i<NUMDRAWPROPS; ++i)
		{
			buttonLoc.rotateAroundZero(drawMenuAngles[i]*TO_RADIANS);
			Vector2 bDist = pos - (buttonLoc+menuPos);

			if (bDist.length() < menuButtonSize*0.29f)
			{
				showInProperties = i+1;
				return i+1;
			}
		}
		
		buttonLoc = Vector2(-(propertiesCircleRadius+menuButtonSize*0.4f), 0.0f);
		for (int i=0; i<NUMDRAWVAR; ++i)
		{
			buttonLoc.rotateAroundZero(penVarAngles[i]*TO_RADIANS);
			Vector2 bDist = pos - (buttonLoc+menuPos);

			if (bDist.length() < menuButtonSize*0.4f)
			{
				showInProperties = NUMEDITPROPS+NUMDRAWPROPS+i+1;
				return NUMEDITPROPS+NUMDRAWPROPS+i+1;
			}
		}
	}
	else if (input->active[INPUTMODE] == EDIT)
	{
		for (int i=0; i<NUMEDITPROPS; ++i)
		{
			buttonLoc.rotateAroundZero(editMenuAngles[i]*TO_RADIANS);
			Vector2 bDist = pos - (buttonLoc+menuPos);

			if (bDist.length() < menuButtonSize*0.29f)
			{
				showInProperties = i+1+NUMDRAWPROPS;
				return i+1+NUMDRAWPROPS;
			}
		}
	}

	return -1;
}

void Menu::draw()
{
	float xMenu = menuPos.x;
	float yMenu = menuPos.y;
	float propSize = propertiesCircleRadius;
	float buttonDist = propSize+menuButtonSize*0.5f;
	float dragButWidth = menuButtonSize*0.588f;

	Vector2 dragButPos[4];
	dragButPos[0] = Vector2(-propSize-menuButtonSize, -dragButWidth*0.5f);
	dragButPos[1] = Vector2(-propSize-menuButtonSize, dragButWidth*0.5f);
	dragButPos[2] = Vector2(-propSize, -dragButWidth*0.5f);
	dragButPos[3] = Vector2(-propSize, dragButWidth*0.5f);

	Vector2 buttonLoc = Vector2(-buttonDist, 0.0f);

	//place main menu buttons
	for (int i=0; i<NUM_PUSH_BUTTONS; ++i)
	{
		buttonLoc.rotateAroundZero(mainMenuAngles[i]*TO_RADIANS);
		pushButtons[i].setPosition(xMenu + buttonLoc.x, yMenu + buttonLoc.y);
		if (input->active[MENUPRESSED] && input->active[GUIBUTTON] == i)
			pushButtons[i].draw(1);
		else if (input->active[MODEBUTTON] == i)
			pushButtons[i].draw(2);
		else
			pushButtons[i].draw(0);
	}

	//std::vector<unsigned long> selectedIds;
	//pushButtons[0]->renderBranch(selectedIds, false);
	
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, menuPropTex);

	//render properties circle
	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2d(0.0,0.0); glVertex2f(xMenu-propSize, yMenu-propSize);
	glTexCoord2d(0.0,1.0); glVertex2f(xMenu-propSize, yMenu+propSize);
	glTexCoord2d(1.0,0.0); glVertex2f(xMenu+propSize, yMenu-propSize);
	glTexCoord2d(1.0,1.0); glVertex2f(xMenu+propSize, yMenu+propSize);
	glEnd();

	if (input->active[INPUTMODE] == DRAW)
	{
		for (int i=0; i<NUMDRAWPROPS; ++i)
		{
			for (int j=0; j<4; ++j)
				dragButPos[j].rotateAroundZero(drawMenuAngles[i]*TO_RADIANS);

			glBindTexture(GL_TEXTURE_2D, drawMenuTex[i]);
			//render brush size drag button
			glBegin(GL_TRIANGLE_STRIP);
			glTexCoord2d(0.0,0.0); glVertex2f(xMenu+dragButPos[0].x, yMenu+dragButPos[0].y);
			glTexCoord2d(0.0,1.0); glVertex2f(xMenu+dragButPos[1].x, yMenu+dragButPos[1].y);
			glTexCoord2d(1.0,0.0); glVertex2f(xMenu+dragButPos[2].x, yMenu+dragButPos[2].y);
			glTexCoord2d(1.0,1.0); glVertex2f(xMenu+dragButPos[3].x, yMenu+dragButPos[3].y);
			glEnd();
		}

		//draw pen variable buttons
		buttonLoc = Vector2(-buttonDist, 0.0f);
		for (int i=0; i<NUMDRAWVAR; ++i)
		{
			buttonLoc.rotateAroundZero(penVarAngles[i]*TO_RADIANS);
			penVar[i].setPosition(xMenu + buttonLoc.x, yMenu + buttonLoc.y);
			if (input->active[MENUPRESSED] && input->active[GUIBUTTON] == NUM_PUSH_BUTTONS+i)
				penVar[i].draw(1);
			else
				penVar[i].draw(0);
		}
	}
	else if (input->active[INPUTMODE] == EDIT)
	{
		for (int i=0; i<NUMEDITPROPS; ++i)
		{
			for (int j=0; j<4; ++j)
				dragButPos[j].rotateAroundZero(editMenuAngles[i]*TO_RADIANS);

			glBindTexture(GL_TEXTURE_2D, editMenuTex[i]);
			//render brush size drag button
			glBegin(GL_TRIANGLE_STRIP);
			glTexCoord2d(0.0,0.0); glVertex2f(xMenu+dragButPos[0].x, yMenu+dragButPos[0].y);
			glTexCoord2d(0.0,1.0); glVertex2f(xMenu+dragButPos[1].x, yMenu+dragButPos[1].y);
			glTexCoord2d(1.0,0.0); glVertex2f(xMenu+dragButPos[2].x, yMenu+dragButPos[2].y);
			glTexCoord2d(1.0,1.0); glVertex2f(xMenu+dragButPos[3].x, yMenu+dragButPos[3].y);
			glEnd();
		}
	}

	glDisable(GL_TEXTURE_2D);

	std::ostringstream conv;
	conv.precision(2);

	std::string text;

	if (showInProperties == propBRUSHSIZE)
	{
		renderRegularBrush();
	}
	else if (showInProperties == propMOVEBRUSHSIZE || showInProperties == propMOVESOFT)
	{
		renderMoveBrush();
	}
	else if (showInProperties == propBRUSHSMOOTH)
	{
		renderSmoothness();
	}
	else if (showInProperties == propZOOM)
	{
		glColor3f(0, 0, 0);
		conv.setf(std::ios::fixed,std::ios::floatfield);
		conv <<  input->canvas->zoom; text = conv.str();
		text.append("x");
		menuFont->renderText(text,xMenu-25.0f, yMenu-30.0f);
	}
	else if (showInProperties == propPRESSUREVAR)
	{
		renderVars(0);
	}
	else if (showInProperties == propSPEEDVAR)
	{
		renderVars(1);
	}
}

void Menu::renderRegularBrush()
{
	input->canvas->brush.drawMenu(menuPos);
}

void Menu::renderMoveBrush()
{
	input->canvas->moveBrush.drawMenu(menuPos);
}

void Menu::renderSmoothness()
{
	Vector2 line[7];
	float hW = propertiesCircleRadius*0.6f;
	float dx = (2.0f*hW)/6.0f;
	float dy = 10-input->canvas->maxStrokeError;
	float step = dx;
	line[0] = Vector2(-hW, 0.0f);

	line[1] = Vector2(-hW+step, dy); step += dx;
	line[2] = Vector2(-hW+step, -dy); step += dx;
	line[3] = Vector2(-hW+step, dy); step += dx;
	line[4] = Vector2(-hW+step, -dy); step += dx;
	line[5] = Vector2(-hW+step, dy);

	line[6] = Vector2(hW, 0.0f);

	glLineWidth(3.0f);
	glColor3f(0.0f, 0.0f, 0.0f);
	glBegin(GL_LINE_STRIP);
	for (int i=0; i<7; ++i)
		glVertex2f(menuPos.x+line[i].x, menuPos.y+line[i].y);
	glEnd();
	glLineWidth(1.0f);
}

void Menu::renderVars(int var)
{
	float radius = propertiesCircleRadius*0.4f;
	
	int tex;
	if (var == 0) //pressure
		tex = input->canvas->brush.varyWithPressure;
	else
		tex = input->canvas->brush.varyWithSpeed;


	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, varTex[tex]);

	//render properties circle
	glBegin(GL_QUADS);
	glTexCoord2d(0.0,1.0); glVertex2f(menuPos.x-radius, menuPos.y-radius); //BL
	glTexCoord2d(0.0,0.0); glVertex2f(menuPos.x-radius, menuPos.y+radius); //TL
	glTexCoord2d(1.0,0.0); glVertex2f(menuPos.x+radius, menuPos.y+radius); //TR
	glTexCoord2d(1.0,1.0); glVertex2f(menuPos.x+radius, menuPos.y-radius); //BR
	glEnd();
	glDisable(GL_TEXTURE_2D);
}
