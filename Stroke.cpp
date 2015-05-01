#include <iostream>
#include <algorithm>
#include <queue>
#include <functional>

#include "Stroke.h"
#include "MergeTree.h"
#include "QuadTree.h"
#include "BoundingBox.h"

using std::cout;

Stroke::Stroke()
{
	numVerts = 0;
	vertices = NULL;
	colours = NULL;
	curMaxRefinement = 2.0f;
}

Stroke::Stroke(float x, float y, float r, float a)
{
	//initialize the stroke at (x,y) as just two 'tips'
	buildControlPoint(x, y, r, a);
	numVerts = 0;
	vertices = NULL;
	colours = NULL;
	curMaxRefinement = 2.0f;
	//this->style = style;
}

Stroke::Stroke(Stroke& stroke)
{
	this->CPs = stroke.CPs;
	this->segments = stroke.segments;
	this->numVerts = stroke.numVerts;
	this->bounds = stroke.bounds;
	vertices = NULL;
	colours = NULL;
	curMaxRefinement = stroke.curMaxRefinement;

	//init the stroke with one dummy segment
	//Vector2 v1, v2;
	//calcRibPoints(CPs[0].p, CPs[0].r, Vector2(1,0), v1, v2);

	//segments.push_back(MergeTree());
	//segments.back().addNode(MergeNode(v1, v2, 1.0f, 2.0f));
/*
	segments.back().addNode(MergeNode(v1, v2, 0.5f, 2.0f));
	segments.back().addNode(MergeNode(v1, v2,  0.25f, 2.0f));
	segments.back().addNode(MergeNode(v1, v2,  0.12f, 2.0f));
	segments.back().addNode(MergeNode(v1, v2,  0.37f, 2.0f));
	segments.back().addNode(MergeNode(v1, v2,  0.75f, 2.0f));
	segments.back().addNode(MergeNode(v1, v2,  0.63f, 2.0f));
	segments.back().addNode(MergeNode(v1, v2,  0.87f, 2.0f));
	segments.back().printIt();*/

}

Stroke::~Stroke()
{

}

//HANDLE STROKE DATA

void Stroke::extend(float x, float y, float r, float a)
{
	ControlPoint newCP = ControlPoint(x, y, r, a);
	appendTempSegment(newCP);
	//resample();
}

//compute the "error" of all points p with respect to the spline defined by its neighbours (without p)
//first and last CP of the stroke should never be resampled!
int Stroke::resample(int winStart, int winEnd, float scale, float maxError)
{
	if (winStart < 1) //don't allow removal of first point
		winStart = 1;

	//std::priority_queue<ErrorNode, vector<ErrorNode>, ErrorPriority> errorDist;
	vector<float> errorDist;
	int numRemoved = 0;

	//compute the errors and add them to the ?priority queue?	
	for (int i=winStart; i<winEnd; i++)
		errorDist.push_back( calcError(i) );
	
	float minError = 0.0f;
	while (minError < maxError)
	{
		minError = maxError;
		int minIndex = -1;
		//search for the minimal error
		for (uint i=0; i<errorDist.size(); i++)
		{
			if (errorDist[i] < minError) {
				minError = errorDist[i];
				minIndex = i;
			}
		}
		if (minIndex < 0)
			break; //from while; there are no removable points left

		//remove the point with the smallest error (least significant)
		++numRemoved;

		std::vector<ControlPoint>::iterator i = CPs.begin() + winStart + minIndex;
		CPs.erase(i);
		std::vector<float>::iterator k = errorDist.begin() + minIndex;
		errorDist.erase(k);
		std::vector<MergeTree>::iterator j = segments.begin() + winStart + minIndex;
		segments.erase(j);

/*
possible optimization:
Specialize std::swap() for the forest_goo struct so that you can swap two of them and have them change 
places without allocating memory. (On most STLs, swap() of vectors is already specialized, 
so you just have to tell the compiler to swap all the pieces.)

Rewrite the algorithm to swap the to-be-killed forest_goo structs to the end of the vector. The swap is a no-mem-alloc operation. 
When done, you can then chop the entire end off of the array with clear, which won't require moving forest_goo structs around.
*/


		//update error values of the 4 points that previously surrounded the deleted point
		if (minIndex-2 > 1)
			errorDist[minIndex-2] = calcError(winStart+minIndex-2); //don't change the first or last two distances
		if (minIndex-1 > 1)
			errorDist[minIndex-1] = calcError(winStart+minIndex-1);
		if (minIndex < (int)errorDist.size())
			errorDist[minIndex] = calcError(winStart+minIndex);
		if (minIndex+1 < (int)errorDist.size())
			errorDist[minIndex+1] = calcError(winStart+minIndex+1);
	}
	
	//recompute merge trees
	for (int i=winStart-1; i < winEnd-numRemoved; ++i)
		genSegmentMergeTree(i, scale);

	calcBounds();

	return winEnd-numRemoved; //new last index + 1 of the window
}

