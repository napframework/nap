#pragma once


#include <functional>
#include <thread>
#include <vector>
#include <nap/blockingconcurrentqueue.h>

namespace nap {
    
    /**
     * 
     *
     */
    class TaskQueue {
    public:
        using Task = std::function<void()>;
        
    public:
        TaskQueue(unsigned int maxQueueItems = 20) : queue(maxQueueItems) { }
        void enqueue(Task task) { queue.enqueue(task); }
        
        void processBlocking();
        void process();
        
    private:
        moodycamel::BlockingConcurrentQueue<Task> queue;
        Task nextTask = nullptr;
    };
    
    
    class WorkerThread : public TaskQueue {
    public:
        WorkerThread(unsigned int maxQueueItems = 20);
        ~WorkerThread();
        
    private:
        std::unique_ptr<std::thread> thread = nullptr;
        std::atomic<bool> stop;
    };
    
    
    class ThreadPool : public TaskQueue {
    public:
        ThreadPool(unsigned int numberOfThreads = 1, unsigned int maxQueueItems = 20);
        ~ThreadPool();
        
        void shutDown();
        void resize(int numberOfThreads);
        void setScheduling(int aPolicy, int aPriority);
        
        int getThreadCount() const { return int(threads.size()); }
        bool isStopping() const { return (stop == true); }
        
    private:
        void addThread();
        
        void setThreadScheduling(std::thread& th);
        
        std::vector<std::thread> threads;
        std::atomic<bool> stop;
    };
    
    
}
