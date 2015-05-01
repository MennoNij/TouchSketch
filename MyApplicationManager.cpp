#include "MyApplicationManager.h"

#include <sstream>
#include <iostream>

#define NOMINMAX 
#include <windows.h>
#include <GL/gl.h>
#include <stdlib.h>
#include <time.h>

#define _USE_MATH_DEFINES
#include <cmath>
//#define TO_RADIANS (M_PI / 180.0)
//#define TO_DEGREES (180.0 / M_PI)

#include <LDF/FrameTimer.h>
#include <LDFToolkit/VisComponentGL.h>
#include <LDF/LargeDisplayEvent.h>
#include <LDFToolkit/ConstantProvider.h>
#include <LDFToolkit/Utils/TextureManager.h>
#include <LDFToolkit/Strategies/Drawers/ImageStrategy.h>
#include <LDFToolkit/Strategies/Drawers/ResizerHandleStrategy.h>

#include "ButtonStrategy.h"
#include "Canvas.h"
#include "InputHandler.h"
#include "DrawBrush.h"
#include "Menu.h"

MyApplicationManager::MyApplicationManager(FrameTimer* timer) : DefaultManager(timer)
{
	srand(time(NULL));

	canvas = new Canvas();
	input = new InputHandler(canvas);
	
	ConstantProvider* constants = ConstantProvider::getInstance();
	pressType = constants->getEventTypeIdentifier("PRESS");
	dragType = constants->getEventTypeIdentifier("DRAG");
	releaseType = constants->getEventTypeIdentifier("RELEASE");

	borderCircleTex = 0;

	curX = curY = 0;

	initialize();
}

MyApplicationManager::~MyApplicationManager(void)
{
	input->writeCommandLog();
	delete input;
	delete canvas;
}


