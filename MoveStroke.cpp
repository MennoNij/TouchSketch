#include "MoveStroke.h"

#include <windows.h>
#include <GL/gl.h>

MoveStroke::MoveStroke()
{

}

MoveStroke::MoveStroke(Stroke* stroke)
{
//	for (uint i=0; i<stroke->CPs.size(); ++i)
//		p.push_back(stroke->CPs[i].p);
	//grab stroke's vertex information
/*	for (uint i=20; i<stroke->numVerts*2-20; i+=4) //ignore the stroke tips
	{
		Vector2 p0 = Vector2(stroke->vertices[i],   stroke->vertices[i+1]);
		Vector2 p1 = Vector2(stroke->vertices[i+2], stroke->vertices[i+3]);
		p.push_back((p1+p0)*0.5f);
		r.push_back( (p1-p0).length()*0.5f );
	}
*/
	//iterate over all the vertices
	for (uint i=0; i<stroke->numVerts*2; i+=2)
	{
		//add points to movestroke
		p.push_back(Vector2(stroke->vertices[i], stroke->vertices[i+1]));
	}


	//get bounds
	for (uint i=0; i<p.size(); ++i)
		bounds.merge(p[i]);
	
	//get center
	Vector2 c = (bounds.pMax+bounds.pMin)*0.5f;
	
	//translate all the points around the center
	for (uint i=0; i<p.size(); ++i)
		p[i] -= c;


	bounds.pMin -= c;
	bounds.pMax -= c;
	//std::cout<<"pMin "<<bounds.pMin.x<<","<<bounds.pMin.y<<" "<<bounds.pMax.x<<","<<bounds.pMax.y<<"\n";

	for (int i=0; i<NUMMOVEDIR; i++)
		dir[i] = Vector2(0,1);

	vertices = NULL;

}

MoveStroke::MoveStroke(vector<GLfloat>& stroke)
{
	for (int i=0; i<stroke.size(); i+=2)
	{
		p.push_back(Vector2(stroke[i], stroke[i+1]));
	}

	for (uint i=0; i<p.size(); i+=2)
	{
		Vector2 r0 = p[i];
		Vector2 r1 = p[i+1];

		Vector2 dist = r0-r1;

		if (dist.length() < 20.0f)
		{
			//stroke is too thing at this part, so fatten it
			Vector2 center = (r0+r1)*0.5f;
			Vector2 dir = dist.normalized();
			Vector2 mov = dir*10.0f;

			p[i] = center+mov;
			p[i+1] = center-mov;
		}
	}

	//get bounds
	for (uint i=0; i<p.size(); ++i)
		bounds.merge(p[i]);
	
	//get center
	Vector2 c = (bounds.pMax+bounds.pMin)*0.5f;
	
	//translate all the points around the center
	for (uint i=0; i<p.size(); ++i)
		p[i] -= c;


	bounds.pMin -= c;
	bounds.pMax -= c;

	vertices = NULL;
}

MoveStroke::~MoveStroke()
{

}

void MoveStroke::setPosition(float x, float y, float gx, float gy)
{
	pos = Vector2(x,y);
	guiPos = Vector2(gx, gy);
}

void MoveStroke::setDirection(float nx, float ny)
{
	//keep track of the last three movement vectors to smooth direction vector
/*	Vector2 d = Vector2(nx, ny);

	for (int i=NUMMOVEDIR-1; i>0; --i)
		dir[i] = dir[i-1];
	dir[0] = d;

	//rotate stroke
	rotP.clear();

	Vector2 pDir = Vector2(0,1);
	Vector2 newDir = dir[0];
	for (int i=1; i<NUMMOVEDIR; ++i)
		newDir += dir[i];
	
	float rot = pDir.calcSignedAngleWith(newDir);

	bounds = BoundingBox();

	for (uint i=0; i<p.size(); ++i)
	{
		rotP.push_back(p[i]);
		rotP.back().rotateAroundZero(rot);
		bounds.merge(rotP.back());
	}
*/
}

