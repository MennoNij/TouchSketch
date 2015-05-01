#include "StrokeList.h"

#include <iostream>

ListNode::ListNode(Stroke* stroke, ListNode* pr, ListNode* ne)
{
	item = stroke;
	item->entry = this;
	prev = pr;
	next = ne;
}

ListNode::~ListNode() {}

int ListNode::erase()
{
	if (prev == NULL) //front of list
		return 1;
	if (next == NULL) //back of list
		return 2;

	//delete item;
	item = NULL;
	prev->next = next;
	next->prev = prev;

	return 0;
}

void ListNode::erase_item()
{
	if (item != NULL)
		delete item;
	item = NULL;
}

StrokeList::StrokeList()
{
	front = NULL;
	back = NULL;
	sze = 0;
}

StrokeList::~StrokeList()
{
	ListNode* cur = front;
	for (uint i=0; i<sze; ++i)
	{
		ListNode* nCur = cur->next;
		cur->erase_item();
		delete cur;

		cur = nCur;
	}
}

void StrokeList::clear()
{
	ListNode* cur = front;
	for (uint i=0; i<sze; ++i)
	{
		if (cur != NULL)
		{
			ListNode* nCur = cur->next;
			cur->erase_item();
			delete cur;

			cur = nCur;
		}
	}
	sze = 0;
}

void StrokeList::remove(uint indx)
{
	ListNode* cur = front;
	for (uint i=0; i < indx; ++i)
		cur = cur->next;
	
	//cur now contains the node/element to be removed
	int result = cur->erase();
	if (result == 1)
		pop_front();
	else if (result == 2)
		pop_back();
	else
	{
		delete cur;
		cur = NULL;

		--sze;
	}
}

void StrokeList::remove(ListNode* node)
{
	int result = node->erase();
	if (result == 1)
		pop_front();
	else if (result == 2)
		pop_back();
	else
	{
		delete node;
		node = NULL;

		--sze;
	}
}
/*
void StrokeList::insert(uint indx, Stroke* item)
{
	ListNode* cur = front;
	for (uint i=0; i < indx; ++i)
		cur = cur->next;
	//cur contains the node after which we insert item
	ListNode* nNext = cur->next;
	ListNode* nPrev = cur;
	ListNode* newNode = new ListNode(item, nPrev, nNext);
	//fix pointers
	nNext->prev = newNode;
	nPrev->next = newNode;

	++sze;
}
*/
bool StrokeList::empty()
{
	if (sze > 0) return false;
	return true;
}

uint StrokeList::size()
{
	return sze;
}

void StrokeList::push_front(Stroke* item)
{
	if (sze == 0)
	{
		front = new ListNode(item, NULL, NULL);
		back = front;
	}
	else
	{
		ListNode* newNode = new ListNode(item, NULL, front);
		front->prev = newNode;
		front = newNode;
	}
	++sze;
}

void StrokeList::push_back(Stroke* item)
{
	if (sze == 0)
	{
		back = new ListNode(item, NULL, NULL);
		front = back;
	}
	else
	{
		ListNode* newNode = new ListNode(item, back, NULL);
		back->next = newNode;
		back = newNode;
	}
	++sze;
}

void StrokeList::pop_front()
{
	if (sze > 1)
	{
		ListNode* newFront = front->next;
		newFront->prev = NULL;
		
		//front->erase_item();
		delete front;
		front = newFront;
	}
	else //just 1 element
	{
		//front->erase_item();
		delete front;
		front = NULL;
		back = NULL;
	}
	--sze;
}

void StrokeList::pop_back()
{
	if (sze > 1)
	{
		ListNode* newBack = back->prev;
		newBack->next = NULL;
		
		//back->erase_item();
		delete back;
		back = newBack;
	}
	else //just 1 element
	{
		//back->erase_item();
		delete back;
		back = NULL;
		front = NULL;
	}
	--sze;
}

Stroke* StrokeList::get_next(ListNode*& node)
{
	if (node->next == NULL)
		return NULL;

	node = node->next;
	return node->item;
}

Stroke* StrokeList::get_front()
{
	if (front != NULL)
		return front->item;
	return NULL;
}