void MyApplicationManager::initComponents()
{
	input->startTime = frameTimer->getCurrentTime();

	//Initialize the different components
	rootComponent = new VisComponentGL();
	rootComponent->setPosition(Point3f(400, 300, 0));
	rootComponent->setWidth(800);
	rootComponent->setHeight(600);
	//Force initialization, only needs to be done with rootComponent
	//rootComponent->getStrategy()->initPropertiesComponent();

	//Lines component

	//GUI Component(s)
	std::vector<std::string> textureFiles;
	textureFiles.push_back(std::string("images/drawButton_up.png"));
	textureFiles.push_back(std::string("images/drawButton_press.png"));
	textureFiles.push_back(std::string("images/drawButton_down.png"));
	textureFiles.push_back(std::string("images/editButton_up.png"));
	textureFiles.push_back(std::string("images/editButton_press.png"));
	textureFiles.push_back(std::string("images/editButton_down.png"));
	textureFiles.push_back(std::string("images/plus.png"));
	textureFiles.push_back(std::string("images/minus.png"));
	textureFiles.push_back(std::string("images/selectWidget.png")); //texID 8
	textureFiles.push_back(std::string("images/selectWidget_high.png"));
	textureFiles.push_back(std::string("images/anchor.png"));
	textureFiles.push_back(std::string("images/corner_circle.png"));
	textureFiles.push_back(std::string("images/properties.png"));
	textureFiles.push_back(std::string("images/brush_sizer.png"));
	textureFiles.push_back(std::string("images/deleteButton_up.png"));
	textureFiles.push_back(std::string("images/deleteButton_press.png"));
	textureFiles.push_back(std::string("images/small_big.png")); //texID 16
	textureFiles.push_back(std::string("images/softness_sizer.png"));
	textureFiles.push_back(std::string("images/smoothness_sizer.png"));
	textureFiles.push_back(std::string("images/vary_size.png"));
	textureFiles.push_back(std::string("images/vary_alpha.png"));
	textureFiles.push_back(std::string("images/pen_pressure_up.png"));
	textureFiles.push_back(std::string("images/pen_pressure_press.png"));
	textureFiles.push_back(std::string("images/pen_speed_up.png"));
	textureFiles.push_back(std::string("images/pen_speed_press.png")); //texID 24
	textureFiles.push_back(std::string("images/vary_none.png"));
	textureFiles.push_back(std::string("images/cancel_up.png"));
	textureFiles.push_back(std::string("images/define_brush.png"));
	textureFiles.push_back(std::string("images/reset_canvas_up.png")); //texID 28
	textureFiles.push_back(std::string("images/reset_canvas_press.png"));
	textureFiles.push_back(std::string("images/0003.png"));

	TextureManager texManager = TextureManager();
	texManager.setTexCompression(false);
	//texManager.setMipMapping(false);

	texManager.loadTextures(textureFiles);

	debugFont = new Font(std::string("images/arial.ttf"), 12);
	screenFont = new Font(std::string("images/unanimo.ttf"), 30);

	//set textures for selection widget
	canvas->selection.widgetTex = texManager.getTextureName(8); //selection widget texture
	canvas->selection.widgetPressTex = texManager.getTextureName(9);
	canvas->selection.widgetAnchorTex = texManager.getTextureName(10);

	borderCircleTex = texManager.getTextureName(11);
	smallToBigTex = texManager.getTextureName(16);

	input->guiMenu.setMenuTextures(texManager.getTextureName(12), texManager.getTextureName(13), texManager.getTextureName(17),
																 texManager.getTextureName(18));

	//button textures
	unsigned int tex[NUM_PUSH_BUTTONS*3];
	tex[0] = texManager.getTextureName(0); tex[1] = texManager.getTextureName(1); tex[2] = texManager.getTextureName(2); //draw
	tex[3] = texManager.getTextureName(3); tex[4] = texManager.getTextureName(4); tex[5] = texManager.getTextureName(5); //edit
	tex[6] = texManager.getTextureName(14); tex[7] = texManager.getTextureName(15); tex[8] = texManager.getTextureName(15); //clear
	tex[9] = texManager.getTextureName(28); tex[10] = texManager.getTextureName(29); tex[11] = texManager.getTextureName(29); //canvas reset

	//setup the buttons
	input->guiMenu.setPushButtons(tex);

	unsigned int tex2[2+NUMDRAWVAR*2];
	tex2[0] = texManager.getTextureName(25); tex2[1] = texManager.getTextureName(19); tex2[2] = texManager.getTextureName(20);
	
	tex2[3] = texManager.getTextureName(21); tex2[4] = texManager.getTextureName(22);
	tex2[5] = texManager.getTextureName(23); tex2[6] = texManager.getTextureName(24);

	input->guiMenu.setVarTextures(tex2);

	unsigned int tex3[3];
	tex3[0] = texManager.getTextureName(26);
	tex3[1] = texManager.getTextureName(14);
	tex3[2] = texManager.getTextureName(27);

	input->canvas->selection.setButtonTex(tex3);
	//add buttons to framework
/*	for (int i=0; i<NUM_PUSH_BUTTONS; ++i)
		addComponent(input->guiMenu.pushButtons[i]);
*/

	//setup the tap widget
	float borderColor[4] = {  0.7f, 0.1f, 0.0f, 0.9f };

	tapWidget = new VisComponentGL();
	tapWidget->setWidth(input->tapWidget.diameter);
	tapWidget->setHeight(input->tapWidget.diameter);

	tapEdge0 = new GradientRoundBorderStrategy();
	tapEdge0->setBorderWidth(10.0);
	tapEdge0->setColor(borderColor);	
	tapWidget->pushStrategy(tapEdge0);

	tapEdge1 = new GradientRoundBorderStrategy();
	tapEdge1->setBorderWidth(-10.0);
	tapEdge1->setColor(borderColor);	
	tapWidget->pushStrategy(tapEdge1);

	//addComponent(c);
}


