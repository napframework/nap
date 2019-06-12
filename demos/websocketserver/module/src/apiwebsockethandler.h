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
	 * 
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
	 * apiwebsockethandlerInstance	
	 */
	class NAPAPI APIWebSocketHandlerComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		APIWebSocketHandlerComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize apiwebsockethandlerInstance based on the apiwebsockethandler resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the apiwebsockethandlerInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update apiwebsockethandlerInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

		// Resolved pointer to the api component instance
		ComponentInstancePtr<APIComponent> mAPIComponent = { this, &APIWebSocketHandlerComponent::mAPIComponent };

		// Resolved pointer to the 2d text component instance
		ComponentInstancePtr<Renderable2DTextComponent> mTextComponent = { this, &APIWebSocketHandlerComponent::mTextComponent };

	private:
		// Called when text needs to change
		void onChangeText(const nap::APIEvent& apiEvent);
		nap::Slot<const nap::APIEvent&> mChangeTextSlot = { this, &APIWebSocketHandlerComponentInstance::onChangeText };
		APIWebSocketServer* mServer = nullptr;
	};
}
