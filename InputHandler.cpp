#include "InputHandler.h"

#include <iostream>
#define _USE_MATH_DEFINES
#include <cmath>

#include "Vector2.h"
#include "DrawBrush.h"
#include "Canvas.h"
#include "Stroke.h"

using std::cout;

InputHandler::InputHandler(Canvas* canv)
{
	canvas = canv;

	active[DEVICEBUTTON] = -1;
	active[MODEBUTTON] = DRAW;
	active[GUIBUTTON] = -1;
	active[SELECTIONWIDGET] = -1;
	active[MENUPRESSED] = false;
	active[INPUTMODE] = DRAW;
	active[MENUACTION] = -1;
	active[CANVASWIDGET] = 0;

	for (int i=0; i<INTERACTIONSIZE; ++i) {	 interaction[i] = false; }

	interaction[TYPEBORDER] = -1;
	interaction[TYPESELECTION] = -1;
	interaction[TYPEMENU] = -1;
	interaction[iTAPWIDGET] = false;

	alreadyPressed = false;
	alreadyReleased = true;

	//borderInteraction = false;

	screenWidth = screenHeight = 1;
	prevX1 = prevY1 = prevX2 = prevY2 = 1;
	pressX1 = pressY1 = 1;
	
	pressTime = 0;
	tapTime = 500;

	borderWidth = 60;
	cornerCircleRadius = borderWidth*1.80f;

	guiMenu = Menu(Vector2(200,150), 70.0f, 60.0f, this);

	currentTime = 0;

	currentSketch = 0;

	strokeStartTime = 0.0f;
}

InputHandler::~InputHandler()
{
}

//USER ROUTINES
void InputHandler::onPress(float x, float y, int button, float pressure)
{
	if (alreadyPressed) //guard against sensitive input devices
		return;

	pressTime = currentTime;
	alreadyPressed = true;
	alreadyReleased = false;
	active[DEVICEBUTTON] = button;
	
	//keep track of press coords to determine if the screen was tapped later on
	pressX1 = x; pressY1 = y;

	int widget = determineWidgetType(x, y); //what is the user interacting with

	switch (widget)
	{
	case CANVAS:
		if (active[CANVASWIDGET] == 1) //prev
		{
			active[CANVASWIDGET] = 0;
			if (currentSketch > 0) //save current, load previous
			{
				std::string s;
				std::stringstream out;
				//save current
				out << currentSketch;
				s = out.str(); s.append(".xml");
				canvas->saveDrawing(s);
				out.str("");
				//load previous
				--currentSketch;
				out << currentSketch;
				s = out.str(); s.append(".xml");
				canvas->loadDrawing(s);
			}
		}
		else if (active[CANVASWIDGET] == 2) //next
		{
			active[CANVASWIDGET] = 0;

			std::string s;
			std::stringstream out;
			//save current
			out << currentSketch;
			s = out.str(); s.append(".xml");
			canvas->saveDrawing(s);
			out.str("");
			//load previous
			++currentSketch;
			out << currentSketch;
			s = out.str(); s.append(".xml");
			canvas->loadDrawing(s);
		}
		else if (!canvas->selection.active)
		{
			if (button == PRIMARY)
			{
				if (active[INPUTMODE] == DRAW)
				{
					logCommand(useBrush);
					strokeStartPos = Vector2(x,y);
					strokeStartTime = currentTime;

					//The user is starting a line
					canvas->startNewLine(x, y, pressure);
				}
				else if (active[INPUTMODE] == EDIT)
				{
					logCommand(useMoveBrush);
					//The user is starting a move
					canvas->startMoveBrush(x, y, pressure);
				}
			}
			else if (button == SECONDARY)
			{
				if (active[INPUTMODE] == DRAW) //the user is erasing
				{
					canvas->eraseFromStroke(x, y, pressure);
				}
				else if (active[INPUTMODE] == EDIT)
				{
					logCommand(useMoveStroke);
					//user is starting a move stroke
					canvas->startMoveStroke(x, y, pressure);
				}
			}
		}
		break;
	
	case BORDER:
		logCommand(useBorder);
		//starting an interaction with the border
		interaction[iBORDER] = true;
		break;
	case MENU:
		//cout<<"MENU\n";
		active[MENUPRESSED] = true; //don't trigger a tap from pressing the menu
		if (active[MENUACTION] == CHANGEPRESSURE)
		{
			canvas->brush.varyWithPressure = (canvas->brush.varyWithPressure+1)%3;
		}
		else if  (active[MENUACTION] == CHANGESPEED)
		{
			canvas->brush.varyWithSpeed = (canvas->brush.varyWithSpeed+1)%3;
		}
		
		break;
	case SELECTWIDGET:
		logCommand(useSelection);
		interaction[iSELECTION] = true;
		break;
	case TAPWIDGET:
		logCommand(useTap);
		interaction[iTAPWIDGET] = true;
		break;
	}

	prevX1 = x; prevY1 = y;
}

