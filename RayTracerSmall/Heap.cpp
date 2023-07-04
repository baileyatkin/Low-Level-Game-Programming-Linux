#include "Heap.h"

Heap::Heap(const char* name)
{
	m_name = name;
}

Heap::~Heap()
{
	delete(m_name);
}

void Heap::Allocate(size_t size)
{
	allocatedBytes += size;
}

void Heap::DelAllocation(size_t size)
{
	allocatedBytes -= size;
}