/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local includes
#include "frame2dtexturecomponent.h"

// External includes
#include <component.h>
#include <apicomponent.h>
#include <componentptr.h>
#include <renderskyboxcomponent.h>
#include <orbitcontroller.h>

namespace napkin
{
	using namespace nap;
	class TexturePreviewApplet;
	class LoadTextureComponentInstance;

	/**
	 * Loads and sets a texture from an API command received from Napkin.
	 */
	class LoadTextureComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(LoadTextureComponent, LoadTextureComponentInstance)
	public:

		// Signature callback names and arguments
		static constexpr const char* loadCmd = "LoadTexture";
		static constexpr const char* loadArg1 = "data";
		static constexpr const char* loadArg2 = "frame";
		static constexpr const char* clearCmd = "ClearTexture";

		// Properties
		nap::ComponentPtr<Frame2DTextureComponent> mFrame2DTextureComponent;	///< Property: 'Frame2DTextureComponent' The component that binds and frames the 2D texture
		nap::ComponentPtr<RenderSkyBoxComponent> mSkyboxComponent;				///< Property: 'SkyboxComponent' The component that renders the skybox
		nap::ComponentPtr<OrbitController> mSkyboxController;					///< Property: 'SkyboxController' The skybox camera controller

		// Requires an api component
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;
	};


	/**
	 * Loads and sets a texture from an API command received from Napkin.
	 */
	class LoadTextureComponentInstance : public ComponentInstance
	{
		friend class TexturePreviewApplet;
		RTTI_ENABLE(ComponentInstance)
	public:

		// Current loaded type
		enum class EType: uint8
		{
			None		= 0,
			Texture2D	= 1,
			Cubemap		= 2
		};

		// Constructor
		LoadTextureComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)					{ }

		// Destructor
		virtual ~LoadTextureComponentInstance() override;

		// Init
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return if this component loaded a texture
		 */
		bool hasTexture() const									{ return mActiveTexture != nullptr; }

		/**
		 * @return current loaded texture, nullptr if no texture is loaded
		 */
		nap::Texture* getTexture() const						{ return mActiveTexture; }

		/**
		 * Current loaded texture type
		 * @return loaded texture type, none when no texture is loaded
		 */
		LoadTextureComponentInstance::EType getType();

		/**
		 * Frames current selection
		 */
		void frame();

		// The resolved 2d texture frame component
		ComponentInstancePtr<Frame2DTextureComponent> mFrame2DTextureComponent = { this, &LoadTextureComponent::mFrame2DTextureComponent };

		// The resolved skybox component
		ComponentInstancePtr<RenderSkyBoxComponent> mSkyboxComponent = { this, &LoadTextureComponent::mSkyboxComponent };

		// The resolved skybox controller
		ComponentInstancePtr<OrbitController> mSkyboxController = { this, &LoadTextureComponent::mSkyboxController };

	private:
		void onLoadRequested(const nap::APIEvent& apiEvent);					//< Loads a texture from JSON
		nap::Slot<const nap::APIEvent&> mLoadRequestedSlot = { this, &LoadTextureComponentInstance::onLoadRequested };

		void onClearRequested(const nap::APIEvent& apiEvent);					//< Clears texture
		nap::Slot<const nap::APIEvent&> mClearRequestedSlot = { this, &LoadTextureComponentInstance::onClearRequested };

		nap::APIComponentInstance* mAPIComponent = nullptr;						//< Pointer to the api component
		std::string mProjectDataDirectory;										//< Data directory to resolve texture load cmds against

		std::unique_ptr<nap::Texture2D> mLoaded2DTexture = nullptr;				//< Current active loaded 2D texture
		std::unique_ptr<nap::TextureCube> mLoadedCubeTexture = nullptr;			//< Current active loaded Cube texture
		nap::Texture* mActiveTexture = nullptr;									//< Selected active texture handle
	};
}