void InputHandler::onRelease(float x, float y, int button)
{
	alreadyPressed = false;
	if (active[MENUPRESSED]) //handle menu interaction before 'tap'
	{
		guiMenu.setDefaultPos();
		active[MENUPRESSED] = false;
		active[MENUACTION] = -1;
		active[GUIBUTTON] = -1;
		return;
	}

	if (alreadyReleased) //guard against sensitive input devices
		return;
	alreadyReleased = true;

	//check if the screen was 'tapped'
	bool tapped = false;
	float tapDistSqr = (x-pressX1)*(x-pressX1) + (y-pressY1)*(y-pressY1);
	if ( (currentTime-pressTime > tapTime && tapDistSqr < 25.0f) || canvas->selection.active)
	{
		tapped = true;

		if (interaction[iTAPWIDGET]) //tapped active tapwidget
		{
			//cout<<"tapped tapwidget\n";
			tapWidget.disable();
			guiMenu.setMenuPos(Vector2(x,y));
			interaction[iTAPWIDGET] = false;
		}
		else if (button == PRIMARY)
		{
			if(active[SELECTIONWIDGET] < 0)
			{
				if (!canvas->selectStroke(x,y)) //no closeby stroke was clicked
				{
					if (canvas->selection.active) //but there is a selection
					{
						canvas->selection.active = false;
						canvas->selectedStrokes.clear();
					}
					else
					{
						tapWidget.enable(x, y, currentTime);
					}
				}
			}
			else  //tapped a selection widget
			{
				//cout<<"tapped selection: "<<active[SELECTIONWIDGET]<<"\n";
				if (active[SELECTIONWIDGET] == 8) //cancel selection
					canvas->selection.unselect();
				else if (active[SELECTIONWIDGET] == 9) //delete selection
				{
					canvas->removeSelection();
					canvas->selection.unselect();
				}
				else if (active[SELECTIONWIDGET] == 10)
				{
					canvas->setMoveStroke();
					canvas->selection.unselect();
				}
			}
		}
		else if (button == SECONDARY)
		{
			if (canvas->selection.active)
			{
				//possibly add or remove strokes from the selection

			}
			else
			{
				tapWidget.enable(x, y, currentTime);
			}
		}

		//std::cout<<"TAP\n";
	}

	if (interaction[iBORDER])	//user is ending a border interaction	
	{
		if (interaction[TYPEBORDER] == BOR_ZOOMIN || interaction[TYPEBORDER] == BOR_ZOOMOUT || interaction[TYPEBORDER] >= BOR_BL)
		{
			//when done zooming, possible update stroke geometry
			canvas->finalizeZoom();
		}
		interaction[iBORDER] = false;
		interaction[TYPEBORDER] = -1;
	}
	else if (interaction[iSELECTION])	 //user is ending a selection interaction
	{
		interaction[iSELECTION] = false;
		interaction[TYPESELECTION] = -1;
		active[SELECTIONWIDGET] = -1;
		canvas->finalizeSelectionAction();
	}
	else if (interaction[iTAPWIDGET]) //user is ending a multi selection interaction
	{
		interaction[iTAPWIDGET] = false;
		if (canvas->selection.activeMulti || canvas->selection.activeErase)
		{
			if (button == PRIMARY)
				canvas->endMultiSelect(x, y);
			else if (button == SECONDARY)
				canvas->endMultiErase(x, y);
		}
	}
	else if (!canvas->selection.active)
	{
		if (button == PRIMARY) 
		{
			if (active[INPUTMODE] == DRAW)
			{
				//The user is ending a line
				logAngles(x,y);
				canvas->endNewLine(x, y);
			}
			if (active[INPUTMODE] == EDIT)
			{
				//The user is ending a move
				canvas->endMoveBrush();
			}
		}
		else if (button == SECONDARY)
		{
			if (active[INPUTMODE] == EDIT)
			{
				//user is ending a move stroke
				canvas->endMoveStroke();
			}
		}
	}

	//always deactivate these
	canvas->showDrawBrush = false;
	canvas->showMoveBrush = false;

	active[DEVICEBUTTON] = -1; //none active
	prevX1 = x; prevY1 = y;
}

