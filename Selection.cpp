#include "Selection.h"

#include "Canvas.h"

using std::cout;

Selection::Selection()
{
	canvas = NULL;
	active = false;
	activeMulti = false;
	activeErase = false;
	scaleMode = false;
	scalingAnchor = 0;
	widgetSize = 25.0f;
	widgetTex = 0;
	widgetPressTex = 0;
	widgetAnchorTex = 0;
	borderWidth = 15.0f;
	totalRotation = 0.0f;
}
Selection::~Selection()
{
}

void Selection::select(Stroke* id)
{
	bounds = id->bounds;
	active = true;
	updateWidgetPositions();
}

void Selection::unselect()
{
	active = false;
	scalingAnchor = 0;
	scaleMode = false;
}

void Selection::updateWidgetPositions()
{
	corner[TL] = Vector2(bounds.pMin.x, bounds.pMax.y);
	n[TL] = Vector2(-1.0f, 1.0f);
	corner[BR] = Vector2(bounds.pMax.x, bounds.pMin.y);
	n[BR] = Vector2(1.0f, -1.0f);
	corner[TR] = Vector2(bounds.pMax.x, bounds.pMax.y);
	n[TR] = Vector2(1.0f, 1.0f);
	corner[BL] = Vector2(bounds.pMin.x, bounds.pMin.y);
	n[BL] = Vector2(-1.0f, -1.0f);
}

void Selection::setWidgetPositions(Vector2 tl, Vector2 tr, Vector2 bl, Vector2 br)
{
	corner[TL] = tl;
	corner[BR] = br;
	corner[TR] = tr;
	corner[BL] = bl;
}

void Selection::setButtonTex(uint tex[])
{
	for (int i=0; i<3; ++i)
		buttonTex[i] = tex[i];
}

void Selection::move(Vector2 di)
{
	for (int i=0; i<4; ++i)
		corner[i] += di;
	//bounds.move(di);
	//updateWidgetPositions();
}

void Selection::rotate(float angle)
{
	totalRotation += angle;

	Vector2 center = (bounds.pMin + bounds.pMax)*0.5f;

	for (int i=0; i<4; ++i)
	{
		corner[i] -= center;
		corner[i].rotateAroundZero(angle);
		corner[i] += center;

		n[i].rotateAroundZero(angle);
	}
}

void Selection::scale(float scaleU, float scaleV, int anchor)
{
	//anchor = 0;
	Vector2 origin = corner[anchor];

	//find u (an adjecent edge to the anchor)
	Vector2 edgeU = corner[(anchor+3)%4] - origin;
	Vector2 u = edgeU / edgeU.length();
	//v is perpendicular to u
	Vector2 v = Vector2(-u.y, u.x);

	for (int i=0; i<4; ++i)
	{
		corner[i] -= origin;
		float pu = corner[i].dot(u);
		float pv = corner[i].dot(v);

		pu *= scaleU;
		pv *= scaleV;

		corner[i] = u*pu + v*pv + origin;
	}
}