void MyApplicationManager::renderScreenBorder()
{
	float circleRadius = input->cornerCircleRadius;
	
	//calculate circle stuff for one corner
	float tilt = -45.0f*TO_RADIANS;
	float xx = 0.0f; float yy = circleRadius;
	float cx = xx*cos(tilt) - yy*sin(tilt);
	float cy = xx*sin(tilt) + yy*cos(tilt);

	circleRadius *= 1.03f; //fix coverage of the border
	float screenHeight = input->screenHeight;
	float screenWidth = input->screenWidth;
	float borderWidth = input->borderWidth;

	Vector2 circleExtent = Vector2(cx, cy);
	Vector2 circlePerp = Vector2(-cy, cx).normalized();
	Vector2 p1 = circleExtent + circlePerp*1000.0f;
	Vector2 p2 = circleExtent - circlePerp*1000.0f;

	//find intersection with bottom edge
	Vector2 b1 = Vector2(0.0f,borderWidth);
	Vector2 b2 = Vector2(1000.0f, borderWidth);

	float b = ((b2.x-b1.x)*(p1.y-b1.y) - (b2.y-b1.y)*(p1.x-b1.x)) / ( (b2.y-b1.y)*(p2.x-p1.x) - (b2.x-b1.x)*(p2.y-p1.y) );
	Vector2 BL1 = p1 + (p2-p1)*b;

	//find intersection with left edge
	Vector2 l1 = Vector2(borderWidth, 0.0f);
	Vector2 l2 = Vector2(borderWidth, 1000.0f);

	float a = ((l2.x-l1.x)*(p1.y-l1.y) - (l2.y-l1.y)*(p1.x-l1.x)) / ( (l2.y-l1.y)*(p2.x-p1.x) - (l2.x-l1.x)*(p2.y-p1.y) );
	Vector2 BL2 = p1 + (p2-p1)*a;

	//calculate for the remaining corners

	//top left
	Vector2 TL1 = Vector2(BL2.x, screenHeight-BL2.y);
	Vector2 TL2 = Vector2(BL1.x, screenHeight-BL1.y);

	//top right
	Vector2 TR1 = Vector2(screenWidth-TL2.x, TL2.y);
	Vector2 TR2 = Vector2(screenWidth-TL1.x, TL1.y);

	//bottom right
	Vector2 BR1 = Vector2(screenWidth-BL2.x, BL2.y);
	Vector2 BR2 = Vector2(screenWidth-BL1.x, BL1.y);

	//draw 'rectangular' border
	glColor4f(0.258f, 0.486f, 0.647f, 0.6f);
	glBegin(GL_TRIANGLE_STRIP);
	glVertex2f(0.0f, 0.0f);
	glVertex2f(BL2.x, BL2.y);
	glVertex2f(0.0f, screenHeight);
	glVertex2f(TL1.x, TL1.y);
	glVertex2f(TL2.x, TL2.y);
	
	glVertex2f(TL2.x, TL2.y);
	glVertex2f(0.0f, screenHeight);
	glVertex2f(TL2.x, TL2.y);
	glVertex2f(screenWidth, screenHeight);
	glVertex2f(TR1.x, TR1.y);
	glVertex2f(TR2.x, TR2.y);

	glVertex2f(TR2.x, TR2.y);
	glVertex2f(screenWidth, screenHeight);
	glVertex2f(TR2.x, TR2.y);
	glVertex2f(screenWidth, 0.0f);
	glVertex2f(BR1.x, BR1.y);
	glVertex2f(BR2.x, BR2.y);


	glVertex2f(BR2.x, BR2.y);
	glVertex2f(screenWidth, 0.0f);
	glVertex2f(BR2.x, BR2.y);
	glVertex2f(0.0f, 0.0f);
	glVertex2f(BL1.x, BL1.y);
	glVertex2f(BL2.x, BL2.y);
	glEnd();

		
	float dsW = 8.0f;

	//render drop shadow of border
	Vector2 nBL = Vector2(1,1);
	Vector2 nTL = Vector2(1,-1);
	Vector2 nTR = Vector2(-1,-1);
	Vector2 nBR = Vector2(-1,1);

	float sh1[4] = {0.0f,0.0f,0.0f,0.45f};
	float sh2[4] = {0.0f,0.0f,0.0f,0.05f};

	glColor4f(0.6, 0.6, 0.6, 0.9f);
	glBegin(GL_TRIANGLE_STRIP);
	
	glColor4fv(sh1); glVertex2f(BL1.x, BL1.y);
	glColor4fv(sh2); glVertex2f(BL1.x+dsW*nBL.x, BL1.y+dsW*nBL.y);	
	
	glColor4fv(sh1); glVertex2f(BL2.x, BL2.y);
	glColor4fv(sh2); glVertex2f(BL2.x+dsW*nBL.x, BL2.y+dsW*nBL.y);	

	glColor4fv(sh1); glVertex2f(TL1.x, TL1.y);
	glColor4fv(sh2); glVertex2f(TL1.x+dsW*nTL.x, TL1.y+dsW*nTL.y);	

	glColor4fv(sh1); glVertex2f(TL2.x, TL2.y);
	glColor4fv(sh2); glVertex2f(TL2.x+dsW*nTL.x, TL2.y+dsW*nTL.y);	

	glColor4fv(sh1); glVertex2f(TR1.x, TR1.y);
	glColor4fv(sh2); glVertex2f(TR1.x+dsW*nTR.x, TR1.y+dsW*nTR.y);	

	glColor4fv(sh1); glVertex2f(TR2.x, TR2.y);
	glColor4fv(sh2); glVertex2f(TR2.x+dsW*nTR.x, TR2.y+dsW*nTR.y);	

	glColor4fv(sh1); glVertex2f(BR1.x, BR1.y);
	glColor4fv(sh2); glVertex2f(BR1.x+dsW*nBR.x, BR1.y+dsW*nBR.y);	

	glColor4fv(sh1); glVertex2f(BR2.x, BR2.y);
	glColor4fv(sh2); glVertex2f(BR2.x+dsW*nBR.x, BR2.y+dsW*nBR.y);	

	glColor4fv(sh1); glVertex2f(BL1.x, BL1.y);
	glColor4fv(sh2); glVertex2f(BL1.x+dsW*nBL.x, BL1.y+dsW*nBL.y);	

	glEnd();


	//render line around border
	glColor4f(0.258f, 0.486f, 0.647f, 1.0f);
	glLineWidth(2.0f);
	glBegin(GL_LINES);
	glVertex2f(TL2.x, TL2.y);
	glVertex2f(TR1.x, TR1.y);
	glVertex2f(TR2.x, TR2.y);
	glVertex2f(BR1.x, BR1.y);
	glVertex2f(BR2.x, BR2.y);
	glVertex2f(BL1.x, BL1.y);
	glVertex2f(BL2.x, BL2.y);
	glVertex2f(TL1.x, TL1.y);
	glEnd();
	glLineWidth(1.0f);

	float innerRadius = circleRadius*0.3f;

	//render corner circles
	glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, borderCircleTex);
	//BL
	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2d(0.0,0.0); glVertex3f(-circleRadius, -circleRadius, 1.0f);
	glTexCoord2d(0.0,1.0); glVertex3f(-circleRadius, circleRadius, 1.0f);
	glTexCoord2d(1.0,0.0); glVertex3f(circleRadius, -circleRadius, 1.0f);
	glTexCoord2d(1.0,1.0); glVertex3f(circleRadius, circleRadius, 1.0f);
	glEnd();

	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2d(0.0,0.0); glVertex3f(-innerRadius, -innerRadius, 1.0f);
	glTexCoord2d(0.0,1.0); glVertex3f(-innerRadius, innerRadius, 1.0f);
	glTexCoord2d(1.0,0.0); glVertex3f(innerRadius, -innerRadius, 1.0f);
	glTexCoord2d(1.0,1.0); glVertex3f(innerRadius, innerRadius, 1.0f);
	glEnd();

	//BR
	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2d(0.0,0.0); glVertex3f(screenWidth-circleRadius, -circleRadius, 1.0f);
	glTexCoord2d(0.0,1.0); glVertex3f(screenWidth-circleRadius, circleRadius, 1.0f);
	glTexCoord2d(1.0,0.0); glVertex3f(screenWidth+circleRadius, -circleRadius, 1.0f);
	glTexCoord2d(1.0,1.0); glVertex3f(screenWidth+circleRadius, circleRadius, 1.0f);
	glEnd();

	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2d(0.0,0.0); glVertex3f(screenWidth-innerRadius, -innerRadius, 1.0f);
	glTexCoord2d(0.0,1.0); glVertex3f(screenWidth-innerRadius, innerRadius, 1.0f);
	glTexCoord2d(1.0,0.0); glVertex3f(screenWidth+innerRadius, -innerRadius, 1.0f);
	glTexCoord2d(1.0,1.0); glVertex3f(screenWidth+innerRadius, innerRadius, 1.0f);
	glEnd();

	//TL
	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2d(0.0,0.0); glVertex3f(-circleRadius, screenHeight-circleRadius, 1.0f);
	glTexCoord2d(0.0,1.0); glVertex3f(-circleRadius, screenHeight+circleRadius, 1.0f);
	glTexCoord2d(1.0,0.0); glVertex3f(circleRadius, screenHeight-circleRadius, 1.0f);
	glTexCoord2d(1.0,1.0); glVertex3f(circleRadius, screenHeight+circleRadius, 1.0f);
	glEnd();

	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2d(0.0,0.0); glVertex3f(-innerRadius, screenHeight-innerRadius, 1.0f);
	glTexCoord2d(0.0,1.0); glVertex3f(-innerRadius, screenHeight+innerRadius, 1.0f);
	glTexCoord2d(1.0,0.0); glVertex3f(innerRadius, screenHeight-innerRadius, 1.0f);
	glTexCoord2d(1.0,1.0); glVertex3f(innerRadius, screenHeight+innerRadius, 1.0f);
	glEnd();

	//TR
	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2d(0.0,0.0); glVertex3f(screenWidth-circleRadius, screenHeight-circleRadius, 1.0f);
	glTexCoord2d(0.0,1.0); glVertex3f(screenWidth-circleRadius, screenHeight+circleRadius, 1.0f);
	glTexCoord2d(1.0,0.0); glVertex3f(screenWidth+circleRadius, screenHeight-circleRadius, 1.0f);
	glTexCoord2d(1.0,1.0); glVertex3f(screenWidth+circleRadius, screenHeight+circleRadius, 1.0f);
	glEnd();

	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2d(0.0,0.0); glVertex3f(screenWidth-innerRadius, screenHeight-innerRadius, 1.0f);
	glTexCoord2d(0.0,1.0); glVertex3f(screenWidth-innerRadius, screenHeight+innerRadius, 1.0f);
	glTexCoord2d(1.0,0.0); glVertex3f(screenWidth+innerRadius, screenHeight-innerRadius, 1.0f);
	glTexCoord2d(1.0,1.0); glVertex3f(screenWidth+innerRadius, screenHeight+innerRadius, 1.0f);
	glEnd();

	//render scaling arrows
  glBindTexture(GL_TEXTURE_2D, smallToBigTex);
	float scaleArrSize = 20.0f;
	Vector2 scaleBL = Vector2(-scaleArrSize, -scaleArrSize);
	Vector2 scaleTL = Vector2(-scaleArrSize, scaleArrSize);
	Vector2 scaleTR = Vector2(scaleArrSize, scaleArrSize);
	Vector2 scaleBR = Vector2(scaleArrSize, -scaleArrSize);

	float arrAng = 60.0f*TO_RADIANS;
	scaleBL.rotateAroundZero(arrAng);
	scaleTL.rotateAroundZero(arrAng);
	scaleTR.rotateAroundZero(arrAng);
	scaleBR.rotateAroundZero(arrAng);

	Vector2 pos = Vector2(screenWidth-70,30);

	glBegin(GL_QUADS);
	glTexCoord2d(0.0,0.0); glVertex2f(pos.x+scaleBL.x, pos.y+scaleBL.y);
	glTexCoord2d(0.0,1.0); glVertex2f(pos.x+scaleTL.x, pos.y+scaleTL.y);
	glTexCoord2d(1.0,1.0); glVertex2f(pos.x+scaleTR.x, pos.y+scaleTR.y);
	glTexCoord2d(1.0,0.0); glVertex2f(pos.x+scaleBR.x, pos.y+scaleBR.y);
	glEnd();

	arrAng = 145.0f*TO_RADIANS;
	scaleBL.rotateAroundZero(arrAng);
	scaleTL.rotateAroundZero(arrAng);
	scaleTR.rotateAroundZero(arrAng);
	scaleBR.rotateAroundZero(arrAng);

	pos = Vector2(screenWidth-30,70);

	glBegin(GL_QUADS);
	glTexCoord2d(0.0,0.0); glVertex2f(pos.x+scaleBL.x, pos.y+scaleBL.y);
	glTexCoord2d(0.0,1.0); glVertex2f(pos.x+scaleTL.x, pos.y+scaleTL.y);
	glTexCoord2d(1.0,1.0); glVertex2f(pos.x+scaleTR.x, pos.y+scaleTR.y);
	glTexCoord2d(1.0,0.0); glVertex2f(pos.x+scaleBR.x, pos.y+scaleBR.y);
	glEnd();


	glDisable(GL_TEXTURE_2D);

	glLineWidth(3);
	glBegin(GL_LINES);
	glVertex2f(0,0);
	glVertex2f(cx, cy);

	glVertex2f(screenWidth,0);
	glVertex2f(screenWidth-cx, cy);

	glVertex2f(0,screenHeight);
	glVertex2f(cx, screenHeight-cy);

	glVertex2f(screenWidth,screenHeight);
	glVertex2f(screenWidth-cx, screenHeight-cy);
	glEnd();
	glLineWidth(1);

}