void InputHandler::onDragging(float x, float y, float pressure)
{
	float dx = x - prevX1;
	float dy = y - prevY1;

	//check if we're not dragging outside of the screen
	if (x < 5 || x > screenWidth-5 || y < 5 || y > screenHeight-5)
		return;

	if (active[MENUPRESSED])
	{
		doMenuInteraction(x, y, dx, dy);
	}
	else if (interaction[iBORDER])
	{
		//the user is interacting with the border to move, rotate or scale
		doBorderInteraction(x, y, dx, dy);
	}
	else if (interaction[iSELECTION] > 0)
	{
		//canvas->scaleStrokes(x, y, dx, dy, activeSelectionWidget);
		//the user is interacting with a selection widget
		doSelectionInteraction(x, y, dx, dy);
		//canvas->rotateAndTranslate(x, y, dx, dy, active[SELECTIONWIDGET]);
	}
	else if (interaction[iTAPWIDGET])
	{
		//cout<<"dragging from tap\n";
		if (active[DEVICEBUTTON] == PRIMARY)
		{
			if (!canvas->selection.activeMulti) //start a multiselection
			{
				logCommand(useTapSelect);
				canvas->startMultiSelect(x, y);
			}
			else //extend a multiselection
				canvas->extendMultiSelect(x, y);
		}
		else if (active[DEVICEBUTTON] == SECONDARY)
		{
			if (!canvas->selection.activeErase)
			{
				logCommand(useTapDel);
				canvas->startMultiErase(x, y);
			}
			else
				canvas->extendMultiErase(x, y);
		}
	}
	else if (!canvas->selection.active)
	{
		if (active[DEVICEBUTTON] == PRIMARY)
		{
			if (active[INPUTMODE] == DRAW)
			{
				//The user is drawing a line
				canvas->extendNewLine(x, y, dx, dy, pressure);
			}
			else if (active[INPUTMODE] == EDIT)
			{
				//The user is dragging the move brush
				canvas->dragMoveObject(prevX1, prevY1, dx, dy, pressure);
			}
		}
		else if (active[DEVICEBUTTON] == SECONDARY)
		{
			if (active[INPUTMODE] == DRAW)
				canvas->eraseFromStroke(x, y, pressure);
			else if (active[INPUTMODE] == EDIT)
				canvas->dragMoveObject(prevX1, prevY1, dx, dy, pressure);
		}
	}

	prevX1 = x; prevY1 = y;
	canvas->storeDxDy(dx, dy);

}

//GUI ROUTINES

void InputHandler::buttonPushed(int id)
{
	active[GUIBUTTON] = id;
	switch(id)
	{
	case DRAW:
		active[INPUTMODE] = DRAW;
		active[MODEBUTTON] = id;
		canvas->drawMode = DRAW;
		break;
	case EDIT:
		active[INPUTMODE] = EDIT;
		active[MODEBUTTON] = id;
		canvas->drawMode = EDIT;
		break;
	case CLEAR:
		canvas->clearCanvas();
		break;
	case RESETCANVAS:
		canvas->resetView(currentTime);
		break;
	default:
		break;
	}
}

