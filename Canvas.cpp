#include "Canvas.h"

#include <windows.h>
#include <GL/gl.h>
#include <stdlib.h>

#include "Stroke.h"
#include "DrawBrush.h"
#include "MoveBrush.h"
#include "Vector2.h"
#include "MergeTree.h"

using std::cout;

Canvas::Canvas()
{
	lineTree = QuadTree(CANVAS_WIDTH, CANVAS_HEIGHT, 8.0f);
	selection.canvas = this;

	strokeBuffer = NULL;
	strokeBuffOffset = 1;

	for (int i=0; i<5; ++i) {	 prevDX[i] = 0.0f; prevDY[i] = 0.0f; }

	showWireframe = false;
	showCPs = false;
	
	canvasAngle = 0.0f;
	xTranslation = yTranslation = 50.0f;
	zoom = 1.0f;

	maxStrokeError = 1.0f;

	screenWidth = screenHeight = 1;

	prevAnimTime = 0;
	totalAnimTime = 0;
	animZoomStep = 0.0f;
	animRotStep = 0.0f;
	resetTime = 1000;

	//brushList.push_back(DrawBrush(5.0f,0, 1, 0.0f, 0.0f, 0.0f));
	//brushList.push_back(DrawBrush(5.0f,2, 0, 0.0f, 0.0f, 0.0f));
	//activeBrush = 0;
	brush = DrawBrush(5.0f,1, 0, 0.0f, 0.0f, 0.0f);

	moveBrush = MoveBrush();
	moveBrush.addRadial(Radial(Vector2(0,0),25.0f,0.2f));

	minSampleRate = 4.0f;
	moveDist = 0.0f;

	totalRenderVerts = 0;

	showMoveBrush = false;
	showDrawBrush = false;
	showStrokeBrush = false;

	drawMode = 0;

	loadDrawing(std::string("0.xml"));
}
Canvas::~Canvas()
{
}

void Canvas::draw()
{
	totalRenderVerts = 0;

	glPushMatrix();
	setCanvasView();

	if (showWireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	BoundingBox viewBounds = calcViewBounds();

	//cout<<"view bound: "<<pmin.x<<","<<pmin.y<<" "<<pmax.x<<","<<pmax.y<<"\n";

	//pass vertex array of each stroke to openGL
	ListNode* it = strokes.front;
	Stroke* curStroke = strokes.get_front();
	for(uint i=0; i<strokes.size(); ++i)
	{
		if (!viewBounds.overlaps(curStroke->bounds)) //visibility culling
		{
			curStroke = strokes.get_next(it);
			continue;
		}

		//is the stroke inside the viewing area
		glColor4f(0,0,0,1.0);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);

		glVertexPointer(2, GL_FLOAT, 0, curStroke->vertices);
		glColorPointer(4, GL_FLOAT, 0, curStroke->colours);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, curStroke->numVerts);
		
		totalRenderVerts += curStroke->numVerts;

		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);

		//curStroke->removeFolds();

		if (showCPs)
		{
			glColor3f(0.5,0,0);
			glPointSize(3);
			glBegin(GL_POINTS);
			for (uint i=0; i<curStroke->CPs.size(); ++i)
			glVertex3f(curStroke->CPs[i].p.x, curStroke->CPs[i].p.y, 1.0);
			glEnd();
		}

		curStroke = strokes.get_next(it);
	}
	
			/*glColor3f(0,0.7,0);
			glPointSize(2);
			glBegin(GL_POINTS);
				glVertex3f(min.x, min.y, 1.0);
				glVertex3f(max.x, max.y, 1.0);
			glEnd();
	
			glColor3f(0.5,0,0);
			glPointSize(3);
			glBegin(GL_POINTS);
			for (uint i=0; i<tempPoints.size(); ++i)
				glVertex3f(tempPoints[i].p.x, tempPoints[i].p.y, 1.0);
			glEnd();
		*/
/*
	for (uint i=0; i<moveBuffer.size(); ++i)
	{
		glColor3f(0,0,0);
		glEnableClientState(GL_VERTEX_ARRAY);
		
		glVertexPointer(2, GL_FLOAT, 0, moveBuffer[i]->vertices);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, moveBuffer[i]->numVerts / 2);
		
		totalRenderVerts += moveBuffer[i]->numVerts;

		glDisableClientState(GL_VERTEX_ARRAY);

		if (showCPs)
		{
			glColor3f(0.5,0,0);
			glPointSize(3);
			glBegin(GL_POINTS);
			for (uint j=0; j<moveBuffer[i]->CPs.size(); ++j)
			glVertex3f(moveBuffer[i]->CPs[j].p.x, moveBuffer[i]->CPs[j].p.y, 1.0);
			glEnd();
		}
	}
*/

	//draw the new stroke buffer
	if (strokeBuffer != NULL && strokeBuffer->CPs.size() > 1)
	{		
		glColor3f(0,0,0);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);

		glVertexPointer(2, GL_FLOAT, 0, strokeBuffer->vertices);
		glColorPointer(4, GL_FLOAT, 0, strokeBuffer->colours);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, strokeBuffer->numVerts);

		totalRenderVerts += strokeBuffer->numVerts;
		
		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);

	}

	if (showWireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	//draw canvas border / quadtree debug info
	lineTree.draw();

	//draw page flips

	glBegin(GL_TRIANGLES);
	glColor3f(0.5, 0.5, 0.5);
	glVertex2f(0,50);
	glColor3f(0.8, 0.8, 0.8);
	glVertex2f(50,50);
	glColor3f(0.5, 0.5, 0.5);
	glVertex2f(50,0);
	
	glColor3f(0.5, 0.5, 0.5);
	glVertex2f(CANVAS_WIDTH,50);
	glColor3f(0.8, 0.8, 0.8);
	glVertex2f(CANVAS_WIDTH-50,50);
	glColor3f(0.5, 0.5, 0.5);
	glVertex2f(CANVAS_WIDTH-50,0);	
	glEnd();


	glPopMatrix();

	glPushMatrix();
	if (showMoveBrush)
		moveBrush.drawCanvas(zoom);
	else if (showDrawBrush)
		brush.drawCanvas(zoom);
	else if (showStrokeBrush)
		moveStroke.draw(zoom);
	else if (drawMode == 0)
		brush.drawOutline(zoom);
	else if (drawMode == 1)
		moveBrush.drawOutline(zoom);

	glPopMatrix();

	glPushMatrix();
	glTranslatef(screenWidth*0.5f, screenHeight*0.5f, 0);
	glScalef(zoom,zoom,1.0);
	glRotatef(canvasAngle, 0, 0, 1);
	glTranslatef(-screenWidth*0.5f+xTranslation, -screenHeight*0.5f+yTranslation, 0);
	selection.draw(zoom);
	glPopMatrix();

	if (selection.activeMulti)
		selection.drawLineSelector();

	if (selection.activeErase)
		selection.drawLineSelector();
}

