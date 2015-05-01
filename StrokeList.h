#pragma once

/*
	Container class for references to all the drawn strokes
*/

#include "Stroke.h"

#define uint unsigned int

class Stroke;

struct ListNode
{
	Stroke* item;
	ListNode* prev;
	ListNode* next;

	ListNode(Stroke* stroke, ListNode* pr, ListNode* ne);
	~ListNode();

	int  erase();
	void erase_item();
};

class StrokeList
{
public:
	ListNode* front;
	ListNode* back;
	uint sze;

	StrokeList();
	~StrokeList();
	
	void clear();
	void remove(uint indx);
	void remove(ListNode* node);
	//void insert(uint indx, Stroke* item);
	bool empty();
	uint size();
	void push_front(Stroke* item);
	void push_back(Stroke* item);
	void pop_front();
	void pop_back();
	Stroke* get_next(ListNode*& node);
	Stroke* get_front();
};