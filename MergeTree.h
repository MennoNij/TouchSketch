#pragma once

#include <vector>
#include <iostream>

#include "Vector2.h"

#define THRES_CONST 1.3f

using std::vector;

// A node of the mergetree
struct MergeNode
{
	Vector2 v0;
	Vector2 v1;
	unsigned int parent;
	vector<unsigned int> children;
	float t;
	float thres;

	MergeNode(Vector2 vv0, Vector2 vv1, float t, float thres) : v0(vv0), v1(vv1)
	{
		this->parent = parent = 0;
		this->t = t;
		this->thres = thres;
		this->children.clear();
	}
};

/*
	The mergetree contains the order of vertex collapses of a stroke segment
*/

class MergeTree
{
public:
	vector<MergeNode> treeList; //list of all the nodes in the tree

	MergeTree();
	~MergeTree();

	void addNode(MergeNode& node);
	vector<MergeNode*> traverseDepth(float threshold);
	MergeNode* getRoot();
	void clear() { treeList.clear(); }

	void printIt();
};