void Selection::draw(float zoom)
{
	if (!active)
		return;

	//draw selection widgets
	float wS = widgetSize/zoom;
	float bW = borderWidth/zoom;

	glColor4f(0.258f, 0.486f, 0.647f, 0.6f);

	glBegin(GL_TRIANGLE_STRIP);
	
	for (int i=0; i<4; ++i)
	{
		glVertex3f(corner[i].x, corner[i].y, 1.0);	
		glVertex3f(corner[i].x+n[i].x*bW, corner[i].y+n[i].y*bW, 1.0);	
		glVertex3f(corner[(i+1)%4].x, corner[(i+1)%4].y, 1.0);
		glVertex3f(corner[(i+1)%4].x+n[(i+1)%4].x*bW, corner[(i+1)%4].y+n[(i+1)%4].y*bW, 1.0);
	}

	glEnd();

	glLineWidth(2);
	glColor4f(0.258f, 0.486f, 0.647f, 1.0f);
	glBegin(GL_LINES);
	
	for (int i=0; i<4; ++i)
	{
		glVertex3f(corner[i].x, corner[i].y, 1.0);
		glVertex3f(corner[(i+1)%4].x, corner[(i+1)%4].y, 1.0);

		glVertex3f(corner[i].x+n[i].x*bW, corner[i].y+n[i].y*bW, 1.0);
		glVertex3f(corner[(i+1)%4].x+n[(i+1)%4].x*bW, corner[(i+1)%4].y+n[(i+1)%4].y*bW, 1.0);
	}
	glEnd();
	glLineWidth(1);

	glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, widgetTex);


	float halfWS = wS*0.5f;

	glColor3f(0.0,0.8,0.0);

	for (int i=0; i<4; ++i)
	{
		Vector2 cen = (corner[i]*2.0f+n[i]*bW)*0.5f;
		glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2d(0.0,0.0); glVertex2f(cen.x-halfWS, cen.y-halfWS);
		glTexCoord2d(0.0,1.0); glVertex2f(cen.x-halfWS, cen.y+halfWS);
		glTexCoord2d(1.0,0.0); glVertex2f(cen.x+halfWS, cen.y-halfWS);
		glTexCoord2d(1.0,1.0); glVertex2f(cen.x+halfWS, cen.y+halfWS);
		glEnd();
	}

	//draw buttons, first find direction
	Vector2 butDir = (corner[1]-corner[2]).normalized();
	float butRad = 15.0f/zoom; float butDist = (15.0f*3.0f)/zoom;
	Vector2 butPos = butDir*butDist;
	for (int i=0; i<3; ++i)
	{
		glBindTexture(GL_TEXTURE_2D, buttonTex[i]);
		glBegin(GL_QUADS);
		glTexCoord2d(0.0,1.0); glVertex2f(corner[1].x+butPos.x-butRad, corner[1].y+butPos.y-butRad); //BL
		glTexCoord2d(0.0,0.0); glVertex2f(corner[1].x+butPos.x-butRad, corner[1].y+butPos.y+butRad); //TL
		glTexCoord2d(1.0,0.0); glVertex2f(corner[1].x+butPos.x+butRad, corner[1].y+butPos.y+butRad); //TR
		glTexCoord2d(1.0,1.0); glVertex2f(corner[1].x+butPos.x+butRad, corner[1].y+butPos.y-butRad); //BR
		glEnd();
		
		butPos.rotateAroundZero(-45.0f*TO_RADIANS);
	}

	glDisable(GL_TEXTURE_2D);
}

int Selection::activeWidget(float x, float y, float scale)
{
	if (!active)
		return 0;

	canvas->toCanvasSpace(x,y);
	Vector2 p = Vector2(x,y);
	float wS = (widgetSize/scale)*0.5f;

	Vector2 cen[4];

	//corners
	for (int i=0; i<4; ++i)
	{
		cen[i] = (corner[i]*2.0f+n[i]*borderWidth)*0.5f;
		Vector2 dist = cen[i] - p;
		if (dist.length() < wS)
			return i;
	}

	//edges
	float bW = borderWidth*0.7f;
	for (int i=0; i<4; ++i)
	{
		float edgeDist = calcDistToEdge(cen[i], cen[(i+1)%4], p);
		if (edgeDist < bW)
			return i+4;
	}

	//buttons
	Vector2 butDir = (corner[1]-corner[2]).normalized();
	float butRad = 15.0f; float butDist = butRad*3.0f;
	Vector2 butPos = butDir*butDist;
	for (int i=0; i<3; ++i)
	{
		Vector2 dist = (butPos+corner[1]) - p;
		if (dist.length() < butRad)
			return i+8;
		
		butPos.rotateAroundZero(-45.0f*TO_RADIANS);
	}

	return -1;
}

inline float Selection::calcDistToEdge(Vector2& p1, Vector2& p2, Vector2& p)
{
	Vector2 n = Stroke::calcLineNormal(p1, p2);
	Vector2 p3 = p+n*999999.0f; Vector2 p4 = p-n*999999.0f;
	float t = ( (p4.x-p3.x)*(p1.y-p3.y) - (p4.y-p3.y)*(p1.x-p3.x) ) / 
             ( (p4.y-p3.y)*(p2.x-p1.x) - (p4.x-p3.x)*(p2.y-p1.y) );

	if (t > 0.0f && t < 1.0f)
	{
		Vector2 intersect = p1+(p2-p1)*t;
		return (intersect - p).length();
	}
	else
		return 9999999.0f;
}

