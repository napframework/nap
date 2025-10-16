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
#include <rtti/jsonreader.h>
#include <mesh.h>

RTTI_BEGIN_CLASS(napkin::MeshPreviewAPIcomponent)
	RTTI_PROPERTY("Loader", &napkin::MeshPreviewAPIcomponent::mLoader, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

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
		// Get mesh data
		auto* data_arg = apiEvent.getArgumentByName(MeshPreviewAPIcomponent::loadMeshArg1);
		assert(data_arg != nullptr);

		// De-serialize mesh
		nap::Core& core = *getEntityInstance()->getCore();
		utility::ErrorState error; rtti::DeserializeResult result;
		if (!rtti::deserializeJSON(data_arg->asString(), rtti::EPropertyValidationMode::DisallowMissingProperties,
			rtti::EPointerPropertyMode::OnlyRawPointers, core.getResourceManager()->getFactory(), result, error))
		{
			error.fail("%s cmd failed", MeshPreviewAPIcomponent::loadMeshCmd);
			nap::Logger::error(error.toString());
			return;
		}

		// Ensure there's at least one object
		if (result.mReadObjects.empty())
		{
			nap::Logger::error("%s cmd failed: invalid payload", MeshPreviewAPIcomponent::loadMeshCmd);
			return;
		}

		// Warn if there's more than 1 object and store
		if (result.mReadObjects.size() > 1)
			nap::Logger::warn("%s cmd holds multiple objects, initializing first one...",
				MeshPreviewAPIcomponent::loadMeshCmd);

		// Ensure type is a mesh
		auto& object = result.mReadObjects[0];
		if (!object->get_type().is_derived_from(RTTI_OF(nap::IMesh)))
		{
			nap::Logger::error("%s cmd failed: unsupported type", MeshPreviewAPIcomponent::loadMeshCmd);
			return;
		}

		// Init mesh relative to project data directory (thread-safe)
		{
			napkin::CWDHandle cwd_handle = mExtension->switchWorkingDir();
			if (!object->init(error))
			{
				nap::Logger::error(error.toString().c_str());
				return;
			}
		}

		// Load it
		std::unique_ptr<IMesh> mesh(static_cast<IMesh*>(object.release()));
		if (!mLoader->load(std::move(mesh), error))
		{
			nap::Logger::error(error.toString());
			return;
		}

		// Frame when requested
		auto* frame_arg = apiEvent.getArgumentByName(MeshPreviewAPIcomponent::loadMeshArg2);
		if (frame_arg->asBool())
			mLoader->frame();
	}


	void MeshPreviewAPIComponentInstance::clear(const nap::APIEvent& apiEvent)
	{
		mLoader->clear();
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