void Canvas::setStrokeBuffer(Stroke& newStroke)
{
	if (strokeBuffer != NULL)
		strokeBuffer = NULL;

	strokeBuffer = new Stroke(newStroke);
}

inline void Canvas::setCanvasView()
{
	glTranslatef(screenWidth*0.5f, screenHeight*0.5f, 0);
	glScalef(zoom,zoom,1.0);
	glRotatef(canvasAngle, 0, 0, 1);
	glTranslatef(-screenWidth*0.5f+xTranslation, -screenHeight*0.5f+yTranslation, 0);
}

BoundingBox Canvas::calcViewBounds()
{
	//bounds of the viewing area
	Vector2 center = Vector2(screenWidth*0.5f, screenHeight*0.5f); //center of the view
	Vector2  trans = Vector2(xTranslation, yTranslation);
	Vector2 viewPos = center - trans;
	float viewWidth = screenWidth / zoom;
	float viewHeight = screenHeight / zoom;

	float viewDim = Vector2(viewWidth, viewHeight).length(); //diagonal

	Vector2 pmin = viewPos - Vector2(viewDim*0.5f, viewDim*0.5f);
	Vector2 pmax = viewPos + Vector2(viewDim*0.5f, viewDim*0.5f);

	BoundingBox viewBounds = BoundingBox(pmin, pmax); //approximate maximum visual area
	return viewBounds;
}

void Canvas::toCanvasSpace(float& x, float& y)
{
	float tilt = -canvasAngle*TO_RADIANS;
	//translate
	x -= screenWidth*0.5f; y -= screenHeight*0.5f;
	
	//take zoom in account
	x /= zoom; y /= zoom;
	
	//rotate
	float xx = x; float yy = y;
	x = xx*cos(tilt) - yy*sin(tilt);
	y = xx*sin(tilt) + yy*cos(tilt);

	//translate back
	x -= xTranslation; y -= yTranslation;
	x += screenWidth*0.5f; y += screenHeight*0.5f;
}

void Canvas::toScreenSpace(float& x, float& y, Vector2 center)
{
	float tilt = -canvasAngle*TO_RADIANS;
float xT = xTranslation; float yT = yTranslation;
	//translate
	x -= 900*0.5f; y -= 900*0.5f;
	//x -= screenWidth*0.5f; y -= screenHeight*0.5f;

//rotateToCanvasSpace(center.x, center.y);
//x -= center.x; y -= center.y;

	//take zoom in account
	x *= zoom; y *= zoom;
	
	//rotate
	float xx = x; float yy = y;
	x = xx*cos(tilt) - yy*sin(tilt);
	y = xx*sin(tilt) + yy*cos(tilt);

	//translate back
	//x += xTranslation; y += yTranslation;

	x += 900*0.5f; y += 900*0.5f;
	//x += screenWidth*0.5f; y += screenHeight*0.5f;
	//x += center.x; y += center.y;

	float ratio = screenHeight / screenWidth;
	//x /= ratio;



//rotateToCanvasSpace(xT, yT);

		float tilt2 = canvasAngle*TO_RADIANS;
		float xxx = xT; float yyy = yT;
		xT = xxx*cos(tilt2) - yyy*sin(tilt2);
		yT = xxx*sin(tilt2) + yyy*cos(tilt2);

xT *= zoom; yT *= zoom;
//x += xT; y += yT;
//x += xTranslation; y += yTranslation;

//	x -= 450.0f; y -= 450.0f;
	//x += xTranslation; y += yTranslation;
	//x -= center.x; y -= center.y;

	//float xx = x; float yy = y;
//	x = xx*cos(tilt) - yy*sin(tilt);
	//y = xx*sin(tilt) + yy*cos(tilt);

	//x *= zoom; y *= zoom;

//	x+= 450.0f; y += 450.0f;
	//x += screenWidth*0.5f; y += screenHeight*0.5f;
}

void Canvas::applyRotation(float rot)
{
	canvasAngle = fmod(canvasAngle - rot*TO_DEGREES, 360.0f); //keep angle in 360 range
}

void Canvas::applyMove(float dx, float dy)
{
	//scale movement speed
	float ddx = dx/zoom;
	float ddy = dy/zoom;
	//account for canvas rotation in movement
	float tilt = -canvasAngle*TO_RADIANS;
	xTranslation += ddx*cos(tilt) - ddy*sin(tilt);//dx;
	yTranslation += ddx*sin(tilt) + ddy*cos(tilt);//dy;
}

void Canvas::applyZoom(float dzoom)
{
	float zoomChange = dzoom*0.1f;
	if (zoom + zoomChange >= 0.1f)
		zoom += zoomChange;
	else 
		zoom = 0.1f;

	if (zoom > 10.0f)	zoom = 10.0f;
}

