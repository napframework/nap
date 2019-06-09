// Local Includes
#include "websocketclientcomponent.h"
#include "websocketservice.h"

// External Includes
#include <entity.h>
#include <nap/core.h>

// nap::websocketcomponent run time class definition 
RTTI_BEGIN_CLASS(nap::WebSocketClientComponent)
	RTTI_PROPERTY("Client", &nap::WebSocketClientComponent::mClient, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::websocketcomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::WebSocketClientComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	WebSocketClientComponentInstance::~WebSocketClientComponentInstance()
	{
		if (mService != nullptr)
			mService->removeClientComponent(*this);
	}


	bool WebSocketClientComponentInstance::init(utility::ErrorState& errorState)
	{
		// Extract the server
		mClient = getComponent<WebSocketClientComponent>()->mClient.get();

		mService = getEntityInstance()->getCore()->getService<nap::WebSocketService>();
		assert(mService != nullptr);
		mService->registerClientComponent(*this);

		return true;
	}
}