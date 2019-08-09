#pragma once

#include <component.h>
#include <componentptr.h>
#include <apicomponent.h>
#include <apievent.h>
#include <nap/resourceptr.h>
#include <renderable2dtextcomponent.h>
#include <apiwebsocketserver.h>

namespace nap
{
	// Forward Declares
	class APIWebSocketHandlerComponentInstance;

	/**
	 * This component receives (and acts upon) specific api web-socket events.
	 * This component is only interested in the 'ChangeText' message. Other messages won't be
	 * handled by this component and are automatically discarded by the system. When a 'ChangeText' message
	 * is received the text (as the first argument) is extracted and used to change the input text of
	 * the linked nap::Renderable2DTextComponent. This renderable 2D text component is drawn to screen 
	 * every render cycle.
	 */
	class NAPAPI APIWebSocketHandlerComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(APIWebSocketHandlerComponent, APIWebSocketHandlerComponentInstance)
	public:
		 
		ComponentPtr<APIComponent> mAPIComponent;					///< Property: "APIComponent" Link to the api component that receives api events
		ComponentPtr<Renderable2DTextComponent> mTextComponent;		///< Property: "TextComponent" Link to the component that renders text
		ResourcePtr<APIWebSocketServer> mServer;					///< Property: "Server" Link to server, used to issue a reply.
	};


	/**
	 * This component receives (and acts upon) specific api web-socket events.
	 * This component is only interested in the 'ChangeText' message. Other messages won't be
	 * handled by this component and are automatically discarded by the system. When a 'ChangeText' message
	 * is received the text (the first argument) is extracted and used to change the input of
	 * the linked nap::Renderable2DTextComponent. 
	 *
	 * On initialization this component tries to find the 'ChangeText' signature, 
	 * this signature is a property of the nap::APIComponent. When found a callback (onChangeText) is registered with 
	 * the api component that is triggered when the api component receives a 'ChangeText' message over the wire.
	 * The callback is always triggered on the main (application) thread. It is therefore safe to immediately
	 * act upon it. In this case the callback updates the text that is displayed in the render window.
	 *
	 * To learn more about the web-socket server and client refer to: nap::APIWebSocketServer and nap::APIWebSocketClient.
	 */
	class NAPAPI APIWebSocketHandlerComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		APIWebSocketHandlerComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize instance based on the resource.
		 * Registers the 'onChangeText' callback with the api component. This callback is triggered
		 * when the api component receives an 'ChangeText' message.
		 * @param errorState holds the error when initialization fails.
		 * @return if the instance initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		// Resolved pointer to the api component instance
		ComponentInstancePtr<APIComponent> mAPIComponent = { this, &APIWebSocketHandlerComponent::mAPIComponent };

		// Resolved pointer to the 2d text component instance
		ComponentInstancePtr<Renderable2DTextComponent> mTextComponent = { this, &APIWebSocketHandlerComponent::mTextComponent };

	private:
		/**
		 * Called when a 'ChangeText' api message is received by the api component.
		 * @param apiEvent the web-socket api event that contains the new line of text.
		 */
		void onChangeText(const nap::APIEvent& apiEvent);
		nap::Slot<const nap::APIEvent&> mChangeTextSlot = { this, &APIWebSocketHandlerComponentInstance::onChangeText };

		// Handle to the server to send a reply
		APIWebSocketServer* mServer = nullptr;
	};
}
