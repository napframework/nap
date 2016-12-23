
#include "coreutils.h"

#ifdef _WIN32
#ifdef _MSC_VER
#include <windows.h>

const DWORD MS_VC_EXCEPTION = 0x406D1388;

#pragma pack(push, 8)
typedef struct tagTHREADNAME_INFO {
	DWORD dwType;	 // Must be 0x1000.
	LPCSTR szName;	// Pointer to name (in user addr space).
	DWORD dwThreadID; // Thread ID (-1=caller thread).
	DWORD dwFlags;	// Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)


void doSetThreadName(uint32_t dwThreadID, const char* threadName)
{

	// DWORD dwThreadID = ::GetThreadId( static_cast<HANDLE>( t.native_handle() ) );

	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = threadName;
	info.dwThreadID = dwThreadID;
	info.dwFlags = 0;

	__try {
		RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
	} __except (EXCEPTION_EXECUTE_HANDLER) {
	}
}
void doSetThreadName(const char* threadName) { doSetThreadName(GetCurrentThreadId(), threadName); }

void doSetThreadName(std::thread* thread, const char* threadName)
{
	DWORD threadId = ::GetThreadId(static_cast<HANDLE>(thread->native_handle()));
	doSetThreadName(threadId, threadName);
}
#else // _MSC_VER
void doSetThreadName(const char* threadName) {}

void doSetThreadName(std::thread* thread, const char* threadName) {}
#endif // _MSC_VER

#else // _WIN32
void doSetThreadName(std::thread* thread, const char* threadName)
{
	auto handle = thread->native_handle();
	pthread_setname_np(handle, threadName);
}

#include <sys/prctl.h>
void doSetThreadName(const char* threadName) { prctl(PR_SET_NAME, threadName, 0, 0, 0); }
#endif // _WIN32



namespace nap
{
	void setThreadName(std::thread* thread, const char* threadName) { doSetThreadName(thread, threadName); }
	void setThreadName(const char* threadName) { doSetThreadName(threadName); }
}
