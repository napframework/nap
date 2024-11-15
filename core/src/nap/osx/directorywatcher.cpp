/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include <nap/directorywatcher.h>
#include <utility/fileutils.h>

// External Includes
#include <assert.h>
#include <CoreServices/CoreServices.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <mach-o/dyld.h>
#include <sys/syslimits.h>
#include <iostream>
#include <mutex>
#include <thread>
#include <filesystem>
#include <unistd.h>

namespace nap {
    
    struct CallbackInfo {
        // the files that have been modified and not yet reported to the user of the system
        std::vector<std::string> modifiedFiles;
        
        // Mutex to lock the modifiedFiles
        std::mutex watchMutex;
    };
    
    /**
     * Internal data container to hide internals from the header.
     */
    struct DirectoryWatcher::PImpl
    {
        FSEventStreamContext context; // could put stream-specific data here.
        CFAbsoluteTime latency = 0.01; // Latency in seconds
        std::string currentPath; // path to the current working directory
        
        CFArrayRef pathsToWatch;
        FSEventStreamRef stream;
        CFRunLoopRef runLoop;
        std::unique_ptr<std::thread> watchThread = nullptr;
        
        CallbackInfo callbackInfo;
    };
    

    
    /**
     * Callback called by the event stream to handle file change events
     */
    
    void scanCallback(ConstFSEventStreamRef streamRef,
                      void *clientCallBackInfo,
                      size_t numEvents,
                      void *eventPaths,
                      const FSEventStreamEventFlags eventFlags[],
                      const FSEventStreamEventId eventIds[])
    {
        CallbackInfo* info = (CallbackInfo*)(clientCallBackInfo);
        char **paths = (char**)eventPaths;
        
        {
            std::unique_lock<std::mutex> lock(info->watchMutex);
            
            for (auto i = 0; i < numEvents; i++) {
                info->modifiedFiles.emplace_back(std::string(paths[i]));
            }
        }
    }
    
    
    /**
     * Deleter operator definition
     */
    void DirectoryWatcher::PImpl_deleter::operator()(DirectoryWatcher::PImpl*ptr) const { delete ptr; }

    
	/**
	* Installs monitor: opens directory, creates event, starts directory scan.
	*/
	DirectoryWatcher::DirectoryWatcher(const std::string& directory)
	{
        mPImpl = std::unique_ptr<PImpl, PImpl_deleter>(new PImpl);
        
        mPImpl->watchThread = std::make_unique<std::thread>([&](){
            
            // retain and release have to be set to NULL explicitly otherwise this causes irregular crashes
            mPImpl->context.version = 0;
            mPImpl->context.info = (void*)(&mPImpl->callbackInfo);
            mPImpl->context.retain = NULL;
            mPImpl->context.release = NULL;
            mPImpl->context.copyDescription = NULL;
            
            // retrieve the path to the current working dir
			char buffer[PATH_MAX];
			mPImpl->currentPath = directory;		
            CFStringRef pathToWatchCF = CFStringCreateWithCString(NULL, mPImpl->currentPath.c_str(), kCFStringEncodingUTF8);
            mPImpl->pathsToWatch = CFArrayCreate(NULL, (const void **)&pathToWatchCF, 1, NULL);
            
            // create event stream for file change event
            mPImpl->stream = FSEventStreamCreate(NULL,
                                                 &scanCallback,
                                                 &mPImpl->context,
                                                 mPImpl->pathsToWatch,
                                                 kFSEventStreamEventIdSinceNow,
                                                 mPImpl->latency,
                                                 kFSEventStreamCreateFlagFileEvents
                                                 );
            
            mPImpl->runLoop = CFRunLoopGetCurrent();
            // schedule the stream on the current run loop
            FSEventStreamScheduleWithRunLoop(mPImpl->stream, mPImpl->runLoop, kCFRunLoopDefaultMode);
            // tells the stream to begin sending events
            FSEventStreamStart(mPImpl->stream);
            
            CFRunLoopRun();
        });
	}


	/**
	* Cleanup
	*/
	DirectoryWatcher::~DirectoryWatcher()
	{
        CFRunLoopStop(mPImpl->runLoop);
        mPImpl->watchThread->join();
        FSEventStreamStop(mPImpl->stream);
        FSEventStreamInvalidate(mPImpl->stream);
        FSEventStreamRelease(mPImpl->stream);
	}


	/**
	* Checks if any changes to files were made, returns true if so. Continue to call this function to retrieve
	* multiple updates.
	*/
	bool DirectoryWatcher::update(std::vector<std::string>& modifiedFiles)
	{
        {
            std::unique_lock<std::mutex> lock(mPImpl->callbackInfo.watchMutex);
            
            if (mPImpl->callbackInfo.modifiedFiles.empty())
                return false;
            
            std::string comparable_watched_path = std::filesystem::canonical(mPImpl->currentPath);
            
            for (auto& modified_file : mPImpl->callbackInfo.modifiedFiles)
            {
                std::string comparable_modified_file = std::filesystem::canonical(modified_file);
                if (!std::filesystem::is_directory(modified_file))
                {
                    // check if the watched path is found at the start if the modified file's path
                    assert(comparable_modified_file.find(comparable_watched_path + "/") != std::string::npos);

                    // strip the watched path from the start
                    comparable_modified_file.erase(0, comparable_watched_path.size() + 1);

                    modifiedFiles.emplace_back(comparable_modified_file);
                }
            }
            
            mPImpl->callbackInfo.modifiedFiles.clear();
        }
        
        // if the modified files found by the event stream are not found in the watched dir we still need to return false
        return !modifiedFiles.empty();
	}
}
