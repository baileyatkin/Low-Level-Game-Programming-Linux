#include "ThreadPool.h"
#include <sstream>
#ifndef _WIN32
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

//This is used with pthread to execute a member function (threadfunc)
void* ExecThreadFunc(void* obj)
{
    return ((ThreadPool*)obj)->ThreadFunc();
}
#endif // !_WIN32


ThreadPool::ThreadPool(unsigned int numThreads, std::mutex* main_mutex)
{
    threadCount = numThreads;
    mainMutex = main_mutex;
    tasks = queue<std::function<void()>>();

#ifdef _WIN32
    //Adds the numThreads threads which loop through the Thread Function
    for (int i = 0; i < numThreads; i++)
    {
        threads.emplace_back(std::thread([this]() { ThreadFunc(); }));
    }
#else
    if (LINUX_POOLING)
    {
        //Initialise the thread vector
        threads = vector<pthread_t>(numThreads);
        for (int i = 0; i < numThreads; i++)
        {
            //Creates pthreads with function execmemberfunction, which calls the thread function
            pthread_create(&threads[i], NULL, &ExecThreadFunc, this);
        }
    }
#endif
}

ThreadPool::~ThreadPool()
{
#ifdef _WIN32
    //Instruct threads to break from the while loop
    stopping = true;
    //Wait for the threads to finish
    for (int i = 0; i < threadCount; i++)
    {
        threads[i].join();
    }
#else
    if (LINUX_POOLING)
    {
        stopping = true;
        for (int i = 0; i < threadCount; i++)
        {
            pthread_join(threads[i], NULL);
        }
    }
#endif // _WIN32
}

void ThreadPool::Enqueue(std::function<void()> task)
{
#ifdef _WIN32
    //Add the task passed through onto the queue and add onto the task count
    tasks.push(task);
    tasksRemaining++;
#else
    if (LINUX_POOLING)
    {
        tasks.push(task);
        tasksRemaining++;
    }
    else
    {
        //If we aren't currently pooling, we will fork instead
        MakeForks(task);
    }
#endif // _WIN32
}

void ThreadPool::WaitUntilCompleted()
{
#ifdef _WIN32
    //A condition variable is created which will lock the main thread until condition is met
    std::unique_lock<std::mutex> lock(*mainMutex);
    cv.wait(lock);
#else
    if (LINUX_POOLING)
    {
        std::unique_lock<std::mutex> lock(*mainMutex);
        cv.wait(lock);
    }
    else
    {
        //Wait for every fork to be finished
        int status;
        while (wait(&status) != -1) {}
    }
#endif // _WIN32
}

void ThreadPool::Lock()
{
    std::unique_lock<std::mutex> localLock(waitMutex);
    lock = std::move(localLock);
}

void ThreadPool::ReleaseLock()
{
    std::unique_lock<std::mutex> localLock = std::move(lock);
    localLock.unlock();
}

void* ThreadPool::ThreadFunc()
{
    // Loop through until told to stop
    while (true)
    {
        if (stopping)
        {
            break;
        }
        Lock(); //Only allow one thread past this point
        if (!tasks.empty())
        {
            // Check if it needs to stop before starting the task, release the lock and break from the while loop if so
            if (stopping)
            {
                ReleaseLock();
                break;
            }

            //Take the task at the front of the queue and remove it from the queue
            std::function<void()> task = tasks.front();
            tasks.pop();

            if (task)
            {
                //Execute task from the queue
                //ReleaseLock();
                task();
                if (--tasksRemaining == 0) //Count down after each task is completed
                {
                    cv.notify_one(); //Unblock the main thread when all tasks are done
                }
                //ReleaseLock();
            }
        }
        else
            ReleaseLock(); // If there are no remaining tasks, then release the lock so others can continue
    }
    return nullptr;
}

#ifndef _WIN32
void ThreadPool::MakeForks(std::function<void()> task)
{
    //New fork is created
    pid_t newFork = fork();
    if (newFork == 0) //Child process
    {
        if (task)
        {
            task();
        }
        exit(0);
    }
    else if (newFork < 0) //If the fork has failed then:
    {
        std::cout << "Error: Could not create fork!\n";
    }
}
#endif // !_WIN32