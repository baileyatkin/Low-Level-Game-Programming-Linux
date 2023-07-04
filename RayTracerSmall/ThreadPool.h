#pragma once

#include <condition_variable>
#include <functional>
#include <vector>
#include <queue>
#include <mutex>
#include <iostream>
#include "Global.h"
#ifdef _WIN32
#include <thread>
#else
#include <pthread.h>
#endif // !_WIN32

using std::vector;
using std::queue;

#define LINUX_POOLING true

class ThreadPool
{
public:
	ThreadPool(unsigned int numThreads, std::mutex* main_mutex);
	~ThreadPool();

	int GetSize() { return threadCount; }

	void Lock();
	void ReleaseLock();

	queue<std::function<void()>> GetTasks() { return tasks; }

	void Enqueue(std::function<void()> task);

	void WaitUntilCompleted();

	void* ThreadFunc();

private:
#ifdef _WIN32
	vector<std::thread> threads;
#else
	vector<pthread_t> threads;

	void MakeForks(std::function<void()> task);
#endif // _WIN32
	queue<std::function<void()>> tasks;
	int tasksRemaining = 0;
	int threadCount;

	std::mutex(waitMutex);
	std::mutex* mainMutex;
	std::condition_variable cv;

	std::unique_lock<std::mutex> lock;
	bool stopping = false;
};