float Stroke::calcError(int index)
{
	ControlPoint& cp = CPs[index];
	float dx[4],     dy[4];
	float coeffX[4], coeffY[4];
	
	//spline formed by the points surrounding p
	if (index > 1) {
		dx[0] = CPs[index-2].p.x; dy[0] = CPs[index-2].p.y;
	} else {  //'virtual' point
		dx[0] = CPs[index-1].p.x; dy[0] = CPs[index-1].p.y;
	}
	dx[1] = CPs[index-1].p.x; dy[1] = CPs[index-1].p.y;
	dx[2] = CPs[index+1].p.x; dy[2] = CPs[index+1].p.y;
	
	if (index < CPs.size()-2) {
		dx[3] = CPs[index+2].p.x; dy[3] = CPs[index+2].p.y;
	} else { //virtual point
		dx[3] = CPs[index+1].p.x; dy[3] = CPs[index+1].p.y;
	}

	/*ControlPoint cp = ControlPoint(10,1,5);
	ControlPoint cpm2 = ControlPoint(6,2,3);
	ControlPoint cpm1 = ControlPoint(8,4,3);
	ControlPoint cpp1 = ControlPoint(12,4,7);
	ControlPoint cpp2 = ControlPoint(14,2,7);

	dx[0] = cpm2.p.x; dy[0] = cpm2.p.y;
	dx[1] = cpm1.p.x; dy[1] = cpm1.p.y;
	dx[2] = cpp1.p.x; dy[2] = cpp1.p.y;
	dx[3] = cpp2.p.x; dy[3] = cpp2.p.y;
*/

	calcSplineCoeffs(dx, coeffX);
	calcSplineCoeffs(dy, coeffY);

	//approximate distance between spline and cp
	Vector2& p1 = CPs[index-1].p; Vector2& p2 = CPs[index+1].p;
	//Vector2& p1 = cpm1.p; Vector2& p2 = cpp1.p;

	Vector2 n = calcLineNormal(p1, p2);
	Vector2 p3 = cp.p+n*100.0f; Vector2 p4 = cp.p-n*100.0f;

	//find the intersection distance along line p1,p2
	float nom = (p4.y-p3.y)*(p2.x-p1.x) - (p4.x-p3.x)*(p2.y-p1.y);
	if (nom != 0.0f)
	{
		float t = ( (p4.x-p3.x)*(p1.y-p3.y) - (p4.y-p3.y)*(p1.x-p3.x) ) / nom;

		t = clamp(t, 0.0f, 1.0f); //clamp t between 0 and 1

		//std::cout << "found t: "<<t<<"\n";

		//calculate spline value at this distance
		Vector2 q = Vector2(calcSplinePoint(coeffX, t), calcSplinePoint(coeffY, t));
		Vector2 pq = Vector2(cp.p.x-q.x, cp.p.y-q.y);

		return pq.length();
	}
	return 0.0f;
}

void Stroke::finalize()
{
/*	//fix end tip
	Vector2 v1, v2;
	//remove second to last segment/cp
	vector<MergeTree>::iterator it;
	it = segments.end()-2;
	segments.erase(it, it+1);

	vector<ControlPoint>::iterator jt;
	jt = CPs.end()-2;
	CPs.erase(jt, jt+1);
	
	segments.back().clear();

	//calculate rib
	Vector2 n = calcLineNormal(CPs.back().p, CPs[CPs.size()-2].p);
	//calculate the radius
	MergeNode* lastRib = segments[segments.size()-2].getRoot();
	Vector2 radLength = lastRib->v1 - lastRib->v0;

	//segments[segments.size()-2].printIt();

	calcRibPoints(CPs.back().p, radLength.length()*0.5f, n, v1, v2);
	segments.back().addNode(MergeNode(v2, v1, 1.0f, 10.0f));
	*/
}

void Stroke::increaseRefinement(float scale)
{
	if (scale > curMaxRefinement)
	{
		//recompute mergetrees
		for (int i=0; i<(int)segments.size()-1; ++i){
			//std::cout<<"gen tree!\n";
			genSegmentMergeTree(i, scale);
		}
		curMaxRefinement = scale+1.0f; //because we always refine extra
		//std::cout<<"refined stroke to "<<scale+1.0f<<"\n";
	}
}

