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

		// Constructor
		LoadTextureComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)					{ }

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
		 * Current loaded texture type, either of type Texture2D or TextureCube 
		 * @return loaded texture type, invalid when no texture is loaded
		 */
		rtti::TypeInfo getType()								{ return hasTexture() ? mActiveTexture->get_type() : rtti::TypeInfo::empty(); }

		// The resolved 2d texture frame component
		ComponentInstancePtr<Frame2DTextureComponent> mFrame2DTextureComponent = { this, &LoadTextureComponent::mFrame2DTextureComponent };

		// The resolved skybox component
		ComponentInstancePtr<RenderSkyBoxComponent> mSkyboxComponent = { this, &LoadTextureComponent::mSkyboxComponent };

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

