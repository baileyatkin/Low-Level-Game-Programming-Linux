#pragma once

#ifdef _WIN32
#else
#include <stdlib.h>
#endif

class Heap
{
public:
	Heap(char* name);
	Heap(const char* name);
	~Heap();
	void Allocate(size_t size);
	void DelAllocation(size_t size);
	size_t GetSize() { return allocatedBytes; }
	const char* GetName() { return m_name; }

private:
	size_t allocatedBytes = 0;
	const char* m_name = "Heap";
};