void Stroke::calcSegmentForwardDiff(uint seg, float spacing, vector<ControlPoint>& sampled)
{
	//get control points of this segment
	float dx[4],     dy[4],     dr[4];
	float coeffX[4], coeffY[4], coeffR[4];
	
	getSegmentCPs(seg, dx, dy, dr);

	calcSplineCoeffs(dx, coeffX);
	calcSplineCoeffs(dy, coeffY);
	calcSplineCoeffs(dr, coeffR);

	//approximate arc-length of the segment
	Vector2 len = CPs[seg].p - CPs[seg+1].p;
	float arclen = len.length();

	int numSteps = (int)(arclen / spacing); //number of spline points to sample
	float h = 1.0f / (float)numSteps; //step size

	//interpolate alpha value
	float a0 = CPs[seg].a;
	float a1 = CPs[seg+1].a;
	float alphaStep = (a1-a0) / (float)numSteps;
	float alph = (a0+a1)/2.0f;

	if (numSteps<2) {
		//sampled.push_back(ControlPoint(calcSplinePoint(coeffX, 0.5f), calcSplinePoint(coeffY, 0.5f), calcSplinePoint(coeffR, 0.0f)));
		//sampled.push_back(ControlPoint(CPs[seg].p.x, CPs[seg].p.y, CPs[seg].r));
		sampled.push_back(ControlPoint(calcSplinePoint(coeffX, 0.01f), calcSplinePoint(coeffY, 0.01f), calcSplinePoint(coeffR, 0.01f), alph));
		return;
	}

	sampled.reserve(numSteps);

	float h2 = h*h; float h3 = h2*h;

	float xP = coeffX[3] * 0.166666667f;
	float yP = coeffY[3] * 0.166666667f;
	float rP = coeffR[3] * 0.166666667f;

	float firstFDX = (coeffX[0] * (h3) + coeffX[1] * (h2) + coeffX[2] * h) * 0.166666667f;
	float firstFDY = (coeffY[0] * (h3) + coeffY[1] * (h2) + coeffY[2] * h) * 0.166666667f;
	float firstFDR = (coeffR[0] * (h3) + coeffR[1] * (h2) + coeffR[2] * h) * 0.166666667f;

	float secondFDX = (6 * coeffX[0] * (h3) + 2 * coeffX[1] * (h2)) * 0.166666667f;
	float secondFDY = (6 * coeffY[0] * (h3) + 2 * coeffY[1] * (h2)) * 0.166666667f;
	float secondFDR = (6 * coeffR[0] * (h3) + 2 * coeffR[1] * (h2)) * 0.166666667f;

	float thirdFDX = (6 * coeffX[0] * (h3)) * 0.166666667f;
	float thirdFDY = (6 * coeffY[0] * (h3)) * 0.166666667f;
	float thirdFDR = (6 * coeffR[0] * (h3)) * 0.166666667f;

	//compute points at each step using forward differencing between (p1..p2)
	for (int i=1; i<numSteps; ++i)
	{
		//std::cout<<"point: "<<xP<<","<<yP<<"\n";
		xP += firstFDX;
		yP += firstFDY;
		rP += firstFDR;

		firstFDX += secondFDX;
		firstFDY += secondFDY;
		firstFDR += secondFDR;

		secondFDX += thirdFDX;
		secondFDY += thirdFDY;
		secondFDR += thirdFDR;
		
		sampled.push_back(ControlPoint(xP, yP, rP, alph));
		alphaStep += alphaStep;
	}
}

void Stroke::replace(int start, int end, vector<ControlPoint>& newPoints)
{
	vector<ControlPoint> newCPs;
	vector<MergeTree> newSegs;

	//std::cout<<"replacing between "<<start<<" and "<<end<<"\n";
	//std::cout<<"BEFORE cp: "<<CPs.size()<<" segs "<<segments.size()<<"\n";
	//std::cout<<"newPoints size: "<<newPoints.size()<<"\n";

	//section before start
	newCPs.insert(newCPs.begin(), CPs.begin(), CPs.begin()+start+1); //[0..start]
	newSegs.insert(newSegs.begin(), segments.begin(), segments.begin()+start); //[0..start)

	//std::cout<<"adding new points\n";

	//add the new points
	newCPs.insert(newCPs.end(), newPoints.begin(), newPoints.end());
	newSegs.push_back(MergeTree());
	for (uint i=0; i<newPoints.size(); ++i)
	{
		//std::cout<<"pushing back segment "<<i<<"\n";
		newSegs.push_back(MergeTree());
		newSegs.back().addNode(MergeNode(newPoints[i].p, newPoints[i].p, 1.0f, 10.0f));	
	}
	//std::cout<<"original section after start\n";
	//original section after start
	newCPs.insert(newCPs.end(),  CPs.begin()+end, CPs.end()); //[end..n]
	newSegs.insert(newSegs.end(), segments.begin()+end, segments.end()); //[end...n]

	CPs = newCPs;
	segments = newSegs;
	//std::cout<<"AFTER cp: "<<CPs.size()<<" segs "<<segments.size()<<"\n";
}

