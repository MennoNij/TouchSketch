#include "ButtonStrategy.h"

#include <LDF/VisComponent.h>
#include <LDFToolkit/ConstantProvider.h>
#include <LDF/LargeDisplayEvent.h>

#include <windows.h>
#include <GL/gl.h>
#include <iostream>

//#include "glext.h"

#include "InputHandler.h"

ButtonStrategy::ButtonStrategy(unsigned int texUp, unsigned int texPressed, unsigned int texDown, int butnID, InputHandler* inpt) : VisComponentStrategy()
{
	texButtonUp = texUp;
	texButtonDown = texDown;
	texButtonPressed = texPressed;
	pressed = false;
	buttonID = butnID;
	input = inpt;
	
	/*GLuint texIndx = glGenLists(2);
	glNewList(texIndx, GL_COMPILE);
		glBindTexture(GL_TEXTURE_2D, texButtonDown);
		glBegin(GL_QUAD_STRIP);
			glTexCoord2d(0.0,0.0); glVertex3fv(v0);
			glTexCoord2d(1.0,0.0); glVertex3fv(v1);
			glTexCoord2d(0.0,1.0); glVertex3fv(v3);
			glTexCoord2d(1.0,1.0); glVertex3fv(v2);
		glEnd();	
	glEndList();*/
}

ButtonStrategy::~ButtonStrategy()
{
}

void ButtonStrategy::draw(const std::vector<unsigned long>& selectedIds)
{
	// Checking if this component is selected
	bool isSelected = false;
	for (unsigned int i = 0; i < selectedIds.size(); i++)
		if (component->getId() == selectedIds[i]) isSelected = true;

	float w2 = component->getWidth() / 2.0f;
	float h2 = component->getHeight() / 2.0f;

	// Starting at the lower left corner and going counter-clockwise
	float v0[3], v1[3], v2[3], v3[3];
	v3[0] = -w2; v3[1] = -h2; v3[2] = 0;
	v2[0] = w2; v2[1] = -h2; v2[2] = 0;
	v1[0] = w2; v1[1] = h2; v1[2] = 0;
	v0[0] = -w2; v0[1] = h2; v0[2] = 0;
	
	glEnable(GL_TEXTURE_2D);

	if (pressed) glBindTexture(GL_TEXTURE_2D, texButtonPressed);
	else if (input->active[GUIBUTTON] == buttonID) glBindTexture(GL_TEXTURE_2D, texButtonDown);
	else glBindTexture(GL_TEXTURE_2D, texButtonUp);
	glBegin(GL_QUAD_STRIP);
		glTexCoord2d(0.0,0.0); glVertex3fv(v0);
		glTexCoord2d(1.0,0.0); glVertex3fv(v1);
		glTexCoord2d(0.0,1.0); glVertex3fv(v3);
		glTexCoord2d(1.0,1.0); glVertex3fv(v2);
	glEnd();

	glDisable(GL_TEXTURE_2D);
}

void ButtonStrategy::drawForPicking()
{
	float w2 = component->getWidth() / 2.0f;
	float h2 = component->getHeight() / 2.0f;

	// Starting at the lower left corner and going counter-clockwise
	float v0[3], v1[3], v2[3], v3[3];
	v0[0] = -w2; v0[1] = -h2; v0[2] = 0;
	v1[0] = w2; v1[1] = -h2; v1[2] = 0;
	v2[0] = w2; v2[1] = h2; v2[2] = 0;
	v3[0] = -w2; v3[1] = h2; v3[2] = 0;

	unsigned char pickingColor[3] = { 0, 0, 0 };
	component->getIdColor(pickingColor);
	glColor3ubv(pickingColor);
	glBegin(GL_QUAD_STRIP);
		glVertex3fv(v0);
		glVertex3fv(v1);
		glVertex3fv(v3);
		glVertex3fv(v2);
	glEnd();
}

void ButtonStrategy::onEvent(LargeDisplayEvent* evt)
{
	if (evt) {
		unsigned int evtType = evt->getType();
		ConstantProvider* constants = ConstantProvider::getInstance();
		if (evtType == constants->getEventTypeIdentifier("PRESS"))
		{
			if (component->isInside((unsigned int) evt->getX(), (unsigned int) evt->getY())) {
				pressed = true;
				//forward press event to input handler
				input->buttonPushed(buttonID);
			}
		} 
		else if (evtType == constants->getEventTypeIdentifier("RELEASE"))
		{
			pressed = false;
		}
	}

}