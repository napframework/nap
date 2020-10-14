#pragma once

// Std includes
#include <atomic>

namespace nap
{
	
	namespace audio
	{
		
		/**
		 * Atomic flag that can indicate wether a system needs updating.
		 * The flag is set on one thread (presumably the main thread) and checked on another thread (usually the audio thread). When the flag is checked it's value automatically changes to non-dirty at the same time in one atomic operation.
		 */
		class DirtyFlag
		{
		public:
			DirtyFlag() { mUpToDate.test_and_set(); }
			
			/**
			 * Set the flag to dirty in one atomic operation.
			 */
			inline void set() { mUpToDate.clear(); }
			
			/**
			 * Returns wether the flag is dirty and sets it to non-dirty at the same time in one atomic operation.
			 */
			inline bool check() { return !mUpToDate.test_and_set(); }
		
		private:
			std::atomic_flag mUpToDate = ATOMIC_FLAG_INIT;
		};
		
	}
	
}
