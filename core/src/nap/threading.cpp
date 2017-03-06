#include "threading.h"

namespace nap {
    
    // utility to adjust thread scheduling to round-robin with highest priority, unix pthread only
    void setThreadScheduling(std::thread& th)
    {
#ifdef UNIX
        sched_param schedParams;
        schedParams.sched_priority = 99;
        pthread_setschedparam(th.native_handle(), SCHED_RR, &schedParams);
#endif
    }
    
    
    TaskQueue::TaskQueue(unsigned int maxQueueItems) : queue(maxQueueItems)
    {
        dequeuedTasks.resize(maxQueueItems);
    }
    
    
    
    void TaskQueue::processBlocking()
    {
        auto it = dequeuedTasks.begin();
        auto count = queue.wait_dequeue_bulk(it, dequeuedTasks.size());
        for (auto i = 0; i < count; ++i)
            (*it++)();
    }
    
    
    void TaskQueue::process()
    {
        auto it = dequeuedTasks.begin();
        auto count = queue.try_dequeue_bulk(it, dequeuedTasks.size());
        for (auto i = 0; i < count; ++i)
            (*it++)();
    }
    
    
    WorkerThread::WorkerThread(unsigned int maxQueueItems) : taskQueue(maxQueueItems)
    {
        stop = false;
        thread = std::make_unique<std::thread>([&](){
            while (!stop)
                taskQueue.processBlocking();
        });
        
        setThreadScheduling(*thread);
    }
    
    
    WorkerThread::~WorkerThread()
    {
        stop = true;
        
        // enqueue empty function for thread to make it stop blocking
        taskQueue.enqueue([](){});
        
        thread->join();
    }
    
    
    ThreadPool::ThreadPool(unsigned int numberOfThreads, unsigned int maxQueueItems) : taskQueue(maxQueueItems)
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
            taskQueue.enqueue([](){});
        
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
    
    
    void ThreadPool::addThread()
    {
        threads.emplace_back([&](){
            while (!stop)
                taskQueue.processBlocking();
        });
        
        setThreadScheduling(threads.back());
    }
    
    
}
