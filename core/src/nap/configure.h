#pragma once

// Std Includes
#include <stdint.h>

// Windows defines it's own min max functions that we don't support
// Use std::min / std::max to be used with nap
#if defined(__WIN32__) || defined(_WIN32)
#ifndef __GNUC__
#define NOMINMAX
#endif
#endif

// Using
namespace nap
{
	// Numeric typedefs
	//using byte = unsigned char;
	using int8 = int8_t;
	using uint8 = uint8_t;
	using int16 = uint16_t;
	using uint16 = uint16_t;
	using int32 = int32_t;
	using uint32 = uint32_t;
//	using int64 = int64_t;
	using uint64 = uint64_t;
	using uint = unsigned int;

	// epsilon and double float
	extern const float f_epsilon;
	extern const double d_epsilon;
}
