#pragma once

#include <thread>
#include <condition_variable>

namespace nap
{
    
    namespace audio
    {
    
        class AsyncObserver
        {
        private:
            std::condition_variable condition;
            bool notified;
            std::mutex m;
            
            uint32_t numberOfNotifications;
            uint32_t notificationCounter;
            
        public:
            AsyncObserver()
            {
                notified = false;
                numberOfNotifications = 0;
                notificationCounter = 0;
            }
            
            ~AsyncObserver() = default;
            
            void setBarrier(uint32_t aNumberOfNotifications = 1)
            {
                numberOfNotifications = aNumberOfNotifications;
                if (numberOfNotifications == 0)
                    notified = true;
            }
            
            void notifyBarrier()
            {
                std::unique_lock<std::mutex> Lock(m);
                if (++notificationCounter >= numberOfNotifications)
                {
                    notified = true;
                    condition.notify_one();
                }
            }
            
            void waitForNotifications()
            {
                if (!notified)
                {
                    std::unique_lock<std::mutex> lock(m);
                    condition.wait(lock, [&](){ return notified; });
                }
                
                notificationCounter = 0;
                notified = false;
            }
            
            void notifyOne()
            {
                std::unique_lock<std::mutex> Lock(m);
                notified = true;
                condition.notify_one();
            }
            
            void notifyAll()
            {
                std::unique_lock<std::mutex> Lock(m);
                notified = true;
                condition.notify_all();
            }
            
        };
        
    }
    
}
