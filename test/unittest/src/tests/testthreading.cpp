#include "testthreading.h"

#include "../testmanager.h"

#include <nap.h>

#include <Lib/Utility/Threading/ThreadPool.h>
#include <Lib/Utility/Threading/AsyncObserver.h>
#include <Lib/Utility/Scheduler/Scheduler.h>


bool testAsyncObserver()
{
    lib::ThreadPool threadPool(4);
    lib::AsyncObserver observer;
    const int numberOfProcesses = 4;
    auto dataSize = 10;

    std::array<std::vector<int>, numberOfProcesses> data;

    for (auto testCounter = 0; testCounter < 100000; ++testCounter)
    {
        observer.setBarrier(numberOfProcesses);
        for (auto i = 0; i < numberOfProcesses; ++i)
        {
            threadPool.enqueue([&, i](){
                // add some data
                for (auto j = 0; j < dataSize; ++j)
                    data[i].emplace_back(i);
                // notify
                observer.notifyBarrier();
            });
        }

        observer.waitForNotifications();

        // check data
        for (auto i = 0; i < numberOfProcesses; ++i)
        {
            TEST_ASSERT(data[i].size() == dataSize, "Data sze mismatch");
            data[i].clear();
        }
    }
    return true;
}


bool testScheduler()
{
    lib::Scheduler scheduler;
    lib::ThreadPool threadPool(4);
    int numberOfTests = 100;
    std::atomic<int> counter;
    counter = 0;

    threadPool.enqueue([&](){
        for (auto i = 0; i < numberOfTests; ++i)
        {
            counter++;
            nap::Logger::debug(std::to_string(counter));

            scheduler.schedule(i, [&](){
                counter--;
                nap::Logger::debug(std::to_string(counter));
            });
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(1ms);
        }
    });

    for (auto i = 0; i < numberOfTests * 2; ++i)
    {
        for (auto i = 0; i < scheduler.getSamplesPerMilliseconds(); ++i)
            scheduler.process();
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1ms);
    }

    nap::Logger::debug(std::to_string(counter));
    return true;
}