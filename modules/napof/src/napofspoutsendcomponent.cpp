#include <napofspoutsendcomponent.h>

namespace nap
{
	// Default constructor
	OFSpoutSenderComponent::OFSpoutSenderComponent() : mSender("SpoutSender")
	{
		mSenderName.valueChanged.connect(mNameChanged);
	}


	// Occurs when the name changes
	void OFSpoutSenderComponent::nameChanged(AttributeBase& attr)
	{
		mSender.SetName(attr.getValue<std::string>());
	}


	// Updates sends the texture
	void OFSpoutSenderComponent::onUpdate()
	{
		// If we don't have a texture, don't do anything
		if (mSendTexture == nullptr)
			return;

		// Send texture, also initializes the component if necessary
		mSender.SendTexture(const_cast<ofTexture&>(*mSendTexture));
	}
}

RTTI_DEFINE(nap::OFSpoutSenderComponent)
