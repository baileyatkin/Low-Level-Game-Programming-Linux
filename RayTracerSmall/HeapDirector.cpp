#include "HeapDirector.h"
#include "Heap.h"
#include <iostream>
#include <vector>
#include <sstream>

std::vector<Heap*> HeapDirector::heaps;
Heap* HeapDirector::m_defaultHeap;

HeapDirector::HeapDirector()
{

}

HeapDirector::~HeapDirector()
{
	//delete everything in the heaps vector
	heaps.clear();
}

Heap* HeapDirector::CreateHeap(const char* name)
{
	//If a heap already exists, a new one isn't created
	Heap* heap = GetHeap(name);
	if (heap == nullptr);
	{
		//create a new heap and push it onto the heaps vector and output that the heap has been created
		heap = new Heap(name);
		heaps.push_back(heap);
		std::stringstream outputMsg;
		outputMsg << "Heap created: " << name << std::endl;
		std::cout << outputMsg.str();
		//std::cout << "Heap created: " << name << std::endl;
	}
	return heap;
}

void HeapDirector::CreateDefaultHeap()
{
	if (m_defaultHeap == nullptr)
	{
		m_defaultHeap = (Heap*)malloc(sizeof(Heap));
		std::stringstream outputMsg;
		outputMsg << "Default Heap created: " << std::endl;
		std::cout << outputMsg.str();
		//std::cout << "Default heap created" << std::endl;
	}
}

Heap* HeapDirector::GetHeap(const char* name)
{
	std::stringstream outputMsg;
	//iterate through all heaps
	for (int i = 0; i < heaps.size(); i++)
	{
		//check if the current heap exists
		if (heaps[i] != NULL)
		{
			//if the current heap name matches the one searched for
			if (heaps[i]->GetName() == name)
			{
				//output that the heap has been found and return the selected heap
				outputMsg << "Found Heap: " << name << std::endl;
				std::cout << outputMsg.str();
				return heaps[i];
			}
		}
	}
	//if the heap hasn't been found then return a null pointer
	outputMsg << "Heap: " << name << " has not been found" << std::endl;
	std::cout << outputMsg.str();
	return nullptr;
}

Heap* HeapDirector::GetDefaultHeap()
{
	//if the default heap doesn't exist then create it
	if (m_defaultHeap == nullptr)
	{
		CreateDefaultHeap();
	}
	//return the default heap
	return m_defaultHeap;
}