void Stroke::popFirstCP()
{
	vector<ControlPoint> newCPs(CPs.begin()+1, CPs.end());
	CPs = newCPs;
	vector<MergeTree> newSegs(segments.begin()+1, segments.end());
	segments = newSegs;
}

void Stroke::popLastCP()
{
	CPs.pop_back();
	segments.pop_back();
}

void Stroke::removeFromQuadTree(QuadTree& tree)
{
	//remove stroke from quadtree
	for (uint k=0; k<CPs.size()-1; ++k)
	{
		StrokeSegment pq = StrokeSegment(CPs[k].p, CPs[k+1].p, this, k);
		tree.remove(&pq);
	}
}

bool Stroke::findOverlappingSegment(BoundingBox& bounds, int& minCP, int& maxCP)
{
	bool startFound = false;
	for (uint j=0; j<CPs.size()-1; ++j)
	{
		//cout<<"segment: "<<editBuffer[i]->CPs[j].p.x<<","<<editBuffer[i]->CPs[j].p.y<<" "<<editBuffer[i]->CPs[j+1].p.x<<","<<editBuffer[i]->CPs[j+1].p.y<<"\n";
		if (!startFound)
		{
			if (bounds.overlaps(CPs[j].p, CPs[j+1].p))
			{
				minCP = j;
				startFound = true;
			}
		}
		else
		{
			if (!bounds.overlaps(CPs[j].p, CPs[j+1].p))
			{
				maxCP = j;
				break;
			}
		}
	}
	return startFound;
}

//GENERATE STROKE GEOMETRY

void Stroke::appendTempSegment(ControlPoint& newCP)
{
	insertControlPoint(newCP);
	//retrieve points which segment will be between
	ControlPoint cp1 = CPs[CPs.size()-1];
	ControlPoint cp0 = CPs[CPs.size()-2];

	//calculate rib
	Vector2 n = calcLineNormal(cp0.p, cp1.p);
	Vector2 v1, v2;
	calcRibPoints(cp1.p, cp1.r, n, v1, v2); //calculate the rib at cp1

	//now add an 'empty' mergetree
	segments.push_back(MergeTree());
	segments.back().addNode(MergeNode(v1, v2, 1.0f, 10.0f));
/*
	if (CPs.size() == 3)
	{
		//we no longer need to guess the orientation of the first rib; recompute it
		segments[0].clear();

		//calculate rib
		n = calcLineNormal(CPs[0].p, CPs[2].p);
		calcRibPoints(CPs[0].p, CPs[2].r, n, v1, v2);

		//segments[0] = MergeTree();
		segments[0].addNode(MergeNode(v1, v2, 1.0f, 10.0f));
	}
*/
}

void Stroke::initSegments()
{
	segments.clear();
	for(uint i=0; i<CPs.size()-1; ++i)
		segments.push_back(MergeTree());
}

void Stroke::genSegmentMergeTree(uint seg, float scale)
{
	segments[seg].clear();
	//get control points of this segment
	float dx[4],     dy[4],     dr[4];
	float coeffX[4], coeffY[4], coeffR[4];
	
	getSegmentCPs(seg, dx, dy, dr);

	calcSplineCoeffs(dx, coeffX);
	calcSplineCoeffs(dy, coeffY);
	calcSplineCoeffs(dr, coeffR);

	//adaptivly subdivide the segment, retrieve division points
	Vector2 pLeft  = Vector2(calcSplinePoint(coeffX, 0.0f), calcSplinePoint(coeffY, 0.0f));
	Vector2 pRight = Vector2(calcSplinePoint(coeffX, 1.0f), calcSplinePoint(coeffY, 1.0f));

	//the points of this segment lie in (0.0,1.0]. 0.0 is excluded because segments are stitched together
	vector<float> tValues; vector<float> thresholds;
	tValues.push_back(1.0); thresholds.push_back(10.0f);

	errorSubDiv(tValues, thresholds, 0.0, 1.0, pLeft, pRight, coeffX, coeffY, scale+1.0f); //add more detail than needed
	
	//std::cout<<"num t values: "<<tValues.size()<<"\n";

	//spine of the stroke at selected t values
	vector<Vector2> spinePoints; spinePoints.reserve(tValues.size());
	vector<float>  spineRadius; spineRadius.reserve(tValues.size());
	for (uint i=0; i<tValues.size(); ++i)
	{
		spinePoints.push_back(Vector2(calcSplinePoint(coeffX, tValues[i]), calcSplinePoint(coeffY, tValues[i])));
		spineRadius.push_back(calcSplinePoint(coeffR, tValues[i]));
	}

	//calculate spine normals for rib directions
	vector<Vector2> spineNormals; spineNormals.reserve(tValues.size());
	for (uint i=0; i<tValues.size(); ++i)
	{
		Vector2& p0 = spinePoints[i];
		Vector2  p1 = Vector2(calcSplinePoint(coeffX, tValues[i]+0.001f), calcSplinePoint(coeffY, tValues[i]+0.001f));
		spineNormals.push_back(calcLineNormal(p0, p1));
	}

	//add ribs to the segment mergetree
	for (uint i=0; i<tValues.size(); ++i)
	{
		Vector2 v1, v2;
		calcRibPoints(spinePoints[i], spineRadius[i], spineNormals[i], v1, v2);

		segments[seg].addNode(MergeNode(v1, v2, tValues[i], thresholds[i]));
	}
}

