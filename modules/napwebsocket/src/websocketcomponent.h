#pragma once

// Local Includes
#include "websocketinterface.h"

// External Includes
#include <component.h>
#include <nap/signalslot.h>
#include <nap/resourceptr.h>

namespace nap
{
	class WebSocketComponentInstance;

	/**
	 *	websocketcomponent
	 */
	class NAPAPI WebSocketComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(WebSocketComponent, WebSocketComponentInstance)
	public:

		nap::ResourcePtr<WebSocketInterface> mInterface;			///< Property: "Server" the web-socket server this component listens to for events.
	};


	/**
	 * websocketcomponentInstance	
	 */
	class NAPAPI WebSocketComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		WebSocketComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		virtual ~WebSocketComponentInstance();

		/**
		 * Initialize websocketcomponentInstance based on the websocketcomponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the websocketcomponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		const WebSocketInterface& getInterface() const					{ return *mInterface; }

		WebSocketInterface& getInterface()								{ return *mInterface; }

		nap::Signal<const WebSocketConnectionOpenedEvent&> connectionOpened;
		nap::Signal<const WebSocketConnectionClosedEvent&> connectionClosed;
		nap::Signal<const WebSocketConnectionFailedEvent&> connectionFailed;
		nap::Signal<const WebSocketMessageReceivedEvent&> messageReceived;

	private:
		WebSocketInterface* mInterface  = nullptr;
		WebSocketService* mService = nullptr;
	};
}
