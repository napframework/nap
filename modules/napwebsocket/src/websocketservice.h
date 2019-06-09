#pragma once

// Nap includes
#include <nap/service.h>

namespace nap 
{   
	class WebSocketServer;
	class WebSocketClient;
	class WebSocketServerComponentInstance;
	class WebSocketClientComponentInstance;

    /**
     * Main interface for processing web socket events in NAP.
     */
    class NAPAPI WebSocketService : public nap::Service
    {
		friend class WebSocketServer;
		friend class WebSocketClient;
		friend class WebSocketServerComponentInstance;
		friend class WebSocketClientComponentInstance;
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
		 * Registers a web socket server with the service
		 */
		void registerServer(WebSocketServer& server);

		/**
		 * Removes a web socket server with the service
		 */
		void removeServer(WebSocketServer& server);

		/**
		 * Registers a web socket server with the service
		 */
		void registerClient(WebSocketClient& client);

		/**
		 * Removes a web socket server with the service
		 */
		void removeClient(WebSocketClient& client);

		/**
		 * Registers a web-socket component with the service
		 */
		void registerServerComponent(WebSocketServerComponentInstance& component);

		/**
		 * Removes a web-socket component from the service
		 */
		void removeServerComponent(WebSocketServerComponentInstance& component);

		/**
		 * Registers a web-socket component with the service
		 */
		void registerClientComponent(WebSocketClientComponentInstance& component);

		/**
		 * Removes a web-socket component from the service
		 */
		void removeClientComponent(WebSocketClientComponentInstance& component);

		/**
		 * Forwards all server events to the right component
		 */
		void forwardServerEvents();

		/**
		 * Forwards all client events to the right component
		 */
		void forwardClientEvents();

		// All the web socket servers currently registered in the system
		std::vector<WebSocketServer*> mServers;

		// All the web socket clients currently registered in the system
		std::vector<WebSocketClient*> mClients;

		// All the web socket server components currently registered in the system
		std::vector<WebSocketServerComponentInstance*> mServerComponents;

		// All the web socket clients components currently registered in the system
		std::vector<WebSocketClientComponentInstance*> mClientComponents;
    };
}