void Canvas::finalizeZoom()
{
	BoundingBox viewBounds = calcViewBounds();

	//create more detailed geometry if and where needed
	ListNode* it = strokes.front;
	Stroke* curStroke = strokes.get_front();
	for(uint i=0; i<strokes.size(); ++i)
	{
		if (!viewBounds.overlaps(curStroke->bounds))
		{
			curStroke = strokes.get_next(it);
			continue;
		}

		//create more detailed geometry if and where needed
		curStroke->increaseRefinement(zoom);
		//redraw visible geometry with new zoom value
		curStroke->genVertexArray(zoom);

		curStroke = strokes.get_next(it);
	}
}

void Canvas::setMoveStroke()
{
	vector<GLfloat> verts;
	vector<GLfloat> color;
	selectedStrokes[0]->genExportArrays(zoom, verts, color);

	moveStroke = MoveStroke(verts);
}

void Canvas::drawMoveBrush()
{
	moveBrush.drawCanvas(zoom);
}

void Canvas::clearCanvas()
{
	ListNode* it = strokes.front;
	Stroke* curStroke = strokes.get_front();

	for(uint i=0; i<strokes.size(); ++i)
	{
		curStroke->removeFromQuadTree(lineTree);
		curStroke = strokes.get_next(it);
	}
	strokes.clear();
}

void Canvas::resetView(unsigned long time)
{
	prevAnimTime = time;
	totalAnimTime = 0;
	//canvasAngle = 0.0f;
	//zoom = 1.0f;
	animZoomStep = (zoom - 1.0f) / (float)resetTime;
	animRotStep = canvasAngle / (float)resetTime;
}

void Canvas::animateReset(unsigned long time)
{
	if (totalAnimTime < resetTime) //still not done with canvas reset animation
	{
		//animate motion
		int diff = time-prevAnimTime;
		prevAnimTime = time;
		canvasAngle -= animRotStep*diff;
		zoom -= animZoomStep*diff;
		totalAnimTime += diff;
	}
}

void Canvas::exportDrawing(std::string filename)
{
	TiXmlDocument doc;
	TiXmlElement* el;
	
	TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "", "" );  
	doc.LinkEndChild(decl);

	TiXmlElement* root = new TiXmlElement("svg");  
	doc.LinkEndChild(root);
	root->SetAttribute("width", "100%");
	root->SetAttribute("height", "100%");
	root->SetAttribute("version", "1.1");
	root->SetAttribute("xmlns", "http://www.w3.org/2000/svg");

	//add all strokes to the xml tree
	ListNode* it = strokes.front;
	Stroke* curStroke = strokes.get_front();
	for(uint i=0; i<strokes.size(); ++i)
	{
		vector<GLfloat> verts;
		vector<GLfloat> color;

		//grab stroke rib information
		curStroke->genExportArrays(zoom, verts, color);

		std::ostringstream conv;

		for (int j=0; j<color.size(); ++j)
		{
			el = new TiXmlElement("polygon");
			root->LinkEndChild(el);

			conv.str("");//clear
			conv << verts[j*4]; conv << ","; conv << verts[j*4+1]; //x0,y0
			conv << " "; conv << verts[j*4+2]; conv << ","; conv << verts[j*4+3]; // x1,y1
			conv << " "; conv << verts[j*4+6]; conv << ","; conv << verts[j*4+7]; // x3,y3
			conv << " "; conv << verts[j*4+4]; conv << ","; conv << verts[j*4+5]; // x2,y2
			
			/*float x0 = (verts[j*4]+verts[j*4+2])*0.5f; //x0
			float y0 = (verts[j*4+1]+verts[j*4+3])*0.5f; //y0

			float x1 = (verts[j*4+4]+verts[j*4+6])*0.5f; //x1
			float y1 = (verts[j*4+5]+verts[j*4+7])*0.5f; //y1
*/
			std::string polyPoints = "";
			polyPoints.append(conv.str());

			conv.str("");
			int rgb = 220.0-255.0*color[j];
			if (rgb < 0) rgb = 0;

			//conv << "fill:rgb("; conv << rgb; conv << ","; conv << rgb; conv << ","; conv << rgb; conv << ");";

			std::string polyColor = "";
			polyColor.append(conv.str());

		  el->SetAttribute("points", polyPoints.c_str());
			//el->SetAttribute("style", polyColor.c_str());

			el->SetAttribute("style", "fill:rgb(0,0,0);");
			el->SetDoubleAttribute("opacity", color[j]);
			/*el->SetDoubleAttribute("x1", x0);
			el->SetDoubleAttribute("y1", y0);
			el->SetDoubleAttribute("x2", x1);
			el->SetDoubleAttribute("y2", y1);
			el->SetAttribute("style", polyColor.c_str());*/

		}

		curStroke = strokes.get_next(it);
	}

	doc.SaveFile(filename.c_str());

}

void Canvas::saveDrawing(std::string filename)
{
	TiXmlDocument doc;
	TiXmlElement* el;
	
	TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "", "" );  
	doc.LinkEndChild(decl);

	TiXmlElement* root = new TiXmlElement("Drawing");  
	doc.LinkEndChild(root);

	//add all strokes to the xml tree
	ListNode* it = strokes.front;
	Stroke* curStroke = strokes.get_front();
	for(uint i=0; i<strokes.size(); ++i)
	{
		TiXmlElement* stroke = new TiXmlElement( "Stroke" );  
		root->LinkEndChild(stroke);  
		
		for (int j=0; j<curStroke->CPs.size(); ++j)
		{
			el = new TiXmlElement("CP");
			stroke->LinkEndChild(el);

			el->SetDoubleAttribute("x", curStroke->CPs[j].p.x);
			el->SetDoubleAttribute("y", curStroke->CPs[j].p.y);
			el->SetDoubleAttribute("r", curStroke->CPs[j].r);
			el->SetDoubleAttribute("a", curStroke->CPs[j].a);
		}

		curStroke = strokes.get_next(it);
	}

	doc.SaveFile(filename.c_str());
}

