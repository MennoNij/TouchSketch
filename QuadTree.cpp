#include "QuadTree.h"

#include <windows.h>
#include <GL/gl.h>
#include <stdlib.h>

#include "Stroke.h"
#include "QuadTree.h"

int Square::findQuadrants(Vector2& p0, Vector2& p1, bool quadrant[])
{
	//we assume the segment is inside the square
	Vector2 center = (bounds.pMin + bounds.pMax)*0.5f;
	//translate segment
	Vector2 tP0 = p0-center;
	Vector2 tP1 = p1-center;

	int qIndex = -1;

	if (tP0.x < 0) //left
		if (tP0.y < 0) {
			quadrant[BOTTOM_L] = true;
			qIndex = BOTTOM_L;
		} else {
			quadrant[TOP_L] = true;
			qIndex = TOP_L;
		}
	else //right
		if (tP0.y < 0) {
			quadrant[BOTTOM_R] = true;
			qIndex = BOTTOM_R;
		} else {
			quadrant[TOP_R] = true;
			qIndex = TOP_R;
		}

	if (tP1.x < 0) //left
		if (tP1.y < 0)
			quadrant[BOTTOM_L] = true;
		else
			quadrant[TOP_L] = true;
	else //right
		if (tP1.y < 0)
			quadrant[BOTTOM_R] = true;
		else
			quadrant[TOP_R] = true;

	return qIndex; //value only means something when segment falls in 1 quadrant
}

void Square::createChildren()
{
	Vector2 center = (bounds.pMin + bounds.pMax)*0.5f;
	children[BOTTOM_L] = new Square(bounds.pMin, center, this);
	children[BOTTOM_R] = new Square(Vector2(center.x,bounds.pMin.y), Vector2(bounds.pMax.x, center.y), this);
	children[TOP_R]    = new Square(center, bounds.pMax, this);
	children[TOP_L]    = new Square(Vector2(bounds.pMin.x,center.y), Vector2(center.x, bounds.pMax.y), this);
}

void Square::removeChildren()
{
	delete children[0];
	children[0] = NULL;
	delete children[1];
	children[1] = NULL;
	delete children[2];
	children[2] = NULL;
	delete children[3];
	children[3] = NULL;

	//std::cout<<"child null: "<<children[0]<<"\n";
}

void Square::draw(float colorVal)
{
	//draw square
	glColor3f(0.7f,0.7f,0.7f);
	glBegin(GL_LINES);
	glVertex3f(bounds.pMin.x,bounds.pMin.y,0);
	glVertex3f(bounds.pMin.x,bounds.pMax.y,0);

	glVertex3f(bounds.pMin.x,bounds.pMin.y,0);
	glVertex3f(bounds.pMax.x,bounds.pMin.y,0);

	glVertex3f(bounds.pMax.x,bounds.pMax.y,0);
	glVertex3f(bounds.pMin.x,bounds.pMax.y,0);

	glVertex3f(bounds.pMax.x,bounds.pMax.y,0);
	glVertex3f(bounds.pMax.x,bounds.pMin.y,0);

	glEnd();

	//draw lines in bucket
	glPointSize(4);
	glColor3f(colorVal,0.0f,1.0f-colorVal);
	glBegin(GL_POINTS);
	for (uint i=0; i<bucket.size(); ++i)
	{

		glVertex3f(bucket[i].p0.x, bucket[i].p0.y, 0.0f);
		glVertex3f(bucket[i].p1.x, bucket[i].p1.y, 0.0f);
	}
	glEnd();
	glPointSize(1);

	glBegin(GL_LINES);
	for (uint i=0; i<bucket.size(); ++i)
	{
		glVertex3f(bucket[i].p0.x, bucket[i].p0.y, 0.0f);
		glVertex3f(bucket[i].p1.x, bucket[i].p1.y, 0.0f);
	}
	glEnd();

	//draw children
	if (children[0] != NULL)
	{
		children[0]->draw(colorVal+0.2f);
		children[1]->draw(colorVal+0.2f);
		children[2]->draw(colorVal+0.2f);
		children[3]->draw(colorVal+0.2f);
	}
}

QuadTree::QuadTree() { }
QuadTree::QuadTree(float width, float height, float minCellHeight)
{
	//init root node
	//root.setBounds(Vector2(-width*0.5f, -height*0.5f), Vector2(width*0.5f, height*0.5f));
	root.setBounds(Vector2(0.0f, 0.0f), Vector2(width, height));	
	
	//determine max tree depth
	float divide = height;
	int depth = 0;
	while(divide > minCellHeight)
	{
		divide *= 0.5f;
		++depth;
	}
	maxDepth = depth;
	std::cout<<"maxDepth: "<<maxDepth<<"\n";

	treeDebug = false;
}

QuadTree::~QuadTree() {}