void Stroke::genMergeTrees(float scale)
{
	for (uint j=0; j<CPs.size()-1; ++j)
	{
		genSegmentMergeTree(j, scale);
	}
}

void Stroke::errorSubDiv(vector<float>& tValues, vector<float>& thresholds, float left, float right, 
												 Vector2 pLeft, Vector2 pRight, float coeffX[], float coeffY[], float scale)
{
	float t = (left+right)*0.5f;

	//value of the straight line between p0 and p1 at t, which for this line equation is equal to the halfway point, 0.5
	Vector2 pMid = (pLeft+pRight)*0.5f;
	//value of the true curve at t
	Vector2 pCurve = Vector2(calcSplinePoint(coeffX, t), calcSplinePoint(coeffY, t));
	//error distance
	Vector2 dist = pCurve-pMid;
	float errDist = dist.length();

	if (errDist > SUBDIV_CONST/scale)
	{
		tValues.push_back(t);
		thresholds.push_back(errDist);
		errorSubDiv(tValues, thresholds, left, t,     pLeft,  pCurve, coeffX, coeffY, scale);
		errorSubDiv(tValues, thresholds, t,    right, pCurve, pRight, coeffX, coeffY, scale);
	}
}

void Stroke::genVertexArray(float scale)
{
	if (vertices != NULL)
		delete[] vertices;
	if (colours != NULL)
		delete[] colours;

	//std::cout<<"generating vertex array\n";

	//first segment is special; needs to be computed on the fly
	Vector2 v1, v2;
	if (CPs.size() < 2) //can't make a proper determination of rib orientation
	{
		calcRibPoints(CPs[0].p, CPs[0].r, Vector2(1,0), v1, v2);
	}
	else //calculate proper orientation
	{
		Vector2 n = calcLineNormal(CPs[0].p, CPs[1].p);
		calcRibPoints(CPs[0].p, CPs[0].r, n, v1, v2);
	}

	vector<GLfloat> verts = genTip(v1, v2, false); //append before the front
	vector<GLfloat> color;
	GLfloat ct0[4]; ct0[0] = 0.0f; ct0[1] = 0.0f; ct0[2] = 0.0f; ct0[3] = CPs[0].a;
	
	for (int i=0; i < verts.size()/2; ++i) //init the tip with the alpha of CP[0]
		color.insert(color.end(), ct0, ct0+4);

	//add degenerate vertices of the first rib
	verts.push_back(v2.x); verts.push_back(v2.y); color.insert(color.end(), ct0, ct0+4);
	verts.push_back(v1.x); verts.push_back(v1.y); color.insert(color.end(), ct0, ct0+4);
	verts.push_back(v2.x); verts.push_back(v2.y); color.insert(color.end(), ct0, ct0+4);

	//generate body of the stroke
	std::vector<MergeTree>::iterator seg;
	int cp = 0;
	for(seg=segments.begin(); seg!=segments.end(); seg+=1)
	{
		float a0 = CPs[cp].a; float a1 = CPs[cp+1].a; float a1ma0 = a1-a0; //get alpha values of surrounding CPs
		vector<MergeNode*> nodeList = (*seg).traverseDepth(scale);
		//iterate through nodeList, add rib vertices to vertex list
		std::vector<MergeNode*>::iterator node;
		for(node=nodeList.begin(); node!=nodeList.end(); node+=1)
		{
			v1 = (*node)->v0;
			v2 = (*node)->v1;
			GLfloat v[4];
			v[0] = v1.x; v[1] = v1.y; v[2] = v2.x; v[3] = v2.y;
			verts.insert(verts.end(),v,v+4);
			GLfloat c[4];
			c[0] = 0.0f; c[1] = 0.0f; c[2] = 0.0f; c[3] = a0+a1ma0*(*node)->t; //interpolate alpha
			color.insert(color.end(),c,c+4); //for v0
			color.insert(color.end(),c,c+4); //for v1
		}
		++cp;
	}	

	GLfloat ct1[4]; ct1[0] = 0.0f; ct1[1] = 0.0f; ct1[2] = 0.0f; ct1[3] = CPs.back().a;

	vector<GLfloat> endTip = genTip(v1, v2, true); //append after the end
	verts.insert(verts.end(), endTip.begin(), endTip.end());

	for (int i=0; i < endTip.size()/2; ++i) //init the tip with the alpha of CP[0]
		color.insert(color.end(), ct1, ct1+4);

	//save generated vertices to array
	this->numVerts = verts.size() / 2;
	this->vertices = new GLfloat[verts.size()];
	std::copy( verts.begin(), verts.end(), this->vertices);

	//save generated colours to array
	this->colours = new GLfloat[color.size()];
	std::copy( color.begin(), color.end(), this->colours);

	//removeFolds();
}