void Canvas::loadDrawing(std::string filename)
{
	//cout<<"file: "<<filename<<"\n";
	TiXmlDocument doc(filename.c_str());
	bool loaded = doc.LoadFile();
	if (loaded)
	{
		clearCanvas(); //remove existing strokes

		TiXmlHandle hDoc(&doc);
		TiXmlElement* el;
		TiXmlHandle root(0);

		el = hDoc.FirstChildElement().Element();

		if (!el) return;

		root = TiXmlHandle(el);

		//read in strokes
		TiXmlElement* stroke = root.FirstChild().Element();
		for (stroke; stroke; stroke = stroke->NextSiblingElement()) //each stroke
		{
			Stroke* newStroke = new Stroke();

			TiXmlElement* strkCP = stroke->FirstChildElement();
			for (strkCP; strkCP; strkCP = strkCP->NextSiblingElement())
			{
				float x, y, r, a;
				x = y = r = a = 0.0f;
				strkCP->QueryFloatAttribute("x", &x);
				strkCP->QueryFloatAttribute("y", &y);
				strkCP->QueryFloatAttribute("r", &r);
				strkCP->QueryFloatAttribute("a", &a);
				newStroke->buildControlPoint(x, y, r, a);
			}
			newStroke->initSegments();
			newStroke->genMergeTrees();
			newStroke->genVertexArray();
			strokes.push_back(newStroke);
			lineTree.addStroke(newStroke); //add stroke to the quadtree
		}		
	}
	else
	{
		clearCanvas();
		cout<<"Failed to load file\n";
	}
}

//LINE DRAWING ROUTINES

void Canvas::startNewLine(float x, float y, float pressure)
{
	brush.setBrushPos(x,y);
	toCanvasSpace(x, y);
	
	showDrawBrush = true;

	//online allow drawing inside the canvas	
	if (!lineTree.root.bounds.inside(Vector2(x, y)))
		return;
	
	//start a new line, get initial sample
	float sampleAlpha = 1.0f;
	float sampleC[3];
	float sampleSize = brush.sampleBrush(x, y, 0.0f, 0.0f, pressure, sampleAlpha, sampleC);
	
	Stroke stroke = Stroke(x, y, sampleSize, sampleAlpha);
	setStrokeBuffer(stroke);
	//stroke.genVertexArray();
	strokeBuffer->genVertexArray(zoom);
}

void Canvas::extendNewLine(float x, float y, float dx, float dy, float pressure)
{
	if (strokeBuffer == NULL)
		return;

	brush.setPosition(x, y);

	//check if there is a new line to extend
	moveDist += sqrt(dx*dx + dy*dy);

	if (moveDist > minSampleRate) //check if we should sample the brush
	{
		toCanvasSpace(x, y);
		
		//online allow drawing inside the canvas
		if (!lineTree.root.bounds.inside(Vector2(x, y)))
			return;		
		
		float sampleAlpha = 1.0f;
		float sampleC[3];
		float sampleSize = brush.sampleBrush(x, y, dx, dy, pressure, sampleAlpha, sampleC); //sample brush for new data point
		moveDist = 0.0f; //reset total distance moved

		strokeBuffer->extend(x, y, sampleSize, sampleAlpha);

		//check if the stroke can be resampled
		if ((int)strokeBuffer->CPs.size()-1 - strokeBuffOffset > WINDOW_SIZE)
		{
			strokeBuffer->resample(strokeBuffOffset, strokeBuffer->CPs.size()-1, zoom, maxStrokeError);
			
			int numRemainingSamples = strokeBuffer->CPs.size()-1 - strokeBuffOffset;
			strokeBuffOffset += numRemainingSamples - WINDOW_OVERLAP;
			if (strokeBuffOffset < 1)
				strokeBuffOffset = 1;
		}
		
		strokeBuffer->genVertexArray(zoom);
	}
}

void Canvas::endNewLine(float x, float y)
{
	showDrawBrush = false;

	//check if there's anything to end
	if (strokeBuffer != NULL)
	{
		if (strokeBuffer->CPs.size() > 1) //don't add stroke when the canvas was tapped
		{
			//add stroke to the canvas
			if ((int)strokeBuffer->CPs.size() - strokeBuffOffset > 2)
			{
				//resample last part
				strokeBuffer->resample(strokeBuffOffset, strokeBuffer->CPs.size()-1, zoom, maxStrokeError);
				strokeBuffer->finalize();
				strokeBuffOffset = 1; //reset
				
				strokeBuffer->genVertexArray(zoom);
				//std::cout<<"strokes size: "<<strokes.size()<<"\n";
			}
			//add stroke to the list
			strokes.push_back(strokeBuffer);
			//add stroke to the quadtree
			lineTree.addStroke(strokeBuffer);
			strokeBuffer = NULL; //'empty' strokeBuffer
		}
		else
		{
			delete strokeBuffer;
			strokeBuffer = NULL;
		}
	}
}

// MOVE BRUSH ROUTINES

void Canvas::startMoveBrush(float x, float y, float pressure)
{
	float cx = x; float cy = y;
	toCanvasSpace(cx, cy);
	moveBrush.setPosition(cx, cy, x, y);
	showMoveBrush = true;
}

