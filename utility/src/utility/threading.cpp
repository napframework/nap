/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "threading.h"

#ifdef _WIN32
#include <windows.h>
#endif

#include <xmmintrin.h>

namespace nap
{
    
    TaskQueue::TaskQueue(int maxQueueItems) : mQueue(maxQueueItems)
    {
        mDequeuedTasks.resize(maxQueueItems);
    }
    
    
    
    void TaskQueue::processBlocking()
    {
        TaskQueue::Task dequeuedTask;
        mQueue.wait_dequeue(dequeuedTask);
        dequeuedTask();
    }
    
    
    void TaskQueue::process()
    {
		Task task;
		while (mQueue.try_dequeue(task))
			task();
    }


	WorkerThread::WorkerThread() : mBlocking(true), mTaskQueue(20)
	{
		mRunning = false;
	}
    
    
    WorkerThread::WorkerThread(bool blocking, int maxQueueItems) : mBlocking(blocking), mTaskQueue(maxQueueItems)
    {
        mRunning = false;
    }
    
    
    WorkerThread::~WorkerThread()
    {
        stop();
    }
    
    
    void WorkerThread::start()
    {
        if (mRunning)
            return;
        
        mRunning = true;
        
        if (mBlocking)
        {
            mThread = std::make_unique<std::thread>([&](){
                while (mRunning)
                    mTaskQueue.processBlocking();
            });
        }
        else {
            mThread = std::make_unique<std::thread>([&](){
                while (mRunning) {
                    mTaskQueue.process();
                    loop();
                }
            });
        }
    }
    
    
    void WorkerThread::stop()
    {
        if (!mRunning)
            return;
        
        mRunning = false;
        
        // enqueue empty function for thread to make it stop blocking
        mTaskQueue.enqueue([](){});
        
        mThread->join();
    }
    
    
    ThreadPool::ThreadPool(int numberOfThreads, int maxQueueItems, bool realTimePriority)
        : mTaskQueue(maxQueueItems), mRealTimePriority(realTimePriority)
    {
        mStop = false;
        for (unsigned int i = 0; i < numberOfThreads; ++i)
            addThread();
    }
    
    
    ThreadPool::~ThreadPool()
    {
        shutDown();
    }
    
    
    void ThreadPool::shutDown()
    {
        mStop = true;
        
        // enqueue empty function for each thread to make it stop blocking
        for (unsigned int i = 0; i < mThreads.size(); ++i)
            mTaskQueue.enqueue([](){});
        
        for (auto& thread : mThreads)
            thread.join();
        
        mThreads.clear();
    }
    
    
    void ThreadPool::resize(int numberOfThreads)
    {
        shutDown();
        
        mStop = false;
        for (auto i = 0; i < numberOfThreads; ++i)
            addThread();
    }
    
    
    void ThreadPool::addThread()
    {
        mThreads.emplace_back([&](){
		  int oldMXCSR = _mm_getcsr();
		  int newMXCSR = oldMXCSR | 0x8040;
		  _mm_setcsr( newMXCSR);

		  TaskQueue::Task dequeuedTask;
            while (!mStop)
			{
				mTaskQueue.wait_dequeue(dequeuedTask);
				dequeuedTask();
			}
        });
        auto& thread = mThreads.back();
        
        if (mRealTimePriority)
        {
#ifdef _WIN32
            auto result = SetThreadPriority(thread.native_handle(), THREAD_PRIORITY_TIME_CRITICAL);
            // If this assertion fails the thread failed to acquire realtime priority
            assert(result != 0);
#else
            sched_param schedParams;
            auto priority = sched_get_priority_max(SCHED_FIFO);
            
            schedParams.sched_priority = priority;
            auto result = pthread_setschedparam(thread.native_handle(), SCHED_FIFO, &schedParams);
            // If this assertion fails the thread failed to acquire realtime priority
            assert(result == 0);
#endif
        }
    }
    
    
}
