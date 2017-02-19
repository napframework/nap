#pragma once

// NAP Includes
#include <nap/component.h>
#include <nap/attribute.h>

#include <napofattributes.h>
#include <napofupdatecomponent.h>

// OF Includes
#include <Spout/nofSpoutSender.h>

namespace nap
{
	class OFSpoutSenderComponent : public OFUpdatableComponent
	{
		RTTI_ENABLE_DERIVED_FROM(OFUpdatableComponent)
	public:
		// Default constructor
		OFSpoutSenderComponent();

		// Attributes
		Attribute<std::string> mSenderName =						{ this, "Name", "SpoutSender" };

		// Texture to send
		void setTexture(const ofTexture& inTexture)					{ mSendTexture = &inTexture; }
		void setMode(NSpoutSender::EHardwareMode inMode)			{ mSender.SetHardwareMode(inMode); }
		bool isInitialized() const									{ return mSender.IsInitialized(); }

		// Update call
		virtual void onUpdate()	override;
		
		// Slots
		NSLOT(mNameChanged, AttributeBase&, nameChanged)

	private:
		// Spout send object
		NSpoutSender mSender;
		const ofTexture* mSendTexture = nullptr;

		// Initializes the spout sender with the new name
		void nameChanged(AttributeBase& inName);
	};
}

RTTI_DECLARE(nap::OFSpoutSenderComponent)
