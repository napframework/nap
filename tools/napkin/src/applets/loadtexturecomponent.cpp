/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "loadtexturecomponent.h"
#include "../naputils.h"

// External Includes
#include <entity.h>
#include <rtti/jsonreader.h>
#include <nap/core.h>
#include <nap/logger.h>
#include <texture.h>

// nap::loadtexturecomponent run time class definition 
RTTI_BEGIN_CLASS(napkin::LoadTextureComponent)
	RTTI_PROPERTY("Frame2DTextureComponent", &napkin::LoadTextureComponent::mFrame2DTextureComponent, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::loadtexturecomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(napkin::LoadTextureComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace napkin
{
	void LoadTextureComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(APIComponent));
	}


	bool LoadTextureComponentInstance::init(utility::ErrorState& errorState)
	{
		mAPIComponent = getEntityInstance()->findComponent<APIComponentInstance>();
		if (!errorState.check(mAPIComponent != nullptr, "Missing API component"))
			return false;

		const auto* load_sig = mAPIComponent->findSignature(LoadTextureComponent::loadCmd);
		if (!errorState.check(load_sig != nullptr, "Missing '%s' cmd signature", LoadTextureComponent::loadCmd))
			return false;

		const auto* clear_sig = mAPIComponent->findSignature(LoadTextureComponent::clearCmd);
		if (!errorState.check(clear_sig != nullptr, "Missing '%s' cmd signature", LoadTextureComponent::clearCmd))
			return false;

		// Register api cmd listeners
		mAPIComponent->registerCallback(*load_sig, mLoadRequestedSlot);
		mAPIComponent->registerCallback(*clear_sig, mClearRequestedSlot);

		return true;
	}


	void LoadTextureComponentInstance::onLoadRequested(const nap::APIEvent& apiEvent)
	{
		auto* data_arg = apiEvent.getArgumentByName(LoadTextureComponent::loadArg1);
		assert(data_arg != nullptr);

		// De-serialize JSON
		nap::Core& core = *getEntityInstance()->getCore();
		utility::ErrorState error; rtti::DeserializeResult result;
		if (!rtti::deserializeJSON(data_arg->asString(), rtti::EPropertyValidationMode::DisallowMissingProperties,
			rtti::EPointerPropertyMode::OnlyRawPointers, core.getResourceManager()->getFactory(), result, error))
		{
			error.fail("%s cmd failed", LoadTextureComponent::loadCmd);
			nap::Logger::error(error.toString());
			return;
		}

		// Ensure there's at least 1 object and it's of type texture
		if (result.mReadObjects.size() == 0 || !result.mReadObjects[0]->get_type().is_derived_from(RTTI_OF(nap::Texture2D)))
		{
			nap::Logger::error("%s cmd failed: invalid payload", LoadTextureComponent::loadCmd);
			return;
		}

		// Warn if there's more than 1 object and store
		if (result.mReadObjects.size() > 1)
			nap::Logger::warn("%s cmd holds multiple objects, initializing first one...", LoadTextureComponent::loadCmd);

		// Init texture relative to project data directory (thread-safe)
		{
			assert(!mProjectDataDirectory.empty());
			napkin::CWDHandle cwd_handle(mProjectDataDirectory);
			if (!result.mReadObjects[0]->init(error))
			{
				nap::Logger::error(error.toString());
				return;
			}
		}

		// Store
		mActiveTexture.reset(static_cast<Texture2D*>(result.mReadObjects[0].release()));

		// Bind
		mFrame2DTextureComponent->bind(*mActiveTexture);

		// Reset pan & zoom controls if requested
		auto* frame_cmd = apiEvent.getArgumentByName(LoadTextureComponent::loadArg2);
		assert(frame_cmd != nullptr);
		if (frame_cmd->asBool())
			mFrame2DTextureComponent->frame();
	}


	void LoadTextureComponentInstance::onClearRequested(const nap::APIEvent& apiEvent)
	{
		mFrame2DTextureComponent->clear();
		mActiveTexture.reset(nullptr);
	}
}
