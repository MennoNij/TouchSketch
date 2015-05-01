#pragma once

#include <LDF/VisComponentStrategy.h>

class InputHandler;

class ButtonStrategy : public VisComponentStrategy
{
public:
	ButtonStrategy(unsigned int texUp, unsigned int texPressed, unsigned int texDown, int butnID = 0, InputHandler* inpt = NULL);
	~ButtonStrategy();

protected:
	void draw(const std::vector<unsigned long>& selectedIds);
	void drawForPicking();
	void onEvent(LargeDisplayEvent* evt);

private:
	bool pressed;
	int buttonID;
	unsigned int texButtonUp;
	unsigned int texButtonDown;
	unsigned int texButtonPressed;
	InputHandler* input;
};
