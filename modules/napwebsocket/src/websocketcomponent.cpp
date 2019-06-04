// Local Includes
#include "websocketcomponent.h"
#include "websocketservice.h"

// External Includes
#include <entity.h>
#include <nap/core.h>

// nap::websocketcomponent run time class definition 
RTTI_BEGIN_CLASS(nap::WebSocketComponent)
	RTTI_PROPERTY("Server", &nap::WebSocketComponent::mServer, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::websocketcomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::WebSocketComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void WebSocketComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{

	}


	WebSocketComponentInstance::~WebSocketComponentInstance()
	{
		if (mService != nullptr)
			mService->removeComponent(*this);
	}


	bool WebSocketComponentInstance::init(utility::ErrorState& errorState)
	{
		// Extract the server
		mServer = getComponent<WebSocketComponent>()->mServer.get();

		mService = getEntityInstance()->getCore()->getService<nap::WebSocketService>();
		assert(mService != nullptr);
		mService->registerComponent(*this);

		return true;
	}
}