void Stroke::genExportArrays(float scale, vector<GLfloat>& verts, vector<GLfloat>& color)
{
	//get the ribs information to construct vector export file
	
	//calculate the first rib on the fly
	Vector2 v1, v2;
	if (CPs.size() < 2) //can't make a proper determination of rib orientation
	{
		calcRibPoints(CPs[0].p, CPs[0].r, Vector2(1,0), v1, v2);
	}
	else //calculate proper orientation
	{
		Vector2 n = calcLineNormal(CPs[0].p, CPs[1].p);
		calcRibPoints(CPs[0].p, CPs[0].r, n, v1, v2);
	}

	//save first rib
	verts.push_back(v1.x); verts.push_back(v1.y);
	verts.push_back(v2.x); verts.push_back(v2.y);

	//generate body of the stroke
	std::vector<MergeTree>::iterator seg;
	int cp = 0;
	for(seg=segments.begin(); seg!=segments.end(); seg+=1)
	{
		float a0 = CPs[cp].a; float a1 = CPs[cp+1].a; float a1ma0 = a1-a0; //get alpha values of surrounding CPs
		vector<MergeNode*> nodeList = (*seg).traverseDepth(scale);
		//iterate through nodeList, add rib vertices to vertex list
		std::vector<MergeNode*>::iterator node;
		for(node=nodeList.begin(); node!=nodeList.end(); node+=1)
		{
			v1 = (*node)->v0;
			v2 = (*node)->v1;
			GLfloat v[4];
			v[0] = v1.x; v[1] = v1.y; v[2] = v2.x; v[3] = v2.y;
			verts.insert(verts.end(),v,v+4);
			float alph = a0+a1ma0*(*node)->t; //interpolate alpha
			color.push_back(alph);
		}
		++cp;
	}	



}

