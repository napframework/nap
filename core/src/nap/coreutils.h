#pragma once

#include <thread>

namespace nap
{
    void setThreadName(const char* name);
	void setThreadName(std::thread* thread, const char* name);
}