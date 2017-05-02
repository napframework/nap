#include "../directorywatcher.h"

#include "assert.h"
#include <CoreServices/CoreServices.h>

namespace nap {
    
    
    void scanCallback(ConstFSEventStreamRef streamRef,
                      void *clientCallBackInfo,
                      size_t numEvents,
                      void *eventPaths,
                      const FSEventStreamEventFlags eventFlags[],
                      const FSEventStreamEventId eventIds[])
    {
        char **paths = (char**)eventPaths;
        
        // printf("Callback called\n");
        for (auto i = 0; i < numEvents; i++) {
            /* flags are unsigned long, IDs are uint64_t */
            printf("Change %llu in %s, flags %u\n", eventIds[i], paths[i], (unsigned int)eventFlags[i]);
        }
    }
    
    
	/**
	* Internal data container to hide internals from the header.
	*/
    struct DirectoryWatcher::PImpl
	{
        CFStringRef mypath = CFSTR("");
        CFArrayRef pathsToWatch = CFArrayCreate(NULL, (const void **)&mypath, 1, NULL);
        FSEventStreamContext* callbackInfo = nullptr; // could put stream-specific data here.
        FSEventStreamRef stream;
        CFAbsoluteTime latency = 3.0; /* Latency in seconds */
    };
    
    
    /**
     * Deleter operator definition
     */
    void DirectoryWatcher::PImpl_deleter::operator()(DirectoryWatcher::PImpl*ptr) const { delete ptr; }

    
	/**
	* Installs monitor: opens directory, creates event, starts directory scan.
	*/
	DirectoryWatcher::DirectoryWatcher()
	{
        mPImpl = std::unique_ptr<PImpl, PImpl_deleter>(new PImpl);
        
        // create event stream for file change event
        mPImpl->stream = FSEventStreamCreate(NULL,
                                             &scanCallback,
                                             mPImpl->callbackInfo,
                                             mPImpl->pathsToWatch,
                                             kFSEventStreamEventIdSinceNow,
                                             mPImpl->latency,
                                             kFSEventStreamCreateFlagNone
                                             );
        // schedule the stream on the current run loop
        FSEventStreamScheduleWithRunLoop(mPImpl->stream, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
        // tells the stream to begin sending events
        FSEventStreamStart(mPImpl->stream);
	}


	/**
	* Cleanup
	*/
	DirectoryWatcher::~DirectoryWatcher()
	{
        FSEventStreamUnscheduleFromRunLoop(mPImpl->stream, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
        FSEventStreamInvalidate(mPImpl->stream);
        FSEventStreamRelease(mPImpl->stream);
	}


	/**
	* Checks if any changes to files were made, returns true if so. Continue to call this function to retrieve
	* multiple updates.
	*/
	bool DirectoryWatcher::update(std::vector<std::string>& modifiedFiles)
	{
		return false;
	}
}