void Stroke::removeFolds()
{
	Vector2 prevRib[2]; prevRib[0] = Vector2(1,1); prevRib[1] = Vector2(10,1);// = {1,1,10,1}; //init values not important
	int rounding = 2;

	//go over the vertex list and locate bend clipping
	int i = rounding; //skip rounding
	while (i < numVerts*2)//for (uint i=0; i<numVerts; i+=2)
	{
		//get rib
		Vector2 v0 = Vector2(vertices[i], vertices[i+1]);//Vector2(vertices[i*2], vertices[i*2+1]);
		Vector2 v1 = Vector2(vertices[i+2], vertices[i+3]);//Vector2(vertices[(i+1)*2], vertices[(i+1)*2+1]);

		i += 4;

		//cout<<"rib "<<i/2<<"\n";
		Vector2 c0 = Vector2(0,0);
		Vector2 c1 = Vector2(0,0);
		int bent = 0;
		
		if (sideTest(prevRib[0], prevRib[1], v0) < 0)
		{
			//v0 of the current rib is in violation, it's on the wrong side of the previous rib
			bent = 1;
			c0 = v0;
			c1 = v1;
		}

		if (sideTest(prevRib[0], prevRib[1], v1) < 0) //v0 of the current rib is in violation
		{
			bent = 2;
			c0 = v1;
			c1 = v0;
		}

		if (bent > 0 && i < (numVerts*2-40) && i > 40)
		{
			//cout<<"found clipping plane A\n";
			//found critical clipping rib A 
			Vector2 a0 = prevRib[0];
			Vector2 a1 = prevRib[1];

			//keep track of the 'average' point
			Vector2 average = a0+c0;
			Vector2 lastPoint;
			int totalAvg = 2;
			totalAvg = 2;

			Vector2 an = calcLineNormal(a0, a1);        
			float brushWidth = (a0-a1).lengthSquared();

			//iterate over new ribs while they are still on the wrong side of the bounds of A        
			int j = 0;
			while (i+j < numVerts*2-rounding)//for (int j=i; j<numVerts; j+=2)
			{
				//cout<<"c0 "<<c0.x<<" "<<c0.y<<"\n";
				glPointSize(3);
				glColor3f(1,0,0);
				glBegin(GL_POINTS);
				glVertex2f(c0.x,c0.y);
				glEnd();
				glBegin(GL_LINES);
				glVertex2f(c1.x,c1.y);
				glVertex2f(c0.x,c0.y);
				glEnd();


				v0 = Vector2(vertices[i+j], vertices[i+j+1]);//Vector2(vertex[j*2], vertex[j*2+1]);
				v1 = Vector2(vertices[i+j+2], vertices[i+j+3]);//Vector2(vertex[(j+1)*2], vertex[(j+1)*2+1]);

				j += 4;

				if (bent == 1)
				{
					c0 = v0;
					c1 = v1;
				} else {
					c0 = v1;
					c1 = v0;
				}

				float dist = (a0-c0).lengthSquared();
				//check if c0 is too far away from A (for too sharp bends and degenerate cases)
				if (dist > brushWidth)
					break;

				//check if c0 is outside the bounding planes of A
				if (sideTest(a0, a1, c0) > 0 || sideTest(a0, (a0+an), c0) > 0)
					break;

				average += c0;
				++totalAvg;
			} // i+j

			//set prevRib to the last rib processed
			prevRib[0] = v0; prevRib[1] = v1;

			//clipping plane B is the last processed rib
			Vector2 b0 = v0; Vector2 b1 = v1;
			Vector2 bn = calcLineNormal(b0, b1);
			brushWidth = (b0-b1).lengthSquared();
			
			//iterate over the ribs before A while they are still on the wrong side of the bounds of B
			int k = 12;
			
			lastPoint = c0;
			
			while (i-k > 0)
			{
				v0 = Vector2(vertices[i-k], vertices[i-k+1]);
				v1 = Vector2(vertices[i-k+2], vertices[i-k+3]);

				

				glPointSize(3);
				glColor3f(0,0.8,0);
				glBegin(GL_POINTS);
				glVertex2f(c0.x,c0.y);
				glEnd();
				glBegin(GL_LINES);
				glVertex2f(c1.x,c1.y);
				glVertex2f(c0.x,c0.y);
				glEnd();

				if (bent == 1)
				{
					c0 = v0;
					c1 = v1;
				} else {
					c0 = v1;
					c1 = v0;
				}

				float dist =  (b0-c0).lengthSquared();
				if (dist > brushWidth)
					break;

				//check if c0 is outside the bounding planes of B
				if (sideTest(b0, b1, c0) > 0 || sideTest(b0, (b0+bn), c0) > 0)
					break;

				k += 4;
				average += c0;
				totalAvg += 1;
			}

			if (k > 12) //we've got something to remodel 
			{
				glPointSize(5);
				glColor3f(0,0,0.8);
				glBegin(GL_POINTS);
				glVertex2f(lastPoint.x,lastPoint.y);
				glEnd();
				//cout<<"remodelling!\n";
				Vector2 vn = calcLineNormal(c0, c1);

				//calculate intersection point
				float lenBx = bn.x; float lenBy = bn.y;

				float lenVx = vn.x; float lenVy = vn.y;

				float m1 = lenVy / lenVx;
				float f1 = c0.y - m1*c0.x;
				float m2 = lenBy / lenBx;
				float f2 = 0;
				if (bent == 1)
					f2 = b0.y - m2*b0.x;
				else
					f2 = b1.y - m2*b1.x;

				float cx = ( f2 - f1 )/( m1 - m2 );
				float cy = m1*cx + f1;

				average /= totalAvg;

				glPointSize(5);
				glColor3f(0,0.8,0.8);
				glBegin(GL_POINTS);
				glVertex2f(average.x,average.y);
				glColor3f(0.8,0,0.8);
				glVertex2f(cx,cy);
				glEnd();

				//calculate angle between intersecting lines
				float angle = acos(vn.x*bn.x+vn.y*bn.y)*TO_DEGREES;
				float maxAngle = 100;

				Vector2 reformPoint = Vector2(cx, cy);
				
				float rDist = (lastPoint-reformPoint).lengthSquared();

				//cout<<"rDist "<<rDist<<" "<<sqrt(brushWidth)<<"\n";
				//if (angle > maxAngle)
				//	reformPoint = average;

				//fix the ribs
				int u = i-k;
				while (u < i+j)
				{
					float v0x = vertices[u];
					float v0y = vertices[u+1];
					float v1x = vertices[u+2];
					float v1y = vertices[u+3];
					if (rDist < 0.25f*brushWidth)
					{
						glColor3f(0.8,0.8,0);
						glBegin(GL_LINES);
						glVertex2f(reformPoint.x,reformPoint.y); //glVertex2f(cx,cy);
						if (bent == 1)
							glVertex2f(v1x,v1y);
						else
							glVertex2f(v0x,v0y);
						glEnd();
					}
					else
					{
						glColor3f(0,0.8,0.8);
						glBegin(GL_LINES);
						glVertex2f(lastPoint.x,lastPoint.y);//glVertex2f(average.x,average.y);
						if (bent == 1)
							glVertex2f(v1x,v1y);
						else
							glVertex2f(v0x,v0y);
						glEnd();
					}
					/*if (bent == 1)
					{
						vertices[u]   = reformPoint.x;
						vertices[u+1] = reformPoint.y;
					}
					else
					{
						vertices[u+2] = reformPoint.x;
						vertices[u+3] = reformPoint.y;
					}*/

					u += 4;
				}

				i += j;

			} //k > 12
		}	
		else
		{
			prevRib[0] = v0;
			prevRib[1] = v1;

		} //bent > 0
	} //for
}

