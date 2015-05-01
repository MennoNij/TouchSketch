#include "TapWidget.h"

TapWidget::TapWidget()
{
	diameter = 40.0f;
	duration = 1200; //duration in miliseconds
	visibility = 0.9f;
	activationTime = 0;
	pos = Vector2();
	active = false;
}

TapWidget::~TapWidget()
{

}

void TapWidget::enable(float x, float y, unsigned long actTime)
{
	active = true;
	pos = Vector2(x, y);
	activationTime = actTime;
}

void TapWidget::disable()
{
	active = false;
	visibility = 0.9f;
}

void TapWidget::update(unsigned long time)
{
	if (active)
	{
		if ((time-activationTime) >= duration)
		{
			disable();
		}
		else
		{
			visibility = 1.0f - ( (float)(time-activationTime) / (float)duration );
		}
	}
}

bool TapWidget::activeWidget(float x, float y)
{
	if (active)
	{
		Vector2 cursor = Vector2(x, y);
		Vector2 dist = cursor - pos;
		if (dist.length() <= (diameter*0.5f))
			return 1;
	}

	return 0;
}
