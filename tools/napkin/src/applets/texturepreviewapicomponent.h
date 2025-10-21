/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local includes
#include "texturepreviewload2dcomponent.h"
#include "texturepreviewloadcubecomponent.h"

// External includes
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
	class AppletExtension;
	class TexturePreviewApplet;
	class TexturePreviewAPIComponentInstance;

	/**
	 * Loads and sets a texture or mesh from an API command received from Napkin.
	 */
	class TexturePreviewAPIComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(TexturePreviewAPIComponent, TexturePreviewAPIComponentInstance)
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
		nap::ComponentPtr<TexturePreviewLoad2DComponent> mLoad2DTextureComponent;		///< Property: 'Load2DTextureComponent' The component that loads and frames the 2D texture
		nap::ComponentPtr<TexturePreviewLoadCubeComponent> mLoadCubeTextureComponent;	///< Property: 'LoadCubemapComponent' The component that loads and frames the cubemap

		// Requires an api component
		void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;
	};


	/**
	 * Loads and sets a texture or mesh from an API command received from Napkin.
	 */
	class TexturePreviewAPIComponentInstance : public ComponentInstance
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

		TexturePreviewAPIComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource) { }

		// Init
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Current loaded texture type
		 * @return loaded texture type, none when no texture is loaded
		 */
		TexturePreviewAPIComponentInstance::EType getType() const { return mSelectedType; }

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
		ComponentInstancePtr<TexturePreviewLoad2DComponent> mLoad2DComponent = { this, &TexturePreviewAPIComponent::mLoad2DTextureComponent };

		// The resolved cubemap frame component
		ComponentInstancePtr<TexturePreviewLoadCubeComponent> mLoadCubeComponent = { this, &TexturePreviewAPIComponent::mLoadCubeTextureComponent };

	private:
		void loadTexture(const nap::APIEvent& apiEvent);						//< Loads a texture from JSON
		nap::Slot<const nap::APIEvent&> mTextureLoadRequested =					{ this, &TexturePreviewAPIComponentInstance::loadTexture };

		void loadMesh(const nap::APIEvent& apiEvent);							//< Loads a mesh from JSON
		nap::Slot<const nap::APIEvent&> mMeshLoadRequested =					{ this, &TexturePreviewAPIComponentInstance::loadMesh };
		
		void clear(const nap::APIEvent& apiEvent);								//< Clears texture
		nap::Slot<const nap::APIEvent&> mClearRequestedSlot =					{ this, &TexturePreviewAPIComponentInstance::clear };

		void changeTheme(const nap::APIEvent& apiEvent);						//< Changes theme
		nap::Slot<const nap::APIEvent&> mChangeThemeSlot =						{ this, &TexturePreviewAPIComponentInstance::changeTheme };

		nap::APIComponentInstance* mAPIComponent = nullptr;						//< Pointer to the api component
		const napkin::AppletExtension* mExtension = nullptr;					//< Applet extension

		// Current selected type
		EType mSelectedType = EType::None;

		// GUI service
		nap::IMGuiService* mGUIService = nullptr;
	};
}
