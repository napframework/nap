/*
  ==============================================================================

    TimedQueue.h
    Created: 2 Nov 2015 1:48:44pm
    Author:  Stijn van Beek

  ==============================================================================
*/

#ifndef TIMEDQUEUE_H_INCLUDED
#define TIMEDQUEUE_H_INCLUDED

#include <cassert>
#include <functional>
#include <atomic>
#include <vector>
#include <blockingconcurrentqueue.h>

namespace lib {
    
    class ThreadPool
    {
    public:
        ThreadPool(unsigned int numberOfThreads = 1, unsigned int maxQueueItems = 20) :
			functionQueue(maxQueueItems)
        {
            stop = false;
            for (unsigned int i = 0; i < numberOfThreads; ++i)
                addThread();
        }
        
        
        ~ThreadPool()
        {
            shutDown();
        }
        
        
        void enqueue(std::function<void()> func)
        {
			functionQueue.enqueue(func);
        }
        
        
        void shutDown()
        {
            stop = true;
            
            // enqueue empty function for each thread to make it stop blocking
            for (unsigned int i = 0; i < threads.size(); ++i)
				functionQueue.enqueue([](){});
            
            for (auto& thread : threads)
                thread.join();
            
            threads.clear();
        }
        
        
        void resize(int numberOfThreads)
        {
            shutDown();
            
            stop = false;
            for (auto i = 0; i < numberOfThreads; ++i)
                addThread();
        }
        
        
        void setScheduling(int aPolicy, int aPriority)
        {
            for (auto& th : threads)
                setThreadScheduling(th);
        }
        
        
        int getThreadCount() const { return threads.size(); }
                
    private:
        void addThread()
        {
            threads.emplace_back([&](){
                std::function<void()> func;
                while (!stop)
                {
					functionQueue.wait_dequeue(func);
					func();
                }
            });
            
            setThreadScheduling(threads.back());
        }
        
        
        void setThreadScheduling(std::thread& th)
        {
//            sched_param schedParams;
//            schedParams.sched_priority = priority;
//            pthread_setschedparam(th.native_handle(), policy, &schedParams);
        }
        
        
        moodycamel::BlockingConcurrentQueue<std::function<void()>> functionQueue;
        std::vector<std::thread> threads;
        std::atomic<bool> stop;
//        int policy = SCHED_RR;
//        int priority = 99;
    };
    
    
}



#endif  // TIMEDQUEUE_H_INCLUDED
