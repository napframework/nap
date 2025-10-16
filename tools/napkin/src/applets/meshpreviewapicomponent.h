/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local includes
#include "meshpreviewloadcomponent.h"

// External includes
#include <apicomponent.h>
#include <componentptr.h>

namespace nap
{
	class IMGuiService;
}

namespace napkin
{
	using namespace nap;
	class MeshPreviewAPIComponentInstance;
	class AppletExtension;

	/**
	 * De-serializes and loads a mesh from an API command received from Napkin.
	 */
	class MeshPreviewAPIcomponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(MeshPreviewAPIcomponent, MeshPreviewAPIComponentInstance)
	public:
		// Load mesh cmd & args
		static constexpr const char* loadMeshCmd = "LoadMesh";
		static constexpr const char* loadMeshArg1 = "data";
		static constexpr const char* loadMeshArg2 = "frame";

		// Clear mesh cmd
		static constexpr const char* clearMeshCmd = "ClearMesh";

		// Change theme cmd
		static constexpr const char* changeThemeCmd = "ChangeTheme";
		static constexpr const char* changeThemeArg1 = "theme";

		// Requires API component
		void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		ComponentPtr<MeshPreviewLoadComponent> mLoader;		///< Property: 'Loader' the load and frame component
	};


	/**
	 * De-serializes and loads a mesh from an API command received from Napkin.
	 */
	class MeshPreviewAPIComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		MeshPreviewAPIComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize component based on the resource
		 * @param errorState should hold the error message when initialization fails
		 * @return if the component initialized successfully
		 */
		bool init(utility::ErrorState& errorState) override;

		// Resolved link to the frame mesh component
		ComponentInstancePtr<MeshPreviewLoadComponent> mLoader = { this, &MeshPreviewAPIcomponent::mLoader };

	private:
		// Callbacks
		void loadMesh(const nap::APIEvent& apiEvent);			//< Loads a mesh from JSON
		nap::Slot<const nap::APIEvent&> mMeshLoadRequested =	{ this, &MeshPreviewAPIComponentInstance::loadMesh };

		void clear(const nap::APIEvent& apiEvent);				//< Clears texture
		nap::Slot<const nap::APIEvent&> mClearRequestedSlot =	{ this, &MeshPreviewAPIComponentInstance::clear };

		void changeTheme(const nap::APIEvent& apiEvent);		//< Changes theme
		nap::Slot<const nap::APIEvent&> mChangeThemeSlot =		{ this, &MeshPreviewAPIComponentInstance::changeTheme };

		// Links
		nap::APIComponentInstance* mAPIComponent = nullptr;		//< Pointer to the api component
		const napkin::AppletExtension* mExtension = nullptr;	//< Applet extension

		// GUI service
		nap::IMGuiService* mGUIService = nullptr;
	};
}