vector<GLfloat> Stroke::genRoundTip(Vector2& pp1, Vector2& pp2, bool reverse)
{
	float v[4];
	Vector2 p1, p2;
	if (reverse) { //begin and end tips are opposites
		p2 = pp1; p1 = pp2;
	} else {
		p1 = pp1; p2 = pp2;
	}
	v[0] = p1.x; v[1] = p1.y; v[2] = p2.x; v[3] = p2.y;
	vector<GLfloat> verts(v,v+4); //init array

	//find midpoint, translate to origin
  Vector2 pO = (p2+p1)*0.5f;
  Vector2 pTrans = p1 - pO;

	float aStep = PI/5;
	float totalAngle = aStep;

	//calculate points on the halfcircle from p1 to p2
	for (int i=1; i<5; i++)
	{
		float xNew = cos(totalAngle)*pTrans.x - sin(totalAngle)*pTrans.y;
		float yNew = sin(totalAngle)*pTrans.x + cos(totalAngle)*pTrans.y;

		xNew += pO.x;
		yNew += pO.y;

		verts.push_back(xNew); verts.push_back(yNew);
		verts.push_back(p2.x); verts.push_back(p2.y);  
		totalAngle += aStep;
	}
	
	return verts;
}

inline void Stroke::getSegmentCPs(uint seg, float dx[], float dy[], float dr[])
{
	//max segment is CPs.size()-2

	if (seg > 0)
	{
		dx[0] = CPs[seg-1].p.x; dy[0] = CPs[seg-1].p.y; dr[0] = CPs[seg-1].r;
	}
	else // 'virtual' cp
	{
		dx[0] = CPs[seg].p.x; dy[0] = CPs[seg].p.y; dr[0] = CPs[seg].r;
	}

	dx[1] = CPs[seg+0].p.x; dy[1] = CPs[seg+0].p.y;	dr[1] = CPs[seg+0].r;
	dx[2] = CPs[seg+1].p.x; dy[2] = CPs[seg+1].p.y;	dr[2] = CPs[seg+1].r;		
	
	if (seg < CPs.size()-2)
	{
		dx[3] = CPs[seg+2].p.x; dy[3] = CPs[seg+2].p.y;	dr[3] = CPs[seg+2].r;
	}
	else //virtual cp
	{
		dx[3] = CPs[seg+1].p.x; dy[3] = CPs[seg+1].p.y;	dr[3] = CPs[seg+1].r;
	}
}

inline void Stroke::getSegmentCPs(uint seg, float dx[], float dy[])
{
	//max segment is CPs.size()-2

	if (seg > 0)
	{
		dx[0] = CPs[seg-1].p.x; dy[0] = CPs[seg-1].p.y;
	}
	else // 'virtual' cp
	{
		dx[0] = CPs[seg].p.x; dy[0] = CPs[seg].p.y;
	}

	dx[1] = CPs[seg+0].p.x; dy[1] = CPs[seg+0].p.y;
	dx[2] = CPs[seg+1].p.x; dy[2] = CPs[seg+1].p.y;		
	
	if (seg < CPs.size()-2)
	{
		dx[3] = CPs[seg+2].p.x; dy[3] = CPs[seg+2].p.y;
	}
	else //virtual cp
	{
		dx[3] = CPs[seg+1].p.x; dy[3] = CPs[seg+1].p.y;
	}
}
