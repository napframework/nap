#include "threading.h"

namespace nap {
    
    
    void TaskQueue::processBlocking()
    {
        queue.wait_dequeue(nextTask);
        nextTask();
    }
    
    
    void TaskQueue::process()
    {
        queue.try_dequeue(nextTask);
        if (nextTask)
            nextTask();
    }
    
    
    WorkerThread::WorkerThread(unsigned int maxQueueItems) : TaskQueue(maxQueueItems)
    {
        stop = false;
        thread = std::make_unique<std::thread>([&](){
            while (!stop)
                processBlocking();
        });
        
#ifdef UNIX
        sched_param schedParams;
        schedParams.sched_priority = 99;
        pthread_setschedparam(thread->native_handle(), SCHED_RR, &schedParams);
#endif
    }
    
    
    WorkerThread::~WorkerThread()
    {
        stop = true;
        
        // enqueue empty function for thread to make it stop blocking
        enqueue([](){});
        
        thread->join();
    }
    
    
    ThreadPool::ThreadPool(unsigned int numberOfThreads, unsigned int maxQueueItems) : TaskQueue(maxQueueItems)
    {
        stop = false;
        for (unsigned int i = 0; i < numberOfThreads; ++i)
            addThread();
    }
    
    
    ThreadPool::~ThreadPool()
    {
        shutDown();
    }
    
    
    void ThreadPool::shutDown()
    {
        stop = true;
        
        // enqueue empty function for each thread to make it stop blocking
        for (unsigned int i = 0; i < threads.size(); ++i)
            enqueue([](){});
        
        for (auto& thread : threads)
            thread.join();
        
        threads.clear();
    }
    
    
    void ThreadPool::resize(int numberOfThreads)
    {
        shutDown();
        
        stop = false;
        for (auto i = 0; i < numberOfThreads; ++i)
            addThread();
    }
    
    
    void ThreadPool::setScheduling(int aPolicy, int aPriority)
    {
        for (auto& th : threads)
            setThreadScheduling(th);
    }
    
    
    void ThreadPool::addThread()
    {
        threads.emplace_back([&](){
            while (!stop)
                processBlocking();
        });
        
        setThreadScheduling(threads.back());
    }
    
    
    void ThreadPool::setThreadScheduling(std::thread& th)
    {
#ifdef UNIX
        sched_param schedParams;
        schedParams.sched_priority = 99;
        pthread_setschedparam(th.native_handle(), SCHED_RR, &schedParams);
#endif
    }
    
    
    
    
    
}