void Canvas::dragMoveObject(float x, float y, float dx, float dy, float pressure)
{
	if (dx == 0 && dy == 0) //fixes problem with wacom
		return;

	float cx = x; float cy = y;
	toCanvasSpace(cx, cy);
	rotateToCanvasSpace(dx, dy);
	BoundingBox b;

	if (showMoveBrush)
	{
		moveBrush.setPosition(cx, cy, x, y);
		
		b	= moveBrush.bounds;
		b.move(moveBrush.pos);
	}
	else if (showStrokeBrush)
	{
		moveStroke.setPosition(cx, cy, x, y);
		moveStroke.setDirection(dx, dy);

		b = moveStroke.bounds;
		b.move(moveStroke.pos);
	}
	else
		return;

	//cout<<"pMin "<<b.pMin.x<<","<<b.pMin.y<<" "<<b.pMax.x<<","<<b.pMax.y<<"\n";

	//determine segments/strokes affected by the brush
	vector<StrokeSegment*> affected = lineTree.findInRange(b);

	//find intersected strokes
	for (uint i=0; i<affected.size(); ++i)
	{
		uint j;
		for (j=0; j<moveBuffer.size(); ++j) //find stroke the segment belongs to
			if (affected[i]->stroke == moveBuffer[j]) //found it, we were already editing this stroke
				break;
		
		if (j == moveBuffer.size()) //found a new stroke to edit
		{
			if (affected[i]->stroke->CPs.size() < 3)
				continue;
			
			moveBuffer.push_back(affected[i]->stroke);
			moveBuffer.back()->removeFromQuadTree(lineTree);
		}
	}

	//handle each stroke in the buffer
	for (uint i=0; i<moveBuffer.size(); ++i)
	{
		//find segment along stroke
		int minCP = 0;
		int maxCP = moveBuffer[i]->CPs.size()-1;
	
		if (!moveBuffer[i]->findOverlappingSegment(b, minCP, maxCP))
			continue; //no part of the line is affected

//cout<<"dragging\n";

		//resample segment
		vector<ControlPoint> points;
		for (uint j=minCP; j<maxCP; ++j) 
			moveBuffer[i]->calcSegmentForwardDiff(j, 5.0f, points); //get sampled control points

		//move points with brush
		if (showMoveBrush)
			moveBrush.pushPoints(dx, dy, points);
		else if (showStrokeBrush)
			moveStroke.pushPoints(dx, dy, points);

		if (minCP == 0)
		{
			vector<ControlPoint> first;
			first.push_back(moveBuffer[i]->CPs[0]);
			moveBrush.pushPoints(dx, dy, first);
			moveBuffer[i]->CPs[0].p = first[0].p;
		}
		if (maxCP == moveBuffer[i]->CPs.size()-1)
		{
			vector<ControlPoint> last;
			last.push_back(moveBuffer[i]->CPs.back());
			moveBrush.pushPoints(dx, dy, last);
			moveBuffer[i]->CPs.back().p = last[0].p;
		}

		moveBuffer[i]->replace(minCP, maxCP, points);
		moveBuffer[i]->genMergeTrees(zoom);
		moveBuffer[i]->genVertexArray(zoom);
	}

}

void Canvas::endMoveBrush()
{
	//handle each stroke in the buffer
	for (uint i=0; i<moveBuffer.size(); ++i)
	{
		//resample stroke
		moveBuffer[i]->resample(1, moveBuffer[i]->CPs.size()-1, zoom, maxStrokeError);
		moveBuffer[i]->genVertexArray(zoom);
	
		//add stroke to the quadtree
		lineTree.addStroke(moveBuffer[i]);
	}
	moveBuffer.clear();
	showMoveBrush = false;
}

// MOVE STROKE ROUTINES
void Canvas::startMoveStroke(float x, float y, float pressure)
{
	float cx = x; float cy = y;
	toCanvasSpace(cx, cy);
	moveStroke.setPosition(cx, cy, x, y);
	showStrokeBrush = true;
}

void Canvas::endMoveStroke()
{
	//handle each stroke in the buffer
	for (uint i=0; i<moveBuffer.size(); ++i)
	{
		//resample stroke
		moveBuffer[i]->resample(1, moveBuffer[i]->CPs.size()-1, zoom, maxStrokeError+3.0f); //get rid of jitter by doing more smoothing
		moveBuffer[i]->genVertexArray(zoom);
	
		//add stroke to the quadtree
		lineTree.addStroke(moveBuffer[i]);
	}

	moveBuffer.clear();
	showStrokeBrush = false;
}

// HANDLE ERASING

