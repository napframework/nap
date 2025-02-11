/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local includes
#include "frame2dtexturecomponent.h"
#include "framecubemapcomponent.h"
#include "../appletextension.h"

// External includes
#include <component.h>
#include <apicomponent.h>
#include <componentptr.h>
#include <renderskyboxcomponent.h>
#include <orbitcontroller.h>

namespace nap
{
	class IMGuiService;
}

namespace napkin
{
	using namespace nap;
	class TexturePreviewApplet;
	class LoadTextureComponentInstance;

	/**
	 * Loads and sets a texture or mesh from an API command received from Napkin.
	 */
	class LoadTextureComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(LoadTextureComponent, LoadTextureComponentInstance)
	public:

		// Load texture cmd & args
		static constexpr const char* loadTextureCmd = "LoadTexture";
		static constexpr const char* loadTextureArg1 = "data";
		static constexpr const char* loadTextureArg2 = "frame";

		// Load mesh cmd & args
		static constexpr const char* loadMeshCmd = "LoadMesh";
		static constexpr const char* loadMeshArg1 = "data";
		static constexpr const char* loadMeshArg2 = "frame";

		// Change theme cmd
		static constexpr const char* changeThemeCmd = "ChangeTheme";
		static constexpr const char* changeThemeArg1 = "theme";

		// Clear selection args
		static constexpr const char* clearCmd = "ClearTexture";

		// Properties
		nap::ComponentPtr<Frame2DTextureComponent> mFrame2DTextureComponent;	///< Property: 'Frame2DTextureComponent' The component that binds and frames the 2D texture
		nap::ComponentPtr<FrameCubemapComponent> mFrameCubemapComponent;		///< Property: 'FrameCubemapComponent' The component that binds and frames the cubemap

		// Requires an api component
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;
	};


	/**
	 * Loads and sets a texture or mesh from an API command received from Napkin.
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
			ComponentInstance(entity, resource) { }

		// Init
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Current loaded texture type
		 * @return loaded texture type, none when no texture is loaded
		 */
		LoadTextureComponentInstance::EType getType() const { return mSelectedType; }

		/**
		 * @return current selected and loaded texture, nullptr if no texture is loaded
		 */
		const Texture* getTexture() const;

		/**
		 * @return current selected mesh, nullptr if no mesh is selected
		 */
		const IMesh* getMesh() const;

		/**
		 * @return current mesh bounds
		 */
		const math::Box& getMeshBounds() const;

		/**
		 * Frames current selection
		 */
		void frame();

		/**
		 * Set alpha of current selection
		 */
		void setOpacity(float alpha);

		/**
		 * Get alpha of current selection
		 */
		float getOpacity() const;

		/**
		 * Set rotation speed (s) of current selection
		 */
		void setRotate(float alpha);

		/**
		 * Get rotation speed (s) of current selection
		 */
		float getRotate() const;

		/**
		 * Process window events
		 * @param inputService the nap input service that holds the events
		 * @param window the window that received the events
		 */
		void processWindowEvents(nap::InputService& inputService, nap::RenderWindow& window);

		/**
		 * Draw current selection to given window
		 * @param renderService the render service to use
		 * @param window the window to render selection to
		 */
		void draw(RenderService& renderService, RenderWindow& window);

		// The resolved 2d texture frame component
		ComponentInstancePtr<Frame2DTextureComponent> mFrame2DTextureComponent = { this, &LoadTextureComponent::mFrame2DTextureComponent };

		// The resolved cubemap frame component
		ComponentInstancePtr<FrameCubemapComponent> mFrameCubeComponent = { this, &LoadTextureComponent::mFrameCubemapComponent };

	private:
		void loadTexture(const nap::APIEvent& apiEvent);						//< Loads a texture from JSON
		nap::Slot<const nap::APIEvent&> mTextureLoadRequested =					{ this, &LoadTextureComponentInstance::loadTexture };

		void loadMesh(const nap::APIEvent& apiEvent);							//< Loads a mesh from JSON
		nap::Slot<const nap::APIEvent&> mMeshLoadRequested =					{ this, &LoadTextureComponentInstance::loadMesh };
		
		void clear(const nap::APIEvent& apiEvent);								//< Clears texture
		nap::Slot<const nap::APIEvent&> mClearRequestedSlot =					{ this, &LoadTextureComponentInstance::clear };

		void changeTheme(const nap::APIEvent& apiEvent);						//< Changes theme
		nap::Slot<const nap::APIEvent&> mChangeThemeSlot =						{ this, &LoadTextureComponentInstance::changeTheme };

		nap::APIComponentInstance* mAPIComponent = nullptr;						//< Pointer to the api component
		const napkin::AppletExtension* mExtension = nullptr;					//< Applet extension

		// Current selected type
		EType mSelectedType = EType::None;

		// GUI service
		nap::IMGuiService* mGUIService;
	};
}
