#pragma once

// nap includes
#include <nap/service.h>

// audio includes
#include "audionodemanager.h"
#include "audiodevice.h"

namespace nap {
    
    namespace audio {
        
        class NAPAPI AudioService : public Service {
            RTTI_ENABLE(Service)
        public:
            AudioService();
            ~AudioService();
        };
        
    }
    
}
