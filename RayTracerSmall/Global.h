#pragma once
#include <iostream>
#include "Heap.h"
#include "HeapDirector.h"

struct Header
{
	int size;
	int checkValue;
	Heap* heap;
};

struct Footer
{
	int reserve;
	int checkValue;
};

void* operator new(size_t size);
void* operator new(size_t size, Heap* pHeap);
void operator delete(void* pMem);

