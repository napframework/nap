#pragma once

// Nap includes
#include <nap/service.h>

namespace nap 
{   
	class WebSocketInterface;
	class WebSocketComponentInstance;

    /**
     * Main interface for processing web socket events in NAP.
     */
    class NAPAPI WebSocketService : public nap::Service
    {
		friend class WebSocketInterface;
		friend class WebSocketComponentInstance;
        RTTI_ENABLE(nap::Service)
    public:
		// Constructor
		WebSocketService(ServiceConfiguration* configuration);
        
        // Initialization
        bool init(nap::utility::ErrorState& errorState) override;

	protected:
		/**
		 * Registers all objects that need a specific way of construction.
		 * @param factory the factory to register the object creators with.
		 */
		virtual void registerObjectCreators(rtti::Factory& factory) override;

		/**
		 * Processes all received web-socket events from all registered web socket servers.
		 * The events are forwarded to all the the registered websocket components.
		 * This function is called automatically by the application loop.
		 * @param deltaTime time in between calls in seconds.
		 */
		virtual void update(double deltaTime) override;

	private:
		/**
		 * Registers a web socket interface with the service
		 */
		void registerInterface(WebSocketInterface& wsInterface);

		/**
		 * Removes a web socket interface from the service
		 */
		void removeInterface(WebSocketInterface& wsInterface);

		/**
		 * Registers a web-socket component with the service
		 */
		void registerComponent(WebSocketComponentInstance& component);

		/**
		 * Removes a web-socket component from the service
		 */
		void removeComponent(WebSocketComponentInstance& component);

		// All the web socket servers currently registered in the system
		std::vector<WebSocketInterface*> mInterfaces;

		// All the web socket server components currently registered in the system
		std::vector<WebSocketComponentInstance*> mComponents;
    };
}