void MyApplicationManager::renderMenu()
{
	input->guiMenu.draw();
	//drawButton->renderBranch(selectedIds, enablePicking);
	//editButton->renderBranch(selectedIds, enablePicking);

}

void MyApplicationManager::renderTapWidget()
{
	std::vector<unsigned long> selectedIds;
	
	if (input->tapWidget.active)
	{
		float c[4];
		c[0] = 0.7f; c[1] = 0.1f; c[2] = 0.0f; c[3] = input->tapWidget.visibility;

		tapEdge0->setColor(c);
		tapEdge1->setColor(c);
		
		tapWidget->setPosition(input->tapWidget.pos.x, input->tapWidget.pos.y);
		tapWidget->renderBranch(selectedIds, false);
	}
}

void MyApplicationManager::processEvent(LargeDisplayEvent* evt, int button, float press, VisComponent* target)
{
	curX = evt->getX();
	curY = evt->getY();

	//std::cout << "BUTTON:"<<button<<"\n";
	DefaultManager::processEvent(evt, target);
	//std::cout << "evt type: " << evt->getType() << " presstype: " << pressType;
	if (evt->getType() == pressType)
		input->onPress(evt->getX(), evt->getY(), button, press);
	if (evt->getType() == releaseType)
		input->onRelease(evt->getX(), evt->getY(), button);
	if (evt->getType() == dragType)
		if ( !(input->active[DEVICEBUTTON] < 0) )
		{
			input->onDragging(evt->getX(), evt->getY(), press);
			input->setCursorPos(evt->getX(), evt->getY());
		}
		else
			input->setCursorPos(evt->getX(), evt->getY());
}

