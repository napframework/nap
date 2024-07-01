/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "readerwriterqueue.h"
#include "blockingconcurrentqueue.h"

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
    class TaskQueue final
	{
    public:
        // a task in the queue is a function object
        using Task = std::function<void()>;

    public:
        /**
         * Constructor takes maximum number of items that can be in the queue at a time.
         */
        TaskQueue(int maxQueueItems = 20);
        /**
         * Add a task to the end of the queue.
         */
        bool enqueue(Task task) { return mQueue.enqueue(task); }

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
    class WorkerThread
	{
    public:
		/**
		 * Explicit default constructor
		 */
		WorkerThread();

		/**
         * @param blocking when true: the threads blocks and waits for enqueued tasks to perform, false: the threads runs through the loop as fast as possible and emits 'execute' every iteration
         * @param maxQueueItems the maximum number of items in the task queue
         */
        WorkerThread(bool blocking, int maxQueueItems = 20);
		virtual ~WorkerThread();

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
    class ThreadPool final
	{
    public:
        ThreadPool(int numberOfThreads = 1, int maxQueueItems = 20, bool realTimePriority = false);
        ~ThreadPool();

        /**
         * Enqueues a task to be performed on the next idle thread.
         */
        void execute(TaskQueue::Task task) {
            assert(task != nullptr);
            mTaskQueue.enqueue(task);
        }

        /**
         * Sets stopping to true and joins and exits all threads in the pool.
         */
        void shutDown();

        /**
         * Resizes the number of threads in the pool, joins and exits all existing threads first!
         */
        void resize(int numberOfThreads);

        /**
         * Returns the number of threads in the pool.
         */
        int getThreadCount() const { return int(mThreads.size()); }

		/**
		 * Returns an estimate of the total number of tasks currently in the queue. This
		 * estimate is only accurate if the queue has completely stabilized before it is called
		 * (i.e. all enqueue and dequeue operations have completed and their memory effects are
		 * visible on the calling thread, and no further operations start while this method is
		 * being called).
		 * Thread-safe.
		 */
		int getTaskCount() const { return mTaskQueue.size_approx(); }

        /**
         * Returns whether this thread is shutting down.
         */
        bool isStopping() const { return (mStop == true); }

    private:
        void addThread();

        std::vector<std::thread> mThreads;
        std::atomic<bool> mStop;
        moodycamel::BlockingConcurrentQueue<TaskQueue::Task> mTaskQueue;
        bool mRealTimePriority = false;
    };
}
