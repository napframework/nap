// Local Includes
#include "websocketservercomponent.h"
#include "websocketservice.h"

// External Includes
#include <entity.h>
#include <nap/core.h>

// nap::websocketcomponent run time class definition 
RTTI_BEGIN_CLASS(nap::WebSocketServerComponent)
	RTTI_PROPERTY("Server", &nap::WebSocketServerComponent::mServer, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::websocketcomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::WebSocketServerComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void WebSocketServerComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{

	}


	WebSocketServerComponentInstance::~WebSocketServerComponentInstance()
	{
		if (mService != nullptr)
			mService->removeServerComponent(*this);
	}


	bool WebSocketServerComponentInstance::init(utility::ErrorState& errorState)
	{
		// Extract the server
		mServer = getComponent<WebSocketServerComponent>()->mServer.get();

		mService = getEntityInstance()->getCore()->getService<nap::WebSocketService>();
		assert(mService != nullptr);
		mService->registerServerComponent(*this);

		return true;
	}
}