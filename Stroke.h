#pragma once

#include <windows.h>
#include <GL/gl.h>
#include <vector>

#include "Vector2.h"
#include "StrokeList.h"
#include "BoundingBox.h"

/*
Stroke is the view dependent line representation
*/

#define uint unsigned int
#define SUBDIV_CONST 0.5f //smaller means more refinement

using std::vector;
class MergeTree;
class QuadTree;
//class BoundingBox;
struct ListNode;
struct StrokeSegment;

enum strokeStyles { SQUARE, ROUND };

struct ControlPoint
{
	Vector2 p; //location of cp
	float r;  //disk radius
	float a; //cp alpha value

	ControlPoint(float x, float y, float r = 1.0f, float a = 1.0f)
	{
		this->p.x = x;
		this->p.y = y;
		this->r = r;
		this->a = a;
	}
	~ControlPoint() { }
};

class Stroke
{
public:
	vector<ControlPoint> CPs; //Control Points
	vector<MergeTree> segments; //merge trees of stroke segments
	GLfloat* vertices; //vertex geometry
	GLfloat* colours; //vertex alpha values
	uint numVerts; //number of vertices
	BoundingBox bounds;
	float curMaxRefinement; //max zoom level the stroke is refined for
	ListNode* entry;
	//char style; //style of the stroke

	Stroke();
	Stroke(float x, float y, float r, float a=1.0f);
	Stroke(Stroke& stroke);
	~Stroke();

	//## handling stroke data
	
	//called by the InputHandler to extend the stroke in the buffer
	void extend(float x, float y, float r, float a=1.0f);
	//resamples stroke in the window range
	int resample(int winStart=0, int winEnd=0, float scale=1.0f, float maxError=2.0f);
	//calculate the error of CPs[index]
	float calcError(int index);
	//subdivide a segment
	void errorSubDiv(vector<float>& tValues, vector<float>& thresholds, float left, float right, 
										Vector2 pLeft, Vector2 pRight, float coeffX[], float coeffY[], float scale);
	//finalize the stroke; do some maintenance like fixing the end tip
	void finalize();
	//update stroke detail
	void increaseRefinement(float scale);
	//generate new equally spaced control points in (p0...p1)
	void calcSegmentForwardDiff(uint seg, float spacing, vector<ControlPoint>& sampled);
	//replace a range of control points with a new set
	void replace(int start, int end, vector<ControlPoint>& newPoints);
	void popFirstCP();
	void popLastCP();
	void removeFromQuadTree(QuadTree& tree);
	//returns the min/max cp of the affected segment of the stroke
	bool findOverlappingSegment(BoundingBox& bounds, int& minCP, int& maxCP);

	//## generating geometry
	
	//extends stroke with a temporary, straight line segment
	void appendTempSegment(ControlPoint& newCP);
	//init empty segments
	void initSegments();
	//generate the mergetree of a segment
	void genSegmentMergeTree(uint seg, float scale=1.0f);
	//generate all mergetrees
	void genMergeTrees(float scale=1.0f);
	//generate vertex array of the stroke
	void genVertexArray(float scale=1.0f);

	void genExportArrays(float scale, vector<GLfloat>& verts, vector<GLfloat>& color);

	void removeFolds();

	inline float sideTest(Vector2 v0, Vector2 v1, Vector2 p)
	{
		Vector2 n = calcLineNormal(v0, v1);
    
    //scaling factor (dot between normal and rib vertex)
    float d1 = v0.x*n.x + v0.y*n.y;
    //dot product of normal and point
    float dot = p.x*n.x + p.y*n.y;
    
    return dot-d1;
	}

	//generate tips of the stroke, overload for different styles
	vector<GLfloat> Stroke::genTip(Vector2& v1, Vector2& v2, bool reverse)
	{
		return genRoundTip(v1, v2, reverse);
	}
	vector<GLfloat> genRoundTip(Vector2& pp1, Vector2& pp2, bool reverse);

	//## inline functions
	
	inline void insertControlPoint(ControlPoint point)
	{
		bounds.merge(point.p);
    CPs.push_back(point);
	}

	inline void buildControlPoint(float x, float y, float r, float a)
	{
		bounds.merge(Vector2(x,y));
		CPs.push_back(ControlPoint(x,y,r,a));
	}

	inline void calcBounds()
	{
		bounds = BoundingBox();
		for (uint i=0; i<CPs.size(); ++i)
			bounds.merge(CPs[i].p);
	}

  //calculates the b-spline at t in 1 dimension
	inline float calcSplinePoint(float coeff[], float t)
	{
		return 0.166666667f * ( ( (coeff[0]*t + coeff[1]) * t + coeff[2]) * t + coeff[3] );
	}

	//generate b-spline coefficients, given surrounding control points
	inline void calcSplineCoeffs(float d[], float coeff[])
	{
		coeff[0] = -d[0]   + 3*d[1] - 3*d[2] + d[3];
		coeff[1] = 3*d[0]  - 6*d[1] + 3*d[2];
		coeff[2] = -3*d[0] + 3*d[2];
		coeff[3] = d[0]    + 4*d[1] + d[2];
	}

	//calculate the endpoints of a rib, given the center line point, radius and normal dir
	inline void calcRibPoints(Vector2& p0, float r0, Vector2& n, Vector2& p1, Vector2& p2)
	{
		p1 = p0 + n*r0;
		p2 = p0 - n*r0;
	}

  //calculate normal of a line defined by p0 and p1
	static inline Vector2 calcLineNormal(Vector2& p0, Vector2& p1)
	{
		Vector2 a = p1-p0;
    
		//normal is perpendicular to tangent
		Vector2 n = Vector2(-a.y, a.x);

		return n.normalized();
	}

	//get the control points of a segment
	inline void getSegmentCPs(uint seg, float dx[], float dy[], float dr[]);
	inline void getSegmentCPs(uint seg, float dx[], float dy[]);

	inline float clamp(float x, float a, float b)
	{
		return x < a ? a : (x > b ? b : x);
	}
};