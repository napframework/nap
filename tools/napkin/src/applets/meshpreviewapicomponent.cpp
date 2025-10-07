/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "meshpreviewapicomponent.h"
#include "../appletextension.h"

// External includes
#include <entity.h>
#include <nap/core.h>
#include <imguiservice.h>

// nap::meshpreviewapicomponent run time class definition 
RTTI_BEGIN_CLASS(napkin::MeshPreviewAPIcomponent)
	// Put additional properties here
RTTI_END_CLASS

// nap::meshpreviewapicomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(napkin::MeshPreviewAPIComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace napkin
{
	void MeshPreviewAPIcomponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(nap::APIComponent));
	}


	bool MeshPreviewAPIComponentInstance::init(utility::ErrorState& errorState)
	{
		// Gui service
		mGUIService = getEntityInstance()->getCore()->getService<nap::IMGuiService>();
		assert(mGUIService != nullptr);

		// Get signatures we're interested in
		mAPIComponent = getEntityInstance()->findComponent<APIComponentInstance>();
		if (!errorState.check(mAPIComponent != nullptr, "Missing API component"))
			return false;

		const auto* load_mesh_sig = mAPIComponent->findSignature(MeshPreviewAPIcomponent::loadMeshCmd);
		if (!errorState.check(load_mesh_sig != nullptr, "Missing '%s' cmd signature", MeshPreviewAPIcomponent::loadMeshCmd))
			return false;

		const auto* clear_sig = mAPIComponent->findSignature(MeshPreviewAPIcomponent::clearMeshCmd);
		if (!errorState.check(clear_sig != nullptr, "Missing '%s' cmd signature", MeshPreviewAPIcomponent::clearMeshCmd))
			return false;

		const auto* change_theme_sig = mAPIComponent->findSignature(MeshPreviewAPIcomponent::changeThemeCmd);
		if (!errorState.check(change_theme_sig != nullptr, "Missing '%s' cmd signature", MeshPreviewAPIcomponent::changeThemeCmd))
			return false;

		// Extension used for thread-safe loading
		mExtension = &getEntityInstance()->getCore()->getExtension<napkin::AppletExtension>();
		if (!errorState.check(mExtension != nullptr && mExtension->hasProject(),
			"Unable to resolve editor project information"))
			return false;

		// Register api cmd listeners
		mAPIComponent->registerCallback(*load_mesh_sig, mMeshLoadRequested);
		mAPIComponent->registerCallback(*change_theme_sig, mChangeThemeSlot);
		mAPIComponent->registerCallback(*clear_sig, mClearRequestedSlot);

		return true;
	}


	void MeshPreviewAPIComponentInstance::loadMesh(const nap::APIEvent& apiEvent)
	{
		nap::Logger::info("should load mesh!");
	}


	void MeshPreviewAPIComponentInstance::clear(const nap::APIEvent& apiEvent)
	{

	}


	void MeshPreviewAPIComponentInstance::changeTheme(const nap::APIEvent& apiEvent)
	{
		auto theme_arg = apiEvent.getArgumentByName(MeshPreviewAPIcomponent::changeThemeArg1);
		assert(theme_arg != nullptr);
		auto theme_name = theme_arg->asString();
		utility::removeChars(" ", theme_name);

		auto pal_type = RTTI_OF(gui::EColorScheme); assert(pal_type.is_enumeration());
		auto var = pal_type.get_enumeration().name_to_value(theme_name.data());
		if (!var.is_valid())
		{
			Logger::error("Unable to bind color palette to theme with name: %s",
				theme_name.c_str());
			return;
		}
		mGUIService->setPalette(var.get_value<gui::EColorScheme>());
	}
}
