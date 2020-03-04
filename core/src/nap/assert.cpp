// Local Includes
#include "assert.h"

// External Includes
#include <iostream>

namespace nap
{
	void _assert(const char* exprs, bool expr, const char* file, int line)
	{
		if (!expr)
		{
			std::cerr << "Assertion failed: " <<
				exprs << ", source: " <<
				file  << ", line " <<
				line  << "\n";
			abort();
		}
	}


	void _assertMsg(const char* exprs, bool expr, const char* file, int line, const char* msg)
	{
		if (!expr)
		{
			std::cerr << "Assertion failed: " <<
				exprs << ", source: " <<
				file  << ", line " <<
				line  << ", msg: " << 
				msg   << "\n";
			abort();
		}
	}
}