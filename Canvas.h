#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <iostream>

#include "tinyxml.h"

#include "StrokeList.h"
#include "QuadTree.h"
#include "DrawBrush.h"
#include "MoveBrush.h"
#include "MoveStroke.h"
#include "Selection.h"

/*
Canvas is a container class for the strokes and BST structure which handles the user actions performed on them.
*/

#define WINDOW_SIZE 14
#define WINDOW_OVERLAP 3
#define uint unsigned int

#define CANVAS_WIDTH 900
#define CANVAS_HEIGHT 900

class Stroke;
class StrokeList;

using std::vector;

class Canvas
{
public:
	StrokeList strokes;
	QuadTree lineTree;
	Selection selection;
	
	vector<Stroke*> moveBuffer;
	vector<Stroke*> selectedStrokes;
	Stroke* strokeBuffer; //the new stroke currently being drawn
	int strokeBuffOffset; //last resampled point

	float prevDX[5]; float prevDY[5]; //store previous movement values to filter out noise

	float canvasAngle; //angle the canvas is rotated at
	float xTranslation, yTranslation; //amount the canvas has been moved in 2D
	float zoom; //zoom factor

	float maxStrokeError; //maximum divergence of the stroke from the input in pixels

	float screenWidth, screenHeight;

	bool showWireframe; //render in wireframe mode
	bool showCPs; //show control points

	unsigned long prevAnimTime;
	int resetTime;
	int totalAnimTime;
	float animZoomStep;
	float animRotStep;

	//vector<DrawBrush> brushList; //collection of defined draw brushes
	//int activeBrush; //current brush ID
	DrawBrush brush;

	//vector<MoveBrush> mbrushList; //the list of movebrushes
	//int activeMBrush; //active movebrush
	MoveBrush moveBrush;

	MoveStroke moveStroke;

	float minSampleRate; //minimal euclidian distance between drawbrush samples in screen pixels
	float moveDist; //distance between samples traveled sofar

	uint totalRenderVerts; //total number of vertices rendered
	
	bool showMoveBrush;
	bool showDrawBrush;
	bool showStrokeBrush;

	int drawMode;

	vector<ControlPoint> tempPoints;

	Vector2 min;
	Vector2 max;

	Canvas();
	~Canvas();

	void draw();
	void animateReset(unsigned long time);
	//void startAnimateReset(unsigned long time);

	void setStrokeBuffer(Stroke& newStroke);

	//## new line routines
	void startNewLine(float x, float y, float pressure);
	void extendNewLine(float x, float y, float dx, float dy, float pressure);
	void endNewLine(float x, float y);

	//## move brush routines
	void startMoveBrush(float x, float y, float pressure);
	void dragMoveObject(float x, float y, float dx, float dy, float pressure);
	void endMoveBrush();

	//## move stroke routines
	void startMoveStroke(float x, float y, float pressure);
	void endMoveStroke();

	//## erasing routines
	void eraseFromStroke(float x, float y, float pressure);
	void removeStroke(Stroke* id);

	bool selectStroke(float x, float y);

	void removeSelection();

	void startMultiSelect(float x, float y);
	void extendMultiSelect(float x, float y);
	void endMultiSelect(float x, float y);

	void startMultiErase(float x, float y);
	void extendMultiErase(float x, float y);
	void endMultiErase(float x, float y);

	//## alter position
	//void rotateAndTranslate(float x, float y, float dx, float dy, int selectWidget);
	void applyMoveSelection(float dx, float dy);
	void applyRotateSelection(float angle);
	//## scaling
	void applyScaleSelection(float x, float y, float dx, float dy, int dragWidget);
	void finalizeSelectionAction();

	void applyRotation(float rot);
	void applyMove(float dx, float dy);
	void applyZoom(float dzoom);
	void finalizeZoom();

	void setMoveStroke();

	void clearCanvas();
	void resetView(unsigned long time);

	void drawMoveBrush();

	void saveDrawing(std::string filename);
	void loadDrawing(std::string filename);

	void exportDrawing(std::string filename);

	int activeWidget(float x, float y);

	BoundingBox calcViewBounds(); //bounds of the view
	void toCanvasSpace(float& x, float& y); //transforms from screen space to canvas space
	void toScreenSpace(float& x, float& y, Vector2 center); //transforms from canvas space to screen space
	inline void rotateToCanvasSpace(float& x, float& y) //only perform the rotation
	{
		float tilt = -canvasAngle*TO_RADIANS;
		float xx = x; float yy = y;
		x = xx*cos(tilt) - yy*sin(tilt);
		y = xx*sin(tilt) + yy*cos(tilt);
	}
	void setScreenSize(unsigned int width, unsigned int height) { screenWidth = (float)width; screenHeight = (float)height; }

	inline void storeDxDy(float dx, float dy)
	{
		prevDX[0] = prevDX[1]; prevDY[0] = prevDY[1];
		prevDX[1] = prevDX[2]; prevDY[1] = prevDY[2];
		prevDX[2] = prevDX[3]; prevDY[2] = prevDY[3];
		prevDX[3] = prevDX[4]; prevDY[3] = prevDY[4];
		prevDX[4] = dx;        prevDY[4] = dy;
	}

	inline Vector2 calcAverageDxDy(int steps, float dx, float dy)
	{
		Vector2 dxdy = Vector2(dx,dy);
		for (int i=4; i>4-steps; --i)
			dxdy += Vector2(prevDX[i], prevDY[i]);
		return dxdy;
	}

private:
	inline void setCanvasView();
	inline void setBrushView();
};
