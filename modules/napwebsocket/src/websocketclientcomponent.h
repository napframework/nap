#pragma once

// Local Includes
#include "websocketclient.h"

// External Includes
#include <component.h>
#include <nap/signalslot.h>

namespace nap
{
	class WebSocketClientComponentInstance;
	class WebSocketService;

	/**
	 *	websocketcomponent
	 */
	class NAPAPI WebSocketClientComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(WebSocketClientComponent, WebSocketClientComponentInstance)
	public:
		nap::ResourcePtr<WebSocketClient> mClient;			///< Property: "Client" the web-socket client this component listens to for events.
	};


	/**
	 * websocketcomponentInstance	
	 */
	class NAPAPI WebSocketClientComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		WebSocketClientComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		virtual ~WebSocketClientComponentInstance();

		/**
		 * Initialize websocketcomponentInstance based on the websocketcomponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the websocketcomponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		const WebSocketClient& getClient() const					{ return *mClient; }

		WebSocketClient& getClient()								{ return *mClient; }

		nap::Signal<const WebSocketConnectionOpenedEvent&> connectionOpened;
		nap::Signal<const WebSocketConnectionClosedEvent&> connectionClosed;
		nap::Signal<const WebSocketConnectionFailedEvent&> connectionFailed;
		nap::Signal<const WebSocketMessageReceivedEvent&> messageReceived;

	private:
		WebSocketClient*  mClient  = nullptr;
		WebSocketService* mService = nullptr;
	};
}
