#pragma once

// Nap includes
#include <nap/service.h>

namespace nap 
{   
    /**
     * Websocket service
     */
    class NAPAPI WebsocketService : public nap::Service
    {
        RTTI_ENABLE(nap::Service)
        
    public:
		WebsocketService(ServiceConfiguration* configuration);
        
        // Initialization
        bool init(nap::utility::ErrorState& errorState) override;
    };
        
}
