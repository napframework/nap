#pragma once

// Local Includes
#include "blockingconcurrentqueue.h"
#include "utility/dllexport.h"

// External Includes
#include <functional>
#include <thread>
#include <vector>

namespace nap 
{
    /**
     * Thread safe queue for tasks that are encapsulated in function objects.
     * It is possible to enqueue tasks at the end of the queue and to execute the tasks in the queue.
     */
    class NAPAPI TaskQueue 
	{
    public:
        // a task in the queue is a function object
        using Task = std::function<void()>;
        
    public:
        /**
         * Constructor takes maximum number of items that can be in the queue at a time.
         */
        TaskQueue(unsigned int maxQueueItems = 20);
        /**
         * Add a task to the end of the queue.
         */
        void enqueue(Task task) { mQueue.enqueue(task); }
        
        /**
         * If the queue is empty, this function blocks until tasks are enqueued and executes them.
         * If the queue is not empty all the tasks are executed.
         */
        void processBlocking();
        
        /**
         * Executes all tasks currently in the queue
         */
        void process();
        
    private:
        moodycamel::BlockingConcurrentQueue<Task> mQueue;
        std::vector<Task> mDequeuedTasks;
    };
    
    
    /**
     * A single thread that runs its own task queue
     */
    class NAPAPI WorkerThread 
	{
    public:
        /**
         * @blocking: 
         *   true: the threads blocks and waits for enqueued tasks to perform
         *   false: the threads runs through the loop as fast as possible and emits @execute every iteration
         * @maxQueueItems: the maximum number of items in the task queue
         */
        WorkerThread(bool blocking = true, unsigned int maxQueueItems = 20);
        ~WorkerThread();
        
        /**
         * enqueues a task to be performed on this thread
         */
        void enqueue(TaskQueue::Task task) { mTaskQueue.enqueue(task); }
        
        /**
         * Start the thread and the thread loop.
         */
        void start();
        
        /**
         * Stop the thread loop and join the thread.
         */
        void stop();
        
        /**
         * Returns wether the thread is running and not shutting down.
         */
        bool isRunning() { return mRunning; }
        
        /**
         * Overwrite this method to specify behaviour to be executed each loop after processing the task queue.
         */
        virtual void loop() { }
        
    private:
        std::unique_ptr<std::thread> mThread = nullptr;
        std::atomic<bool> mRunning;
        bool mBlocking = true;
        TaskQueue mTaskQueue;
    };
    
    
    /**
     * A pool of threads that can be used to perform multiple tasks at the same time
     */
    class NAPAPI ThreadPool 
	{
    public:
        ThreadPool(unsigned int numberOfThreads = 1, unsigned int maxQueueItems = 20);
        ~ThreadPool();
        
        /**
         * Enqueues a task to be performed on the next idle thread.
         */
        void execute(TaskQueue::Task task) { mTaskQueue.enqueue(task); }
        
        /**
         * Sets stopping to true and joins and exits all threads in the pool.
         */
        void shutDown();
        
        /**
         * Resizes the number of threads in the pool, joins and exits all existing threads first!
         */
        void resize(int numberOfThreads);
        
        /**
         * Returns the number ot threads in the pool.
         */
        int getThreadCount() const { return int(mThreads.size()); }
        
        /**
         * Returns wether this thread is shutting down.
         */
        bool isStopping() const { return (mStop == true); }
        
    private:
        void addThread();
        
        std::vector<std::thread> mThreads;
        std::atomic<bool> mStop;
        TaskQueue mTaskQueue;
    };
}