void QuadTree::insert(StrokeSegment& pq)
{
	//compare segment against the tree
	Square* curSquare = &root;
	int depth = 0;

	//std::cout<<"\nINSERTING SEGMENT\n";

	if (!curSquare->bounds.inside(pq.p0, pq.p1))
		return; //segment does not fall inside the tree

	while (true) //iterate through the tree
	{
		//is this square a leaf?
		if (curSquare->children[0] == NULL)	//we've reached the current bottom 
		{
			//if this square is empty, or max depth has been reached, add it to the bucket
			if (curSquare->bucket.empty() || depth == maxDepth)
				curSquare->bucket.push_back(pq);
			else
			{
				//otherwise, give this square children
				curSquare->createChildren();

				//resassign the segments in the bucket to the square or one of the children
				bool quadrant[4];
				curSquare->bucket.push_back(pq); //add pq to the bucket (for convenience)
				vector<StrokeSegment> bucket = curSquare->bucket;
				curSquare->bucket.clear();
				
				for (uint i=0; i<bucket.size(); ++i)
				{
					quadrant[0] = quadrant[1] = quadrant[2] = quadrant[3] = false;
					int qIndex = curSquare->findQuadrants(bucket[i].p0, bucket[i].p1, quadrant);
					int numQ = quadrant[0]+quadrant[1]+quadrant[2]+quadrant[3]; //number of quads the line covers
					
					if (numQ > 1) //line covers two quadrants, this is the smallest square where the segment will fit
						curSquare->bucket.push_back(bucket[i]);
					else //otherwise line just covers one quadrant, add segment to that child
						curSquare->children[qIndex]->bucket.push_back(bucket[i]);
				}
			}
			break; //from while; we're done
		}
		else //square is a parent, check inside which children te segment falls
		{
			bool quadrant[4];
			quadrant[0] = quadrant[1] = quadrant[2] = quadrant[3] = false;
			
			int qIndex = curSquare->findQuadrants(pq.p0, pq.p1, quadrant);
			int numQ = quadrant[0]+quadrant[1]+quadrant[2]+quadrant[3]; //number of quads the line covers

			//'worst case': line covers two quadrants, this is the smallest square where the segment will fit
			if (numQ > 1)
			{
				curSquare->bucket.push_back(pq);
				break; //from while; we're done
			}
			else //otherwise line just covers one quadrant, iterate with that child
			{
				curSquare = curSquare->children[qIndex];
			}
		}
		++depth;
	} //while

}

void QuadTree::remove(StrokeSegment* pq)
{
	Square* result = findSquare(pq);
	if (result != NULL)
		remove(result, pq);
}

inline void QuadTree::remove(Square* sqr, StrokeSegment* pq)
{
	//remove segment from square
	for (uint i=0; i<sqr->bucket.size(); ++i)
	{
		if ( (pq->p0.x == sqr->bucket[i].p0.x && pq->p0.y == sqr->bucket[i].p0.y &&
			   pq->p1.x == sqr->bucket[i].p1.x && pq->p1.y == sqr->bucket[i].p1.y) )//|| pq->stroke == sqr->bucket[i].stroke ) //found it
		//if (pq->stroke == sqr->bucket[i].stroke && pq->segIndex == sqr->bucket[i].segIndex)
		{
			vector<StrokeSegment>::iterator it = sqr->bucket.begin() + i;
			sqr->bucket.erase(it);
			//if this square is empty, check if things can be deleted
			if (sqr->bucket.empty() && sqr != &root)
			{
				Square* delSqr = NULL;
				if (sqr->children[0] != NULL) //parent
					delSqr = sqr;
				else //leaf
					delSqr = sqr->parent;

				if (delSqr->children[0]->children[0] == NULL && delSqr->children[1]->children[0] == NULL &&
					  delSqr->children[2]->children[0] == NULL && delSqr->children[3]->children[0] == NULL)
					if (delSqr->children[0]->bucket.empty() && delSqr->children[1]->bucket.empty() &&
							delSqr->children[2]->bucket.empty() && delSqr->children[3]->bucket.empty())
					{
						delSqr->removeChildren();
					}
			}
			return;
		}
	}
	std::cout<<"couldn't find\n";
}

Square* QuadTree::findSquare(StrokeSegment* pq)
{
	Square* curSquare = &root;
	bool quadrant[4];
	
	while (true)
	{
		if (curSquare->children[0] != NULL) //parent
		{
			quadrant[0] = quadrant[1] = quadrant[2] = quadrant[3] = false;
			//find quadrant pq is in
			int qIndex = curSquare->findQuadrants(pq->p0, pq->p1, quadrant);
			int numQ = quadrant[0]+quadrant[1]+quadrant[2]+quadrant[3];

			if (numQ > 1) //this is smallest square that could contain pq
				return curSquare;
			else //continue iteration with the child
				curSquare = curSquare->children[qIndex];
		}
		else //leaf
			return curSquare;
	}
}

