#include "MergeTree.h"

#define STACKSIZE 1000
#define uint unsigned int

MergeTree::MergeTree()
{
}

MergeTree::~MergeTree()
{
}

void MergeTree::addNode(MergeNode& node)
{
	if (treeList.empty()) //easy!
	{
		treeList.push_back(node);
	}
	else //propagate node through the tree
	{
		uint curNode = 0;
		uint childPointer = 0;
		bool doWhile = true;

		while(doWhile)
		{
			vector<uint>& curChildren = treeList[curNode].children;
			
			uint numChildren = curChildren.size();
			if (numChildren > 0)
			{
				bool foundChild = false;
				//traverse children; compare t values

				std::vector<uint>::const_iterator child;
				for(child=curChildren.begin(); child!=curChildren.end(); child++){
					if (treeList[(*child)].t < node.t)
					{
						//we've found a mergeNode to the right of the new node
						curNode = (*child);
						foundChild = true;
						break; //the for loop
					}
				}
				if (!foundChild)
				{
					//make the new node a child of the current node
					curChildren.push_back( treeList.size() ); //index of new node
					node.parent = curNode;
					treeList.push_back(node);
					doWhile = false;
				}
			}
			else //we've found a leaf, add node as new child of curNode
			{
				curChildren.push_back( treeList.size() );
				node.parent = curNode;
				treeList.push_back(node);
				doWhile = false;
			}
		}//while
	}
}

vector<MergeNode*> MergeTree::traverseDepth(float scale)
{
		int nodeID = 0;
		int childID = treeList[nodeID].children.size() - 1;
    vector<MergeNode*> nodeList;
		vector<uint> nodeIDstack; vector<uint> childIDstack;
		nodeIDstack.reserve(STACKSIZE); childIDstack.reserve(STACKSIZE); //hope 1000 places is enough...

		nodeIDstack.push_back(-1); childIDstack.push_back(0); //init stack

		while(!nodeIDstack.empty())
		{
			//traverse parent
			if (childID >= 0) //still children left to traverse
			{
				int evalChild = treeList[nodeID].children[childID];
				//compare the threshold to see if this child will be displayed
				if (treeList[evalChild].thres >= (THRES_CONST/scale) )
				{
					//put this parent node / next child combo on the stack, continue with the branch of this child
					nodeIDstack.push_back(nodeID); childIDstack.push_back(childID-1);
					
					nodeID = evalChild;
					childID = treeList[evalChild].children.size() - 1;
					nodeList.push_back(&treeList[nodeID]);
				}
				else
				{
					//goto the next child
					childID = childID - 1;
				}
			}
			else //all children of this branch have been handled, return to the branch of its parent
			{
				nodeID = nodeIDstack.back(); childID = childIDstack.back();
				nodeIDstack.pop_back(); childIDstack.pop_back();
			}
		}
		nodeList.push_back(&treeList[0]); //always add the rootnode
		return nodeList;
}

MergeNode* MergeTree::getRoot()
{
	return &treeList[0];
}

void MergeTree::printIt()
{
	for (uint i=0; i<treeList.size();i++)
	{
		std::cout <<"skip  # "<<i<<"\n";
		std::cout <<"skip v0 "<<treeList[i].v0.x<<","<<treeList[i].v0.y<<" v1 "<<treeList[i].v1.x<<","<<treeList[i].v1.y<<"\n";
		std::cout <<"skip rib len "<< (treeList[i].v0-treeList[i].v1).length()<<"\n";
		std::cout <<"skip  p "<<treeList[i].parent<<"\n";
		std::cout <<"skip  t "<<treeList[i].t<<"\n";
		std::cout <<"-------\n";
	}
}
