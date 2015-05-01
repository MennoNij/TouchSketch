#pragma once

#include <iostream>
#include <vector>

#include "BoundingBox.h"
#include "Vector2.h"


#define uint unsigned int
using std::vector;

class Stroke;

enum Quadrants { BOTTOM_L, BOTTOM_R, TOP_L, TOP_R };

//represents an approximation to a segment of a stroke
struct StrokeSegment
{
	Vector2 p0;
	Vector2 p1;
	Stroke* stroke;
	uint segIndex;

	StrokeSegment(Vector2& pp0, Vector2& pp1, Stroke* str=NULL, uint seg=0) : p0(pp0), p1(pp1)
	{
		stroke = str;
		segIndex = seg;
	}
	~StrokeSegment() {}
};

//quadtree element
struct Square
{
	BoundingBox bounds; //Bounding Box
	vector<StrokeSegment> bucket; //array of all the segments in this square
	Square* parent; //parent of this square
	Square* children[4]; //children of this square

	Square()
	{
		parent = NULL;
		children[0] = children[1] = children[2] = children[3] = NULL;
	}
	Square(Vector2& min, Vector2& max, Square* par=NULL)
	{
		bounds = BoundingBox(min, max);
		parent = par;
		children[0] = children[1] = children[2] = children[3] = NULL;
	}
	~Square() {}

	void setBounds(Vector2& min, Vector2& max)
	{
		bounds = BoundingBox(min, max);
	}
	int findQuadrants(Vector2& p0, Vector2& p1, bool quadrant[]);
	void createChildren();
	void removeChildren();

	void draw(float colorVal);
};

//quadtree containing the linesegments of all the strokes
class QuadTree
{
public:
	Square root; //root node
	vector<Square> nodes; //all child nodes
	int maxDepth; //maximum tree depth

	bool treeDebug;

	QuadTree();
	QuadTree(float width, float height, float minCellHeight);
	~QuadTree();

	//insert a segment into the tree
	void insert(StrokeSegment& pq);
	//remove a segment from the tree
	void remove(StrokeSegment* pq);
	//remove segment from a square
	inline void remove(Square* sqr, StrokeSegment* pq);
	//approximately find all segments inside an area
	vector<StrokeSegment*> findInRange(BoundingBox& range);
	//find the square containing pq
	Square* findSquare(StrokeSegment* pq);
	//find strokes intersecting the range
	void findIntersectingStrokes(BoundingBox& bounds, vector<Stroke*>& intersecting);

	void addStroke(Stroke* stroke);
	void draw();
};