vector<StrokeSegment*> QuadTree::findInRange(BoundingBox& range)
{
	Square* curSquare = &root;
	vector<StrokeSegment*> lines;
	vector<Square*> stack;

	while (true)
	{
		if (curSquare->bounds.overlaps(range))
		{
			//square overlaps the query range. If it has lines, include them
			if (!curSquare->bucket.empty())
				for (uint i=0; i<curSquare->bucket.size(); ++i)
					if (range.overlaps(curSquare->bucket[i].p0, curSquare->bucket[i].p1))
						lines.push_back(&curSquare->bucket[i]);

			//if the square has children, check them
			if (curSquare->children[0] != NULL)
			{
				//push all 4 children on the stack
				stack.insert(stack.end(), curSquare->children, curSquare->children+4);
			}
		}
		if (stack.empty())
			break; //from while; we're done
		else
		{
			curSquare = stack.back();
			stack.pop_back();
		}
	}

	return lines;
}

void QuadTree::findIntersectingStrokes(BoundingBox& bounds, vector<Stroke*>& intersecting)
{
	vector<StrokeSegment*> affected = findInRange(bounds);
	for (uint i=0; i<affected.size(); ++i)
	{
		uint j;
		for (j=0; j<intersecting.size(); ++j) //find stroke the segment belongs to
			if (affected[i]->stroke == intersecting[j]) //found it, we were already editing this stroke
				break;
		
		if (j == intersecting.size()) //found a new stroke to edit
		{		
			intersecting.push_back(affected[i]->stroke);
		}
	}
}

void QuadTree::addStroke(Stroke* stroke)
{
	for (uint i=0; i<stroke->CPs.size()-1; ++i)
	{
		StrokeSegment pq = StrokeSegment(stroke->CPs[i].p, stroke->CPs[i+1].p, stroke, i);
		insert(pq);		
	}
}

void QuadTree::draw()
{
	/*glLineWidth(3);
	glColor3f(0.7f,0.7f,0.7f);
	glBegin(GL_LINES);
	glVertex3f(root.bounds.pMin.x,root.bounds.pMin.y,0);
	glVertex3f(root.bounds.pMin.x,root.bounds.pMax.y,0);

	glVertex3f(root.bounds.pMin.x,root.bounds.pMin.y,0);
	glVertex3f(root.bounds.pMax.x,root.bounds.pMin.y,0);

	glVertex3f(root.bounds.pMax.x,root.bounds.pMax.y,0);
	glVertex3f(root.bounds.pMin.x,root.bounds.pMax.y,0);

	glVertex3f(root.bounds.pMax.x,root.bounds.pMax.y,0);
	glVertex3f(root.bounds.pMax.x,root.bounds.pMin.y,0);

	glEnd();
	glLineWidth(1);
*/
	float fade = 15.0f;
	Vector2 nBL = Vector2(-1,-1);
	Vector2 nTL = Vector2(-1, 1);
	Vector2 nTR = Vector2(1, 1);
	Vector2 nBR = Vector2(1, -1);

	glBegin(GL_TRIANGLE_STRIP);
	glColor4f(0.7f,0.7f,0.7f,1.0f); glVertex2f(root.bounds.pMin.x, root.bounds.pMin.y);
	glColor4f(0.7f,0.7f,0.7f,0.1f); glVertex2f(root.bounds.pMin.x+nBL.x*fade, root.bounds.pMin.y+nBL.y*fade);

	glColor4f(0.7f,0.7f,0.7f,1.0f); glVertex2f(root.bounds.pMin.x, root.bounds.pMax.y);
	glColor4f(0.7f,0.7f,0.7f,0.1f); glVertex2f(root.bounds.pMin.x+nTL.x*fade, root.bounds.pMax.y+nTL.y*fade);

	glColor4f(0.7f,0.7f,0.7f,1.0f); glVertex2f(root.bounds.pMax.x, root.bounds.pMax.y);
	glColor4f(0.7f,0.7f,0.7f,0.1f); glVertex2f(root.bounds.pMax.x+nTR.x*fade, root.bounds.pMax.y+nTR.y*fade);

	glColor4f(0.7f,0.7f,0.7f,1.0f); glVertex2f(root.bounds.pMax.x, root.bounds.pMin.y);
	glColor4f(0.7f,0.7f,0.7f,0.1f); glVertex2f(root.bounds.pMax.x+nBR.x*fade, root.bounds.pMin.y+nBR.y*fade);

	glColor4f(0.7f,0.7f,0.7f,1.0f); glVertex2f(root.bounds.pMin.x, root.bounds.pMin.y);
	glColor4f(0.7f,0.7f,0.7f,0.1f); glVertex2f(root.bounds.pMin.x+nBL.x*fade, root.bounds.pMin.y+nBL.y*fade);

	glEnd();

	if (treeDebug)
		root.draw(0.0f);

}