void MoveStroke::pushPoints(float dx, float dy, vector<ControlPoint>& points)
{
	debugP0.clear();
	debugP1.clear();
	debugP2.clear();

	Vector2 d = Vector2(dx,dy);
	for (uint i=0; i<points.size(); ++i)
	{
		debugP1.push_back(points[i].p);
		for (uint j=0; j<p.size()-2; ++j)
		{
			if (triangleContribution(points[i].p, j))
			{
				points[i].p += d*2.15;
				debugP2.push_back(points[i].p);
				break;
			}
		}
	}

/*	std::cout<<"pushing points\n";
	Vector2 d = Vector2(dx,dy);
	for (uint i=0; i<points.size(); ++i)
	{
		debugP0.push_back(points[i].p);

		Vector2 p2 = points[i].p;
		//for each point, find the move stroke points they are close to
		float minDist = 999999.0f;
		uint minP = -1;
		for (uint j=0; j<p.size(); ++j)
		{
			float distSqr = (p[j]+pos - points[i].p).lengthSquared();
			if (distSqr < minDist)
			{
				minDist = distSqr;
				minP = j;
			}
		}
		//std::cout<<"minP "<<minP<<"\n";
		
		//closest point is found, now check which side it's on
		Vector2 p0 = p[minP];
		Vector2 n; Vector2 p1;
		if (minP < p.size()-1)
			p1 = p[minP+1];
		else
			p1 = p[minP-1]; //last point is a special case

		debugP2.push_back(p0);
		debugP2.push_back(p2);
		
		n = (p1-p0).normalized();
		n = -Vector2(-n.y, n.x); //get normal dir
		//n = Vector2(0,-1);

		//compare situation before with the situation after
		Vector2 postQ = points[i].p - p0; //current position of p in regards to closest move point
		Vector2 preQ  = points[i].p - (p0-Vector2(dx,dy)); //position of closest move point before the move

		//check if angle is smaller or greater than 90 degrees (smaller: + side, larger: - side)
		float preAngle  = n.calcAngleWith(preQ)*TO_DEGREES;  //angle before
		float postAngle = n.calcAngleWith(postQ)*TO_DEGREES; //angle after
		//std::cout<<"preAngle: "<<preAngle<<" ";
		//if (preAngle < 90.0f && postAngle > 90.0f || preAngle > 90.0f && postAngle < 90.0f) //p switched sides in regard to p0
		
		float u = ( (p2.x-p0.x)*(p1.x-p0.x) + (p2.y-p0.y)*(p1.y-p0.y)  ) / (p1-p0).lengthSquared();
		float xI = p0.x+u*(p1.x-p0.x);
		float yI = p0.y+u*(p1.y-p0.y);

		float dist = (Vector2(xI, yI) - p2).length();
		std::cout<<" "<<dist;
		if (postAngle > 90)
		{
			//std::cout << "switched sides!\n";
			points[i].p += Vector2(dx, dy)*1.01f;
			debugP1.push_back(points[i].p);
		}
	}*/
}

bool MoveStroke::triangleContribution(Vector2& q, uint indx)
{
	/*Vector2 p0 = Vector2(-30,-30);
	Vector2 p1 = Vector2(-30,30);
	Vector2 p2 = Vector2(30,-30);
*/
	Vector2 p0 = p[indx];
	Vector2 p1 = p[indx+1];
	Vector2 p2 = p[indx+2];
	
	p0 += pos;
	p1 += pos;
	p2 += pos;

	float b0 = (p1.x-p0.x)*(p2.y-p0.y) - (p2.x-p0.x)*(p1.y-p0.y);
	float b0Inv = 1.0f / b0;

	float b1 = ( (p1.x-q.x)*(p2.y-q.y) - (p2.x-q.x)*(p1.y-q.y) ) * b0Inv;
	float b2 = ( (p2.x-q.x)*(p0.y-q.y) - (p0.x-q.x)*(p2.y-q.y) ) * b0Inv;
	float b3 = ( (p0.x-q.x)*(p1.y-q.y) - (p1.x-q.x)*(p0.y-q.y) ) * b0Inv;

	if (b1 < 0.0f || b2 < 0.0f || b3 <0.0f)
		return false;

	return true;
}

void MoveStroke::draw(float zoom)
{
	delete[] vertices;
	vertices = new GLfloat[p.size()*2];
	for (uint i=0; i<p.size(); ++i)
	{
		vertices[i*2] = p[i].x*zoom+guiPos.x;
		vertices[i*2+1] = p[i].y*zoom+guiPos.y;
	}

	glColor3f(1.0, 0,0);

	glEnableClientState(GL_VERTEX_ARRAY);

	glVertexPointer(2, GL_FLOAT, 0, vertices);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, p.size());

	glDisableClientState(GL_VERTEX_ARRAY);	
	
	/*glPointSize(3);
	glBegin(GL_POINTS);
	for (uint i=0; i<rotP.size(); ++i)
		glVertex2f(vertices[i*2], vertices[i*2+1]);
	//glVertex2f(-30+guiPos.x, -30+guiPos.y);
	//glVertex2f(-30+guiPos.x, 30+guiPos.y);
	//glVertex2f(30+guiPos.x, -30+guiPos.y);
	glEnd();

	glColor3f(0.0, 0.7,0);
	glPointSize(3);
	glBegin(GL_POINTS);
	for (uint i=0; i<debugP0.size(); ++i)
		glVertex2f(debugP0[i].x, debugP0[i].y);
	glEnd();

	glColor3f(0.0, 0.0,1);
	glPointSize(5);
	glBegin(GL_POINTS);
	for (uint i=0; i<debugP1.size(); ++i)
		glVertex2f(debugP1[i].x, debugP1[i].y);
	glEnd();

	glColor3f(1.0, 0.0, 1);
	//glPointSize(5);
	glBegin(GL_LINES);
	for (uint i=0; i<debugP2.size(); i+=2)
	{
		glVertex2f(debugP2[i].x, debugP2[i].y);
		glVertex2f(debugP2[i].x, debugP2[i].y+25);
	}
	glEnd();*/
}