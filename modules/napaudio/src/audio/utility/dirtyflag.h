#pragma once

// Std includes
#include <atomic>

namespace nap {
    
    namespace audio {
        
        class DirtyFlag {
        public:
            DirtyFlag() { mUpToDate.test_and_set(); }
            inline void set() { mUpToDate.clear(); }
            inline bool check() { return !mUpToDate.test_and_set(); }
        private:
            std::atomic_flag mUpToDate = ATOMIC_FLAG_INIT;
        };
        
    }
    
}
