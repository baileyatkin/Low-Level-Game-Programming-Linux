#include "Global.h"
#include <cassert>

int checkValueHeader = 0xDEADC0DE;
int checkValFooter = 0XDEADBEEF;

void* operator new(size_t size)
{
	return ::operator new(size, HeapDirector::GetDefaultHeap());
}

void* operator new(size_t size, Heap* pHeap)
{
	//std::cout << "New function called" << std::endl;
	size_t nRequestedBytes = size + sizeof(Header) + sizeof(Footer);
	char* pMem = (char*)malloc(nRequestedBytes);
	Header* pHeader = (Header*)pMem;

	pHeader->size = size;
	pHeader->heap = pHeap;
	pHeader->checkValue = checkValueHeader;

	void* pFooterAddr = pMem + sizeof(Header) + size;
	Footer* pFooter = (Footer*)pFooterAddr;
	pFooter->checkValue = checkValFooter;

	//std::cout << pHeader->size << std::endl;
	pHeader->heap->Allocate(pHeader->size + sizeof(pHeader) + sizeof(pFooter));

	void* pStartMemoryBlock = pMem + sizeof(Header);
	return pStartMemoryBlock;
}

void operator delete(void* pMem)
{
	//std::cout << "Delete function called" << std::endl;
	Header* pHeader = (Header*)((char*)pMem - sizeof(Header));
	assert(pHeader->checkValue == checkValueHeader);

	Footer* pFooter = (Footer*)((char*)pMem + pHeader->size);
	assert(pFooter->checkValue == checkValFooter);

	//std::cout << pHeader->size << std::endl;
	pHeader->heap->DelAllocation(pHeader->size + sizeof(pHeader) + sizeof(pFooter));

	free(pHeader);
}