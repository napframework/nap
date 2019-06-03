#pragma once

// Nap includes
#include <nap/service.h>

namespace nap 
{   
    /**
     * Ensures that all web socket events are correctly dispatched onto the main thread.
     */
    class NAPAPI WebSocketService : public nap::Service
    {
        RTTI_ENABLE(nap::Service)

    public:
		WebSocketService(ServiceConfiguration* configuration);
        
        // Initialization
        bool init(nap::utility::ErrorState& errorState) override;

	protected:
		/**
		 * Registers all objects that need a specific way of construction
		 * @param factory the factory to register the object creators with
		 */
		virtual void registerObjectCreators(rtti::Factory& factory) override;
    };
}