void InputHandler::keyPressed(string& key, int modifier)
{
	if (key == "w")
		canvas->showWireframe = !canvas->showWireframe;
	else if (key == "c")
		canvas->showCPs = !canvas->showCPs;
	else if (key == "t")
		canvas->lineTree.treeDebug = !canvas->lineTree.treeDebug;
	else if (key == "[")
		canvas->maxStrokeError -= 0.5f;
	else if (key == "]")
		canvas->maxStrokeError += 0.5f;
	else if (key == "C")
		canvas->clearCanvas();
	else if (key == "S")
	{
		std::string s;
		std::stringstream out;
		//save current
		out << currentSketch;
		s = out.str(); s.append(".xml");
		canvas->saveDrawing(s);
		//canvas->saveDrawing(std::string("test.xml"));
	}
	else if (key == "L")
	{
		std::string s;
		std::stringstream out;
		//save current
		out << currentSketch;
		s = out.str(); s.append(".xml");
		canvas->loadDrawing(s);
		//canvas->loadDrawing(std::string("test.xml"));
	}
	else if (key == "E")
	{
		std::string s;
		std::stringstream out;
		//save current
		out << currentSketch;
		s = out.str(); s.append(".svg");
		canvas->exportDrawing(s);
		//canvas->loadDrawing(std::string("test.xml"));
	}
}

int InputHandler::determineWidgetType(float x, float y)
{
	Vector2 pos = Vector2(x,y);
	//check if the user interacted with the menu
	
	int menuVal = guiMenu.activeWidget(pos);
	
	if (menuVal == 0)
		return MENU;
	else if (menuVal > 0)
	{
		active[MENUACTION] = menuVal;
		return MENU;
	}

	//check if the user interacted with the screen border
	if (Vector2(x, y).length() < cornerCircleRadius) //bottom-left corner
	{
		logCommand(useBorScale);
		interaction[TYPEBORDER] = BOR_BL;	
		return BORDER;	
	}
	else if (Vector2(screenWidth-x, y).length() < cornerCircleRadius) //bottom-right corner
	{
		logCommand(useBorScale);
		interaction[TYPEBORDER] = BOR_BR;
		return BORDER;
	}
	else if (Vector2(x, screenHeight-y).length() < cornerCircleRadius) //bottom-right corner
	{
		logCommand(useBorScale);
		interaction[TYPEBORDER] = BOR_TL;
		return BORDER;
	}
	else if (Vector2(screenWidth-x, screenHeight-y).length() < cornerCircleRadius) //top-right corner
	{
		logCommand(useBorScale);
		interaction[TYPEBORDER] = BOR_TR;
		return BORDER;
	}

	if (x < borderWidth || x > screenWidth - borderWidth)
		return BORDER;
	if (y < borderWidth || y > screenHeight - borderWidth)
		return BORDER;

	if (tapWidget.activeWidget(x, y))
		return TAPWIDGET;
	/*Vector2 drawPos = menuPos - Vector2(menuButtonSize*0.75f,0);
	Vector2 editPos = menuPos + Vector2(menuButtonSize*0.75f,0);

	Vector2 drawDist = pos - drawPos;
	if (drawDist.length() <= menuButtonSize*0.5f)
		return MENU;

	Vector2 editDist = pos - editPos;
	if (editDist.length() <= menuButtonSize*0.5f)
		return MENU;*/

	//check if the user is interaction with a selection border
	if (canvas->selection.active)
	{
		active[SELECTIONWIDGET] = canvas->selection.activeWidget(x, y, canvas->zoom);
		//cout<<"active widget: "<<active[SELECTIONWIDGET]<<"\n";
		if (active[SELECTIONWIDGET] >= 0)
			return SELECTWIDGET;
	}
//	std::cout<<"selection: "<<activeSelectionWidget<<"\n";
	//must be canvas, check canvas widgets
	active[CANVASWIDGET] = canvas->activeWidget(x, y);

	return CANVAS; //interacted with canvas
}