void MyApplicationManager::processKeyPress(string key, int modifier)
{
	input->keyPressed(key, modifier);
}

void MyApplicationManager::renderAll(bool enablePicking)
{
	std::vector<unsigned long> selectedIds;

	processTimedEvents();

	//render canvas
	canvas->draw();

	//std::cout<<"time: "<<frameTimer->getCurrentTime()<<"\n";

	renderScreenBorder();

	//doesn't render touch indicators, HACK
	LargeDisplayManager::renderOtherComponents(999999999,	enablePicking);

	renderMenu();

	renderTapWidget();
	
	//print various text to screen
	std::ostringstream conv;
	conv.precision(2);
	std::string text;

/*	conv.str("");
	conv << canvas->maxStrokeError;
	text = "Smoothing: ";
	text.append(conv.str());

	screenFont->renderText(text, displayWidth- 130.0f, 30.0f);
*/
	conv.str("");//clear
	conv << std::fixed << canvas->totalRenderVerts;
	text = "Rendered verts: ";
	text.append(conv.str());

	//debugFont->renderText(text, 70.0f, 15.0f);

	//std::cout<<"FPS: "<<frameTimer->calculateFrameRate()<<"\n";

	//renderAllTouchIndicators();

}

void MyApplicationManager::processTimedEvents()
{
	input->currentTime = frameTimer->getCurrentTime();
	input->updateTapWidget();
	canvas->animateReset(frameTimer->getCurrentTime());

	canvas->brush.calcDynaVars(curX, curY);
}

void MyApplicationManager::resize(unsigned int width, unsigned int height)
{
	//std::cout << width << "," << height << " ";
	input->setScreenSize(width, height);
	canvas->setScreenSize(width, height);
	DefaultManager::resize(width, height);
}

void MyApplicationManager::writeCommandLog()
{
	input->writeCommandLog();
}