void Canvas::eraseFromStroke(float x, float y, float pressure)
{

//cout<<"erasing...\n";

	//get brush sample
	float sampleAlpha = 1.0f;
	float sampleC[3];
	float sample = brush.sampleBrush(x, y, 0.0f, 0.0f, pressure, sampleAlpha, sampleC);
	toCanvasSpace(x, y);

	//calculate bounds of the sample
	BoundingBox bounds= BoundingBox(Vector2(x,y));
	bounds.expand(sample);

	//range-search quadtree for affected strokes
	vector<Stroke*> editBuffer;
	lineTree.findIntersectingStrokes(bounds, editBuffer);

	for (uint i=0; i<editBuffer.size(); ++i) //handle each stroke
	{
		editBuffer[i]->removeFromQuadTree(lineTree);

		if (editBuffer[i]->CPs.size() < 3)
		{
			removeStroke(editBuffer[i]);
			continue;
		}

		//find segment along stroke
		int minCP = 0;
		int maxCP = editBuffer[i]->CPs.size()-1;

		if (!editBuffer[i]->findOverlappingSegment(bounds, minCP, maxCP))
			continue; //no part of the line is affected

		vector<ControlPoint> points;
		vector<ControlPoint> remaining;
		//points.push_back(editBuffer[i]->CPs[minCP]);

		//resample affected area
			for (uint j=minCP; j<maxCP; ++j) 
				editBuffer[i]->calcSegmentForwardDiff(j, 5.0f, points); //get sampled control points
	
		//check which points are erased
		int first = -1;
		int second = points.size()-1;
		
		if (bounds.overlaps(editBuffer[i]->CPs[minCP].p, points[0].p))
			first = 0;

		for (uint j=0; j<points.size()-1; ++j) //points[0] can fall before the previous segment
		{
			//Vector2 dist = Vector2(x-points[j].p.x, y-points[j].p.y);
			if (first < 0)
			{
				if (!bounds.overlaps(points[j].p, points[j+1].p))
					remaining.push_back(points[j]);
				else
				{
					first = j; //found the end of the first half
					second = j+1;
				}
			}
			else
			{
				if (bounds.overlaps(points[j].p, points[j+1].p))
					second = j;
				else
					remaining.push_back(points[j]);
			}
		}

		if (first < 0)
		{
			lineTree.addStroke(editBuffer[i]);
			continue;
		}

		tempPoints = remaining;
		//if either end is erased, redo stroke tips
		if ( (minCP == 0 && first == 0) || (maxCP == editBuffer[i]->CPs.size()-1 && second == points.size()-1) ) //tips
		{
			editBuffer[i]->replace(minCP, maxCP, remaining);
			
			if (minCP == 0)
				editBuffer[i]->popFirstCP();
			else
				editBuffer[i]->popLastCP();
	
			if (editBuffer[i]->CPs.size() > 2)
			{
				editBuffer[i]->resample(1, editBuffer[i]->CPs.size()-1, zoom, maxStrokeError);
				//editBuffer[i]->genMergeTrees(zoom);
				lineTree.addStroke(editBuffer[i]);
				editBuffer[i]->genVertexArray(zoom);
			}
			else
			{
				removeStroke(editBuffer[i]);
			}
		}
		else //otherwise split stroke
		{
			editBuffer[i]->replace(minCP, maxCP, remaining);

			//divide the CPs and segments between both new strokes
			vector<ControlPoint> firstCPs(editBuffer[i]->CPs.begin(), editBuffer[i]->CPs.begin()+minCP+first);
			vector<MergeTree> firstSegs(editBuffer[i]->segments.begin(), editBuffer[i]->segments.begin()+minCP+first-1);

			Stroke* secondHalf = new Stroke();
			vector<ControlPoint> secondCPs(editBuffer[i]->CPs.begin()+minCP+first+1, editBuffer[i]->CPs.end());
			vector<MergeTree> secondSegs(editBuffer[i]->segments.begin()+minCP+first+1, editBuffer[i]->segments.end());

			if (firstCPs.size() > 2)
			{
				editBuffer[i]->CPs = firstCPs;
				editBuffer[i]->segments = firstSegs;
				editBuffer[i]->resample(1, editBuffer[i]->CPs.size()-1, zoom, maxStrokeError);
				//editBuffer[i]->genMergeTrees(zoom);
				lineTree.addStroke(editBuffer[i]);
				editBuffer[i]->genVertexArray(zoom);
				editBuffer[i]->calcBounds();
			}
			else
				removeStroke(editBuffer[i]);
			
			if (secondCPs.size() > 2)
			{
				secondHalf->CPs = secondCPs;
				secondHalf->segments = secondSegs;

				strokes.push_back(secondHalf); //add new stroke
				secondHalf->resample(1, secondHalf->CPs.size()-1, zoom, maxStrokeError);
				//secondHalf->genMergeTrees(zoom);
				lineTree.addStroke(secondHalf);
				secondHalf->genVertexArray(zoom);
				secondHalf->calcBounds();
			}
		}
	} //editBuffer[i]
	//cout<<"done ersasing\n";
}

void Canvas::removeStroke(Stroke* id)
{
	ListNode* it = strokes.front;
	Stroke* curStroke = strokes.get_front();
	for(uint i=0; i<strokes.size(); ++i)
	{
		if (curStroke == id)
		{
			strokes.remove(it);
			delete curStroke;
			break;
		}
		curStroke = strokes.get_next(it);
	}
}

bool Canvas::selectStroke(float x, float y)
{
	selectedStrokes.clear();
	toCanvasSpace(x,y);

	//check if there are lines closeby
	Vector2 p = Vector2(x,y);
	BoundingBox bounds = BoundingBox(p);
	bounds.expand(5.0f); //max distance for a stroke to still be selected

	//find closest
	float minDist = 99999.0f;
	Stroke* closest = NULL;

	vector<StrokeSegment*> affected = lineTree.findInRange(bounds);

	for (uint i=0; i<affected.size(); ++i)
	{
		//find minimal distance between segment and select point
		float x0 = affected[i]->p0.x;
		float y0 = affected[i]->p0.y;
		float x1 = affected[i]->p1.x;
		float y1 = affected[i]->p1.y;

		Vector2 p1p0 = affected[i]->p1 - affected[i]->p0;
		float u = ( (x-x0)*(x1-x0) + (y-y0)*(y1-y0) ) / p1p0.lengthSquared();

		if (u > 1.0f || u < 0.0f) //closest point does not fall inside the segment
			continue; //skip to next segment

		//compute intersect point between p and p1p0
		float intX = x0 + u*(x1-x0);
		float intY = y0 + u*(y1-y0);
		Vector2 intP = Vector2(intX, intY);

		Vector2 dist = intP - p;
		float d = dist.lengthSquared(); //compare squared lengths for efficiency
		if (d < minDist)
		{
			minDist = d;
			closest = affected[i]->stroke;
		}
	}
	//cout<<"closest: "<<closest<<"\n";
	if (closest == NULL)
		return false;
	
	selectedStrokes.push_back(closest);
	selection.select(closest); //selection widgets

	return true;
}

void Canvas::removeSelection()
{
	for (uint i=0; i<selectedStrokes.size(); ++i)
	{
		//cout<<"removing : "<<selectedStrokes[i]<<"\n";
		selectedStrokes[i]->removeFromQuadTree(lineTree);
		removeStroke(selectedStrokes[i]);
	}
	selectedStrokes.clear();
}

