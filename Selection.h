#pragma once

#include <vector>

#include "BoundingBox.h"

#define uint unsigned int

using std::vector;

enum BoxCorners { BL, TL, TR, BR };
enum SelectButtons { SELECTCANCEL, SELECTDELETE, SELECTBRUSH };

class Canvas;
class Stroke;

struct Selection
{
	BoundingBox bounds;
	bool active;
	bool activeMulti;
	bool activeErase;
	bool scaleMode;
	float widgetSize;
	int scalingAnchor;
	Canvas* canvas;
	uint widgetTex;
	uint widgetPressTex;
	uint widgetAnchorTex;
	float borderWidth;
	float totalRotation;

	Vector2 corner[4];
	Vector2 n[4];

	Vector2 multiStart;
	Vector2 multiEnd;

	Vector2 eraseStart;
	Vector2 eraseEnd;

	vector<Vector2> selectLine;

	uint buttonTex[3];
	/*Vector2 drawTL;
	Vector2 drawTR;
	Vector2 drawBL;
	Vector2 drawBR;

	Vector2 nTR;
	Vector2 nBL;
	Vector2 nBR;
	Vector2 nTL;*/

	Selection();
	~Selection();

	void select(Stroke* id);
	void unselect();
	void draw(float zoom);
	int activeWidget(float x, float y, float scale);
	Vector2 getWidgetPos(int widget);
	Vector2 getNormalDir(int widget);
	Vector2 getEdgeNormal(int widget);
	void updateWidgetPositions();
	void setWidgetPositions(Vector2 tl, Vector2 tr, Vector2 bl, Vector2 br);
	void setButtonTex(uint tex[]);
	void move(Vector2 di);
	void rotate(float angle);
	void scale(float scaleX, float scaleY, int anchor);
	inline float calcDistToEdge(Vector2& p1, Vector2& p2, Vector2& p);

	void startMultiSelect(float x, float y);
	void updateMultiSelect(float x, float y);
	void endMultiSelect();

	void startMultiErase(float x, float y);
	void updateMultiErase(float x, float y);
	void endMultiErase();

	void drawMultiSelector();
	void multiSelect(vector<Stroke*>& selected);

	void drawMultiErase();

	void drawLineSelector();
	void sampleLineSelectorPoint(float x, float y);
};