void InputHandler::doMenuInteraction(float x, float y, float dx, float dy)
{
	Vector2 dirOfMotion = Vector2(x - pressX1, y - pressY1);
	if (dirOfMotion.length() < 5.0f)
		return; //line isn't long enough to determine type

	if (active[MENUACTION] < 1)
		return;
	
	//location of the button
	Vector2 buttonLoc = Vector2(-(guiMenu.propertiesCircleRadius+guiMenu.menuButtonSize*0.5f), 0.0f);
	float rotAngle = 0.0f;
	if (active[MENUACTION] < MOVEBRUSHSIZE)
		rotAngle = guiMenu.drawMenuAngles[active[MENUACTION]-1];
	else
		rotAngle =  guiMenu.editMenuAngles[active[MENUACTION]-1-NUMDRAWPROPS];

	buttonLoc.rotateAroundZero(rotAngle*TO_RADIANS);
	Vector2 n = -buttonLoc.normalized();
	buttonLoc += guiMenu.menuPos;

	Vector2 prevDist = Vector2(prevX1, prevY1) - buttonLoc;
	Vector2 dist = Vector2(x,y) - buttonLoc;

	float angle = n.calcAngleWith(prevDist)*TO_DEGREES;
	float amount = Vector2(dx, dy).length();
//cout<<"menu interaction\n";
	if (active[MENUACTION] == BRUSHSIZE)
	{
		float newSize = canvas->brush.radius;
		
		if (angle < 90.0f) //increase
			if (prevDist.length() < dist.length())
				newSize += amount*0.25f;
			else
				newSize -= amount*0.25f;
		else //decrease
			if (prevDist.length() < dist.length())
				newSize -= amount*0.25f;
			else
				newSize += amount*0.25f;

		if (newSize >= 0.5f && newSize < 25.5f) //clamp brush size
			canvas->brush.radius = newSize;
	}
	else if (active[MENUACTION] == BRUSHSMOOTH)
	{
		float strError = canvas->maxStrokeError;
		if (angle < 90.0f) //increase
			if (prevDist.length() < dist.length())
				strError *= 1.0f+amount*0.15f;
			else
				strError *= 1.0f-amount*0.15f;
		else //decrease
			if (prevDist.length() < dist.length())
				strError *= 1.0f-amount*0.15f;
			else
				strError *= 1.0f+amount*0.15f;
		cout<<"error: "<<strError<<"\n";
		if (strError <= 10.0f && strError > 0.5f)
			canvas->maxStrokeError = strError;
		else 
			 if (canvas->maxStrokeError > 10.0f)
				 canvas->maxStrokeError = 10.0f;
			 else if (canvas->maxStrokeError < 0.5f)
				 canvas->maxStrokeError = 1.0f;
	}
	else if (active[MENUACTION] == MOVEBRUSHSIZE)
	{
		if (angle < 90.0f) //increase
			if (prevDist.length() < dist.length())
				canvas->moveBrush.scaleBrush(1.0f+amount*0.05f, 70.0f);
			else
				canvas->moveBrush.scaleBrush(1.0f-amount*0.05f, 70.0f);
		else //decrease
			if (prevDist.length() < dist.length())
				canvas->moveBrush.scaleBrush(1.0f-amount*0.05f, 70.0f);
			else
				canvas->moveBrush.scaleBrush(1.0f+amount*0.05f, 70.0f);
	}
	else if (active[MENUACTION] == MOVESOFT)
	{
		if (angle < 90.0f) //increase
			if (prevDist.length() < dist.length())
				canvas->moveBrush.scaleSoftness(1.0f+amount*0.05f);
			else
				canvas->moveBrush.scaleSoftness(1.0f-amount*0.05f);
		else //decrease
			if (prevDist.length() < dist.length())
				canvas->moveBrush.scaleSoftness(1.0f-amount*0.05f);
			else
				canvas->moveBrush.scaleSoftness(1.0f+amount*0.05f);
	}
	else if (active[MENUACTION] == CHANGEPRESSURE)
	{
		//if (canvas->brush.varyAlphaWith == 0) //alpha was being varied with pressure
		//cout<<"change!\n";
		canvas->brush.varyWithPressure = (canvas->brush.varyWithPressure+1)%2;
	}
	else if (active[MENUACTION] == CHANGESPEED)
	{

	}
}