void Canvas::applyScaleSelection(float x, float y, float dx, float dy, int dragWidget)
{
	int anchored = -1; ////set anchored widget in regards to which scaling will be done
	//int dragWidget = 0;

	Vector2 center = (selection.bounds.pMin + selection.bounds.pMax)*0.5f;

	anchored = (dragWidget+2)%4;

	dx /= zoom;
	dy /= zoom;
	rotateToCanvasSpace(dx, dy);

	//get anchor position
	Vector2 anchor = selection.getWidgetPos(anchored);
	//get widget position
	Vector2 dragPos = selection.getWidgetPos(dragWidget);
	Vector2 mousePos = dragPos + Vector2(dx, dy);

//	float scale = (mousePos - anchor).dot(widgetPos - anchor) / (widgetPos - anchor).dot(widgetPos - anchor);
//	float scaleX = scale;
//	float scaleY = 1;

	Vector2 Pg = selection.getWidgetPos((anchored+3)%4); //first adjacent point
	Vector2 Ph = selection.getWidgetPos((anchored+1)%4); //second adjacent point

	//get length of the adjacent edges
	Vector2 g = Pg - anchor;
	Vector2 h = Ph - anchor;

	//get the length of the pull vector
	Vector2 f = mousePos - anchor;

	//get the length of the new adjacent edges
	Vector2 gProj = g*(f.dot(g) / g.lengthSquared());
	Vector2 hProj = h*(f.dot(h) / h.lengthSquared());
	
	float gProjLen = gProj.length();
	float hProjLen = hProj.length();

	if (gProjLen < 5.0f || hProjLen < 5.0f)
		return;

	float scaleU = gProjLen / g.length();
	float scaleV = hProjLen / h.length();

	Vector2 origin = selection.corner[anchored];

	//find u (an adjecent edge to the anchor)
	Vector2 edgeU = selection.corner[(anchored+3)%4] - origin;
	Vector2 u = edgeU / edgeU.length();
	//v is perpendicular to u
	Vector2 v = Vector2(-u.y, u.x);

	for (uint i=0; i<selectedStrokes.size(); ++i)
	{
		selectedStrokes[i]->removeFromQuadTree(lineTree);

		//scale each CP relative to the anchor point
		for (uint j=0; j<selectedStrokes[i]->CPs.size(); ++j)
		{
			Vector2& p = selectedStrokes[i]->CPs[j].p;

			p -= origin;
			float pu = p.dot(u);
			float pv = p.dot(v);

			pu *= scaleU;
			pv *= scaleV;

			p = u*pu + v*pv + origin;
		}
		selectedStrokes[i]->calcBounds();

		lineTree.addStroke(selectedStrokes[i]);
		selectedStrokes[i]->genMergeTrees(zoom);
		selectedStrokes[i]->genVertexArray();
	}

	selection.scale(scaleU, scaleV, anchored);

}

void Canvas::applyMoveSelection(float dx, float dy)
{
	dx /= zoom;
	dy /= zoom;
	rotateToCanvasSpace(dx, dy);

	//cout<<"moving: "<<dx<<","<<dy<<"\n";

	for (uint i=0; i<selectedStrokes.size(); ++i)
	{
		selectedStrokes[i]->removeFromQuadTree(lineTree);

		//move the location of each CP by dx,dy
		for (uint j=0; j<selectedStrokes[i]->CPs.size(); ++j)
		{
			//cout<<"before "<<selectedStrokes[i]->CPs[j].p.x<<","<<selectedStrokes[i]->CPs[j].p.y;
			selectedStrokes[i]->CPs[j].p += Vector2(dx,dy);
		//cout<<" after "<<selectedStrokes[i]->CPs[j].p.x<<","<<selectedStrokes[i]->CPs[j].p.y<<"\n";

		}

		lineTree.addStroke(selectedStrokes[i]);
		selectedStrokes[i]->genMergeTrees(zoom);
		selectedStrokes[i]->genVertexArray();
	}

	selection.move(Vector2(dx, dy));
}

void Canvas::applyRotateSelection(float angle)
{
	Vector2 center = (selection.bounds.pMin + selection.bounds.pMax)*0.5f;

	for (uint i=0; i<selectedStrokes.size(); ++i)
	{
		selectedStrokes[i]->removeFromQuadTree(lineTree);

		for (uint j=0; j<selectedStrokes[i]->CPs.size(); ++j)
		{
			selectedStrokes[i]->CPs[j].p -= center;
			selectedStrokes[i]->CPs[j].p.rotateAroundZero(angle);
			selectedStrokes[i]->CPs[j].p += center;
		}

		lineTree.addStroke(selectedStrokes[i]);
		selectedStrokes[i]->genMergeTrees(zoom);
		selectedStrokes[i]->genVertexArray();
	}

	//rotate bounds to match
	selection.rotate(angle);
}

void Canvas::finalizeSelectionAction()
{
	if (selectedStrokes.size() == 0)
		return;

	//cout<<"finalize\n";

	selectedStrokes[0]->calcBounds();
	selection.bounds = selectedStrokes[0]->bounds;

	for (uint i=1; i<selectedStrokes.size(); ++i)
	{
		selectedStrokes[i]->calcBounds();
		selection.bounds.merge(selectedStrokes[i]->bounds.pMax);
		selection.bounds.merge(selectedStrokes[i]->bounds.pMin);
	}

	//selection.updateWidgetPositions();
}

void Canvas::startMultiSelect(float x, float y)
{
	selection.startMultiSelect(x, y);
}

void Canvas::extendMultiSelect(float x, float y)
{
	selection.updateMultiSelect(x, y);
}

