#pragma once

// Local Includes
#include "websocketcomponent.h"

namespace nap
{
	class WebSocketHandlerInstance;

	/**
	 *	websockethandler
	 */
	class NAPAPI WebSocketHandler : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(WebSocketHandler, WebSocketHandlerInstance)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;
	};


	/**
	 * websockethandlerInstance	
	 */
	class NAPAPI WebSocketHandlerInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		WebSocketHandlerInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize websockethandlerInstance based on the websockethandler resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the websockethandlerInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update websockethandlerInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

	private:
		WebSocketComponentInstance* mWSComponent = nullptr;

		void onConnectionOpened(const WebSocketConnectionOpenedEvent& wsEvent);
		nap::Slot<const WebSocketConnectionOpenedEvent&> mConnectionOpened = { this, &WebSocketHandlerInstance::onConnectionOpened};

		void onConnectionClosed(const WebSocketConnectionClosedEvent& wsEvent);
		nap::Slot<const WebSocketConnectionClosedEvent&> mConnectionClosed = { this, &WebSocketHandlerInstance::onConnectionClosed };

		void onMessageReceived(const WebSocketMessageReceivedEvent& wsEvent);
		nap::Slot<const WebSocketMessageReceivedEvent&> mMessageReceived = { this, &WebSocketHandlerInstance::onMessageReceived };
	};
}