void InputHandler::doBorderInteraction(float x, float y, float dx, float dy)
{
	float cenX = 0.5f*screenWidth;
	float cenY = 0.5f*screenHeight;
	
	if (interaction[TYPEBORDER] < 0) //type of interaction still has to be determined
	{
		Vector2 dirOfMotion = Vector2(x - pressX1, y - pressY1);
		if (dirOfMotion.length() < 5.0f)
			return; //line isn't long enough to determine type

		//get normal of closest screen edge
		Vector2 edgeNormal;
		if (pressY1 < borderWidth) //bottom
				edgeNormal = Vector2(0.0f,1.0f);
		else if (pressY1 > screenHeight - borderWidth) //top
				edgeNormal = Vector2(0.0f,-1.0f);
		else if (pressX1 < borderWidth ) //left
				edgeNormal = Vector2(1.0f,0.0f);
		else //right
				edgeNormal = Vector2(-1.0f,0.0f);

		float angle = edgeNormal.calcAngleWith(dirOfMotion)*TO_DEGREES;

		//if the angle is more towards the screen center, it's a translation
		if (angle < 70.0f)
		{
			logCommand(useBorMove);
			interaction[TYPEBORDER] = BOR_MOVE;
		}
		else
		{
			logCommand(useBorRot);
			interaction[TYPEBORDER] = BOR_ROTATE;
		}

	}
	else
	{
		//perform the interaction
		if (interaction[TYPEBORDER] == BOR_MOVE)
			canvas->applyMove(dx, dy);
		else if (interaction[TYPEBORDER] ==  BOR_ROTATE)
		{
			Vector2 curPos = Vector2(x-cenX, y-cenY);
			Vector2 prevPos = Vector2(curPos.x + dx, curPos.y + dy);
			float rot = prevPos.calcSignedAngleWith(curPos);
			canvas->applyRotation(rot);
		}
		else //interaction with corners
		{
			guiMenu.showInProperties = propZOOM;

			//find coordinate of the corner
			Vector2 corner;
			Vector2 n;
			float mirror = 0.1f; //fix to mirror right-side screen behavior to the left side
			if (interaction[TYPEBORDER] == BOR_BL)
			{
				corner = Vector2(0,0);
				n = Vector2(1,1);
				mirror = -mirror;
			}
			else if (interaction[TYPEBORDER] == BOR_BR)
			{			
				corner = Vector2(screenWidth, 0);
				n = Vector2(-1,1);
			}
			else if (interaction[TYPEBORDER] == BOR_TL)
			{			
				corner = Vector2(0, screenHeight);
				n = Vector2(1,-1);
				mirror = -mirror;
			}
			else
			{
				corner = Vector2(screenWidth, screenHeight);
				n = Vector2(-1,-1);
			}

			Vector2 pressDist = Vector2(pressX1, pressY1) - corner;
			Vector2 dist = Vector2(x,y) - corner;
			//check if inside 'perma zoom' area

			//otherwise zoom according to gesture motion
			Vector2 prevDist = corner - Vector2(prevX1, prevY1);
			
			float side = n.calcSignedAngleWith(pressDist);

			float amount = Vector2(dx, dy).length();
			if (side < 0)
				if (prevDist.length() < dist.length())
					canvas->applyZoom(-amount*mirror);
				else
					canvas->applyZoom(amount*mirror);
			else
				if (prevDist.length() < dist.length())
					canvas->applyZoom(amount*mirror);
				else
					canvas->applyZoom(-amount*mirror);
		}
	}
}