void Canvas::endMultiSelect(float x, float y)
{
/*	Vector2 selStart = selection.multiStart;
	Vector2 selEnd = selection.multiEnd;
	selection.endMultiSelect();

	//grab strokes in the selection range
	selectedStrokes.clear();

	toCanvasSpace(selStart.x, selStart.y);
	toCanvasSpace(selEnd.x, selEnd.y);

	BoundingBox bounds = BoundingBox(selStart, selEnd);
	vector<StrokeSegment*> affected = lineTree.findInRange(bounds);

	for (uint i=0; i<affected.size(); ++i)
	{
		uint j;
		for (j=0; j<selectedStrokes.size(); ++j) //find stroke the segment belongs to
			if (affected[i]->stroke == selectedStrokes[j]) //found it, we were already editing this stroke
				break;
		
		if (j == selectedStrokes.size()) //found a new stroke to select
			//cout<<"multi selected: "<<affected[i]->stroke<<"\n";
			selectedStrokes.push_back(affected[i]->stroke);
	}*/

	Vector2 p0 = selection.selectLine.front();
	Vector2 p1 = selection.selectLine.back();

	toCanvasSpace(p0.x, p0.y);
	toCanvasSpace(p1.x, p1.y);

	BoundingBox b = BoundingBox(p0);
	for (uint k=1; k<selection.selectLine.size(); ++k)
	{
		Vector2 p = selection.selectLine[k];
		toCanvasSpace(p.x, p.y);
		b.merge(p);
	}
	
	selectedStrokes.clear();

	vector<StrokeSegment*> affected = lineTree.findInRange(b);

	for (uint i=0; i<affected.size(); ++i)
	{
		
		//check if the segment intersects one of the erase line segments
		Vector2 a0 = affected[i]->p0;
		Vector2 a1 = affected[i]->p1;

		for (uint n=0; n<selection.selectLine.size()-1; ++n)
		{
			Vector2 q0 = selection.selectLine[n];
			Vector2 q1 = selection.selectLine[n+1];

			toCanvasSpace(q0.x, q0.y);
			toCanvasSpace(q1.x, q1.y);

			BoundingBox segBound = BoundingBox(q0, q1);

			if (segBound.overlaps(a0, a1))
			{
				uint j;
				for (j=0; j<selectedStrokes.size(); ++j) //try to find stroke the segment belongs to
					if (affected[i]->stroke == selectedStrokes[j]) //found it, we were already going to erase this stroke
						break;
					
				if (j == selectedStrokes.size()) //found a new stroke to erase
					selectedStrokes.push_back(affected[i]->stroke);

			}
		}

	}
	
	selection.endMultiSelect();
	selection.multiSelect(selectedStrokes); //selection widgets
}

void Canvas::startMultiErase(float x, float y)
{
	selection.startMultiErase(x, y);
}

void Canvas::extendMultiErase(float x, float y)
{
	selection.updateMultiErase(x, y);
}

void Canvas::endMultiErase(float x, float y)
{
	Vector2 p0 = selection.selectLine.front();
	Vector2 p1 = selection.selectLine.back();

	toCanvasSpace(p0.x, p0.y);
	toCanvasSpace(p1.x, p1.y);

	BoundingBox b = BoundingBox(p0);
	for (uint k=1; k<selection.selectLine.size(); ++k)
	{
		Vector2 p = selection.selectLine[k];
		toCanvasSpace(p.x, p.y);
		b.merge(p);
	}
	
	vector<Stroke*> toDelete;

	vector<StrokeSegment*> affected = lineTree.findInRange(b);

	for (uint i=0; i<affected.size(); ++i)
	{
		
		//check if the segment intersects one of the erase line segments
		Vector2 a0 = affected[i]->p0;
		Vector2 a1 = affected[i]->p1;

		for (uint n=0; n<selection.selectLine.size()-1; ++n)
		{
			Vector2 q0 = selection.selectLine[n];
			Vector2 q1 = selection.selectLine[n+1];

			toCanvasSpace(q0.x, q0.y);
			toCanvasSpace(q1.x, q1.y);

			BoundingBox segBound = BoundingBox(q0, q1);

			//float nom = (q1.y-q0.y)*(a1.x-a0.x) - (q1.x-q0.x)*(a1.y-a0.y);
			//if (nom != 0.0f)
			//{
			//	float t = ( (q1.x-q0.x)*(a0.y-q0.y) - (q1.y-q0.y)*(a0.x-q0.x) ) / nom;
				//if (t >= 0.0f && t <= 1.0f) //intersection is within range
				if (segBound.overlaps(a0, a1))
				{
					//cout<<"hit: "<<q0.x<<","<<q0.y<<" "<<q1.x<<","<<q1.y<<" with "<<a0.x<<","<<a0.y<<" "<<a1.x<<","<<a1.y<<"\n";
					uint j;
					for (j=0; j<toDelete.size(); ++j) //try to find stroke the segment belongs to
						if (affected[i]->stroke == toDelete[j]) //found it, we were already going to erase this stroke
							break;
					
					if (j == toDelete.size()) //found a new stroke to erase
						toDelete.push_back(affected[i]->stroke);

				}

			//}
		}

	}

	//erase strokes
	for (uint i=0; i<toDelete.size(); ++i)
	{
		toDelete[i]->removeFromQuadTree(lineTree);
		removeStroke(toDelete[i]);
	}
	
	selection.endMultiErase();
}

int Canvas::activeWidget(float x, float y)
{
	toCanvasSpace(x, y);
	Vector2 q = Vector2(x, y);

	//check if one of the the page flips were clicked	
	Vector2 p0[2]; p0[0] = Vector2(0,50);  p0[1] = Vector2(CANVAS_WIDTH,50);
	Vector2 p1[2]; p1[0] = Vector2(50,50); p1[1] = Vector2(CANVAS_WIDTH-50,50);
	Vector2 p2[2]; p2[0] = Vector2(50,0);	 p2[1] = Vector2(CANVAS_WIDTH-50,0);
	
	for (int i=0; i<2; ++i)
	{
		float b0 = (p1[i].x-p0[i].x)*(p2[i].y-p0[i].y) - (p2[i].x-p0[i].x)*(p1[i].y-p0[i].y);
		float b0Inv = 1.0f / b0;

		float b1 = ( (p1[i].x-q.x)*(p2[i].y-q.y) - (p2[i].x-q.x)*(p1[i].y-q.y) ) * b0Inv;
		float b2 = ( (p2[i].x-q.x)*(p0[i].y-q.y) - (p0[i].x-q.x)*(p2[i].y-q.y) ) * b0Inv;
		float b3 = ( (p0[i].x-q.x)*(p1[i].y-q.y) - (p1[i].x-q.x)*(p0[i].y-q.y) ) * b0Inv;

		if (!(b1 < 0.0f || b2 < 0.0f || b3 <0.0f))
			return i+1;
	}

	return 0;
}