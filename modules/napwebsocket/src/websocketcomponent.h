#pragma once

// Local Includes
#include "websocketserver.h"

// External Includes
#include <component.h>
#include <nap/signalslot.h>

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

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		nap::ResourcePtr<WebSocketServer> mServer;			///< Property: "Server" the web-socket server this component listens to for events.
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

		const WebSocketServer& getServer() const					{ return *mServer; }

		WebSocketServer& getServer()								{ return *mServer; }

		nap::Signal<const WebSocketConnectionOpenedEvent&> connectionOpened;
		nap::Signal<const WebSocketConnectionClosedEvent&> connectionClosed;
		nap::Signal<const WebSocketConnectionFailedEvent&> connectionFailed;
		nap::Signal<const WebSocketMessageReceivedEvent&> messageReceived;

	private:
		WebSocketServer*  mServer  = nullptr;
		WebSocketService* mService = nullptr;
	};
}