void InputHandler::doSelectionInteraction(float x, float y, float dx, float dy)
{
	Vector2 center = (canvas->selection.bounds.pMin + canvas->selection.bounds.pMax)*0.5f;
	//canvas->toCanvasSpace(x, y);

	if (interaction[TYPESELECTION] < 0) //type has to be determined
	{
		if (active[SELECTIONWIDGET] > 3) //translation or rotation
		{
			Vector2 dirOfMotion = Vector2(x - pressX1, y - pressY1);
			if (dirOfMotion.length() < 5.0f)
				return; //line isn't long enough to determine type

			canvas->rotateToCanvasSpace(dirOfMotion.x, dirOfMotion.y);

			Vector2 widgetNormal = canvas->selection.getEdgeNormal(active[SELECTIONWIDGET]);
			float angle = widgetNormal.calcAngleWith(dirOfMotion)*TO_DEGREES;

			if (angle < 70.0f)
			{
				logCommand(useSelMove);
				interaction[TYPESELECTION] = SEL_MOVE;
			}
			else
			{
				logCommand(useSelRot);
				interaction[TYPESELECTION] = SEL_ROTATE;
			}
		}
		else //the corners; either scaling or rotation
		{
			Vector2 dirOfMotion = Vector2(x - pressX1, y - pressY1);
			if (dirOfMotion.length() < 5.0f)
				return; //line isn't long enough to determine type

			Vector2 n = canvas->selection.getNormalDir(active[SELECTIONWIDGET]);
			float angle = n.calcAngleWith(dirOfMotion)*TO_DEGREES;

			if (angle < 45.0f || angle > 135.0f)
			{
				logCommand(useSelScale);
				interaction[TYPESELECTION] = SEL_SCALE;
			}
			else //otherwise scale
			{
				logCommand(useSelRot);
				interaction[TYPESELECTION] = SEL_ROTATE;
			}
		}
	}
	else //do interaction
	{
		if (interaction[TYPESELECTION] == SEL_MOVE)
		{
			canvas->applyMoveSelection(dx, dy);
		}
		else if (interaction[TYPESELECTION] == SEL_ROTATE)
		{
			canvas->toCanvasSpace(x, y);
			canvas->rotateToCanvasSpace(dx, dy);
			Vector2 curPos = Vector2(x, y) - center;
			Vector2 prevPos = curPos + Vector2(dx, dy);
			float rot = prevPos.calcSignedAngleWith(curPos);

			canvas->applyRotateSelection(-rot);
		}
		else if (interaction[TYPESELECTION] == SEL_SCALE)
		{
			canvas->applyScaleSelection(x, y, dx, dy, active[SELECTIONWIDGET]);
		}
	}

}

void InputHandler::updateTapWidget()
{
	tapWidget.update(currentTime);
}

void InputHandler::setCursorPos(float x, float y)
{
	//cout<<x<<" "<<y<<"\n";
	cursorPos = Vector2(x,y);
	canvas->brush.setBrushPos(x,y);
	canvas->moveBrush.guiPos = cursorPos;
}

void InputHandler::logCommand(int command)
{
	commandList.push_back(command);
	commandTime.push_back( (float)(currentTime-startTime) / 1000.0f);
}

void InputHandler::logAngles(float x, float y)
{
	Vector2 strokeEndPos = Vector2(x,y);
	Vector2 avgStrokeDir = strokeEndPos - strokeStartPos;
	float angle = avgStrokeDir.calcSignedAngleWith(Vector2(1,0)) * TO_DEGREES; //angle in regards to a horizontal line

	//if (angle > 90.0) //keep angles between 0 and 90 degrees
	//	angle = abs(angle-180.0);

	float canvasAngle = canvas->canvasAngle;

	strokeStartPos = Vector2();

	//log data
	angleList.push_back(angle);
	angleList.push_back(canvasAngle);
	angleList.push_back( (float)(strokeStartTime-startTime) / 1000.0f );
	angleList.push_back( (currentTime-strokeStartTime) / 1000.0f );

	//cout<<"stroke angle: "<<angle<<" canvas angle: "<<canvasAngle<<"\n";
}

void InputHandler::writeCommandLog()
{
	TiXmlDocument doc;
	TiXmlElement* el;
	
	TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "", "" );  
	doc.LinkEndChild(decl);

	TiXmlElement* root = new TiXmlElement("CommandLog");  
	doc.LinkEndChild(root);

	//add all commands to the xml tree
	for(uint i=0; i<commandList.size(); ++i)
	{
		TiXmlElement* comm = new TiXmlElement( "c" );  
		root->LinkEndChild(comm);  
		comm->SetAttribute("v", commandList[i]);
		comm->SetDoubleAttribute("t", commandTime[i]);
	}

	//add angle information to the log
	for(uint i=0; i<angleList.size(); i+=4)
	{
		TiXmlElement* comm = new TiXmlElement( "a" );  
		root->LinkEndChild(comm);  
		comm->SetDoubleAttribute("an", angleList[i]);
		comm->SetDoubleAttribute("cAn", angleList[i+1]);
		comm->SetDoubleAttribute("sT", angleList[i+2]);
		comm->SetDoubleAttribute("dT", angleList[i+3]);
	}

	std::ostringstream conv;
	std::string filename;

	conv << startTime;
	conv << ".log";
	filename = conv.str();

	doc.SaveFile(filename.c_str());
}