Vector2 Selection::getWidgetPos(int widget)
{
	return corner[widget];
}

Vector2 Selection::getNormalDir(int widget)
{
	return n[widget];
}

Vector2 Selection::getEdgeNormal(int widget)
{
	int Wfirst = widget-4;
	return Stroke::calcLineNormal(corner[Wfirst], corner[(Wfirst+1)%4]);
}

void Selection::startMultiSelect(float x, float y)
{
	selectLine.clear();
	multiStart = Vector2(x, y);
	multiEnd = Vector2(x, y);
	activeMulti = true;
}

void Selection::updateMultiSelect(float x, float y)
{
	sampleLineSelectorPoint(x,y);
	multiEnd = Vector2(x, y);
}

void Selection::endMultiSelect()
{
	selectLine.clear();
	multiStart = Vector2();
	multiEnd = Vector2();
	activeMulti = false;
}

void Selection::startMultiErase(float x, float y)
{
	selectLine.clear();
	eraseStart = Vector2(x,y);
	eraseEnd = Vector2(x,y);
	activeErase = true;
}

void Selection::updateMultiErase(float x, float y)
{
	sampleLineSelectorPoint(x,y);
	eraseEnd = Vector2(x,y);
}

void Selection::endMultiErase()
{
	selectLine.clear();
	eraseEnd = Vector2();
	eraseStart = Vector2();
	activeErase = false;
}


void Selection::drawMultiSelector()
{
	glColor4f(0.258f, 0.486f, 0.647f, 0.6f);

	glBegin(GL_QUADS);
	glVertex2f(multiStart.x, multiStart.y);
	glVertex2f(multiStart.x, multiEnd.y);
	glVertex2f(multiEnd.x, multiEnd.y);
	glVertex2f(multiEnd.x, multiStart.y);
	glEnd();

	glColor4f(0.258f, 0.486f, 0.647f, 1.0f);

	glLineWidth(3.0f);
	glBegin(GL_LINE_STRIP);
	glVertex2f(multiStart.x, multiStart.y);
	glVertex2f(multiStart.x, multiEnd.y);
	glVertex2f(multiEnd.x, multiEnd.y);
	glVertex2f(multiEnd.x, multiStart.y);
	glVertex2f(multiStart.x, multiStart.y);
	glEnd();
	glLineWidth(1.0f);
}

void Selection::multiSelect(vector<Stroke*>& selected)
{
	if (selected.size() > 0)
	{
		bounds = selected[0]->bounds;
		for (uint i=1; i<selected.size(); ++i)
			bounds.merge(selected[i]->bounds);
		
		active = true;
		updateWidgetPositions();
	}
}

void Selection::drawMultiErase()
{
	glColor4f(0.258f, 0.486f, 0.647f, 0.6f);

	glLineWidth(3.0f);
	glBegin(GL_LINES);
	glVertex2f(eraseStart.x, eraseStart.y);
	glVertex2f(eraseEnd.x, eraseEnd.y);
	glEnd();
	glLineWidth(1.0f);
}

void Selection::drawLineSelector()
{
	glColor4f(0.258f, 0.486f, 0.647f, 0.6f);
	glLineWidth(3.0f);
	glBegin(GL_LINE_STRIP);
	for (uint i=0; i<selectLine.size(); ++i)
		glVertex2f(selectLine[i].x, selectLine[i].y);
	glEnd();
	glLineWidth(1.0f);
}

void Selection::sampleLineSelectorPoint(float x, float y)
{
	if (!selectLine.empty())
	{
		Vector2 p0 = selectLine.back();
		Vector2 p1 = Vector2(x,y);

		float dist = (p0-p1).length();

		if (dist > 5.0f)
			selectLine.push_back(p1);
	}
	else
		selectLine.push_back(Vector2(x,y));
}