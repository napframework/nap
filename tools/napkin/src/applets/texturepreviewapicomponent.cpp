/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "texturepreviewapicomponent.h"
#include "../appletextension.h"
#include "../naputils.h"

// External Includes
#include <entity.h>
#include <rtti/jsonreader.h>
#include <nap/core.h>
#include <nap/logger.h>
#include <renderglobals.h>
#include <imguiservice.h>

// nap::loadtexturecomponent run time class definition 
RTTI_BEGIN_CLASS(napkin::TexturePreviewAPIComponent)
	RTTI_PROPERTY("Load2DTextureComponent",	&napkin::TexturePreviewAPIComponent::mLoad2DTextureComponent,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("LoadCubemapComponent",	&napkin::TexturePreviewAPIComponent::mLoadCubeTextureComponent,		nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::loadtexturecomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(napkin::TexturePreviewAPIComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace napkin
{
	void TexturePreviewAPIComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(APIComponent));
	}


	bool TexturePreviewAPIComponentInstance::init(utility::ErrorState& errorState)
	{
		// Gui service
		mGUIService = getEntityInstance()->getCore()->getService<nap::IMGuiService>();
		assert(mGUIService != nullptr);

		mAPIComponent = getEntityInstance()->findComponent<APIComponentInstance>();
		if (!errorState.check(mAPIComponent != nullptr, "Missing API component"))
			return false;

		const auto* load_texture_sig = mAPIComponent->findSignature(TexturePreviewAPIComponent::loadTextureCmd);
		if (!errorState.check(load_texture_sig != nullptr, "Missing '%s' cmd signature", TexturePreviewAPIComponent::loadTextureCmd))
			return false;

		const auto* clear_sig = mAPIComponent->findSignature(TexturePreviewAPIComponent::clearCmd);
		if (!errorState.check(clear_sig != nullptr, "Missing '%s' cmd signature", TexturePreviewAPIComponent::clearCmd))
			return false;

		const auto* load_mesh_sig = mAPIComponent->findSignature(TexturePreviewAPIComponent::loadMeshCmd);
		if (!errorState.check(load_mesh_sig != nullptr, "Missing '%s' cmd signature", TexturePreviewAPIComponent::loadMeshCmd))
			return false;

		const auto* change_theme_sig = mAPIComponent->findSignature(TexturePreviewAPIComponent::changeThemeCmd);
		if (!errorState.check(change_theme_sig != nullptr, "Missing '%s' cmd signature", TexturePreviewAPIComponent::changeThemeCmd))
			return false;

		mExtension = &getEntityInstance()->getCore()->getExtension<napkin::AppletExtension>();
		if (!errorState.check(mExtension != nullptr && mExtension->hasProject(),
			"Unable to resolve editor project information"))
			return false;

		// Register api cmd listeners
		mAPIComponent->registerCallback(*load_texture_sig, mTextureLoadRequested);
		mAPIComponent->registerCallback(*load_mesh_sig, mMeshLoadRequested);
		mAPIComponent->registerCallback(*clear_sig, mClearRequestedSlot);
		mAPIComponent->registerCallback(*change_theme_sig, mChangeThemeSlot);

		return true;
	}


	void TexturePreviewAPIComponentInstance::loadTexture(const nap::APIEvent& apiEvent)
	{
		auto* data_arg = apiEvent.getArgumentByName(TexturePreviewAPIComponent::loadTextureArg1);
		assert(data_arg != nullptr);

		// De-serialize JSON
		nap::Core& core = *getEntityInstance()->getCore();
		utility::ErrorState error; rtti::DeserializeResult result;
		if (!rtti::deserializeJSON(data_arg->asString(), rtti::EPropertyValidationMode::DisallowMissingProperties,
			rtti::EPointerPropertyMode::OnlyRawPointers, core.getResourceManager()->getFactory(), result, error))
		{
			error.fail("%s cmd failed", TexturePreviewAPIComponent::loadTextureCmd);
			nap::Logger::error(error.toString());
			return;
		}

		// Ensure there's at least 1 object and it's of type texture
		if (result.mReadObjects.empty())
		{
			nap::Logger::error("%s cmd failed: invalid payload", TexturePreviewAPIComponent::loadTextureCmd);
			return;
		}

		// Ensure type is Texture2D or Cubemap
		if (!result.mReadObjects[0]->get_type().is_derived_from(RTTI_OF(nap::Texture2D)) &&
			!result.mReadObjects[0]->get_type().is_derived_from(RTTI_OF(nap::TextureCube)))
		{
			nap::Logger::error("%s cmd failed: unsupported texture type", TexturePreviewAPIComponent::loadTextureCmd);
			return;
		}

		// Warn if there's more than 1 object and store
		if (result.mReadObjects.size() > 1)
			nap::Logger::warn("%s cmd holds multiple objects, initializing first one...", TexturePreviewAPIComponent::loadTextureCmd);

		// Init texture relative to project data directory (thread-safe)
		{
			napkin::CWDHandle cwd_handle(mExtension->switchWorkingDir());
			if (!result.mReadObjects[0]->init(error))
			{
				nap::Logger::error(error.toString().c_str());
				return;
			}
		}

		// Check if we need to re-frame camera if requested
		auto* frame_arg = apiEvent.getArgumentByName(TexturePreviewAPIComponent::loadTextureArg2);
		assert(frame_arg != nullptr);

		// Select and bind as active texture
		if (result.mReadObjects[0]->get_type().is_derived_from(RTTI_OF(Texture2D)))
		{
			std::unique_ptr<Texture2D> tex = rtti_cast<Texture2D>(result.mReadObjects[0]);
			mLoad2DComponent->load(std::move(tex));
			mSelectedType = EType::Texture2D;
			if(frame_arg->asBool())
			{
				mLoad2DComponent->setMode(TexturePreviewLoad2DComponentInstance::EMode::Plane);
				mLoad2DComponent->frame();
			}
		}
		else
		{
			std::unique_ptr<TextureCube> tex = rtti_cast<TextureCube>(result.mReadObjects[0]);
			assert(tex != nullptr);
			mLoadCubeComponent->load(std::move(tex));
			mSelectedType = EType::Cubemap;
			if (frame_arg->asBool())
				mLoadCubeComponent->frame();
		}
	}


	void TexturePreviewAPIComponentInstance::loadMesh(const nap::APIEvent& apiEvent)
	{
		// Bail if no texture is loaded
		if (getType() == EType::None)
		{
			nap::Logger::warn("%s cmd failed: can't assign mesh, no texture loaded", TexturePreviewAPIComponent::loadMeshCmd);
			return;
		}

		// Get mesh data
		auto* data_arg = apiEvent.getArgumentByName(TexturePreviewAPIComponent::loadMeshArg1);
		assert(data_arg != nullptr);

		// De-serialize mesh
		nap::Core& core = *getEntityInstance()->getCore();
		utility::ErrorState error; rtti::DeserializeResult result;
		if (!rtti::deserializeJSON(data_arg->asString(), rtti::EPropertyValidationMode::DisallowMissingProperties,
			rtti::EPointerPropertyMode::OnlyRawPointers, core.getResourceManager()->getFactory(), result, error))
		{
			error.fail("%s cmd failed", TexturePreviewAPIComponent::loadMeshCmd);
			nap::Logger::error(error.toString());
			return;
		}

		// Ensure there's at least one object
		if (result.mReadObjects.empty())
		{
			nap::Logger::error("%s cmd failed: invalid payload", TexturePreviewAPIComponent::loadMeshCmd);
			return;
		}

		// Ensure type is a mesh
		if (!result.mReadObjects[0]->get_type().is_derived_from(RTTI_OF(nap::IMesh)))
		{
			nap::Logger::error("%s cmd failed: unsupported type", TexturePreviewAPIComponent::loadMeshCmd);
			return;
		}

		// Warn if there's more than 1 object and store
		if (result.mReadObjects.size() > 1)
			nap::Logger::warn("%s cmd holds multiple objects, initializing first one...", TexturePreviewAPIComponent::loadMeshCmd);

		// Init mesh relative to project data directory (thread-safe)
		{
			napkin::CWDHandle cwd_handle = mExtension->switchWorkingDir();
			if (!result.mReadObjects[0]->init(error))
			{
				nap::Logger::error(error.toString().c_str());
				return;
			}
		}

		// Select and set mesh based on selected type
		auto new_mesh = rtti_cast<nap::IMesh>(result.mReadObjects[0]);
		switch (getType())
		{
			case EType::Texture2D:
			{
				// Load user mesh into 2D texture visualizer -> bail if mesh can't be loaded
				int idx = mLoad2DComponent->load(std::move(new_mesh), error);
				if (idx < 0)
				{
					nap::Logger::error("%s cmd failed: %s", TexturePreviewAPIComponent::loadMeshCmd, error.toString().c_str());
					break;
				}

				// Select and frame when requested
				auto* frame_arg = apiEvent.getArgumentByName(TexturePreviewAPIComponent::loadMeshArg2);
				if (frame_arg->asBool())
				{
					mLoad2DComponent->setMode(TexturePreviewLoad2DComponentInstance::EMode::Mesh);
					mLoad2DComponent->setMeshIndex(idx);
					mLoad2DComponent->frame();
				}
				break;
			}
			case EType::Cubemap:
			{
				// Load user mesh into cubemap visualizer
				int idx = mLoadCubeComponent->load(std::move(new_mesh), error);
				if (idx < 0)
				{
					nap::Logger::error("%s cmd failed: %s", TexturePreviewAPIComponent::loadMeshCmd, error.toString().c_str());
					break;
				}

				// Select and frame when requested
				auto* frame_arg = apiEvent.getArgumentByName(TexturePreviewAPIComponent::loadMeshArg2);
				if (frame_arg->asBool())
				{
					mLoadCubeComponent->setMeshIndex(idx);
					mLoadCubeComponent->frame();
				}
				break;
			}
			default:
			{
				assert(false);
				break;
			}
		}
	}


	const nap::Texture* TexturePreviewAPIComponentInstance::getTexture() const
	{
		switch (getType())
		{
		case EType::Texture2D:
			return &mLoad2DComponent->getTexture();
			break;
		case EType::Cubemap:
			return &mLoadCubeComponent->getTexture();
			break;
		case EType::None:
			return nullptr;
			break;
		default:
			assert(false);
			break;
		}
		return nullptr;
	}


	const IMesh* TexturePreviewAPIComponentInstance::getMesh() const
	{
		switch (getType())
		{
		case EType::Texture2D:
			return &mLoad2DComponent->getMesh();
			break;
		case EType::Cubemap:
			return &mLoadCubeComponent->getMesh();
			break;
		case EType::None:
			return nullptr;
			break;
		default:
			assert(false);
			break;
		}
		return nullptr;
	}


	const math::Box& TexturePreviewAPIComponentInstance::getMeshBounds() const
	{	
		switch (getType())
		{
		case EType::Texture2D:
			return mLoad2DComponent->getBounds();
		case EType::Cubemap:
			return mLoadCubeComponent->getBounds();
		default:
			assert(false);
			break;
		}
		const static math::Box invalid = { 0, 0, 0 };
		return invalid;
	}


	void TexturePreviewAPIComponentInstance::frame()
	{
		switch (getType())
		{
			case EType::Texture2D:
				mLoad2DComponent->frame();
				break;
			case EType::Cubemap:
				mLoadCubeComponent->frame();
				break;
			default:
				assert(false);
				break;
		}
	}


	float TexturePreviewAPIComponentInstance::getOpacity() const
	{
		switch (getType())
		{
			case EType::Texture2D:
				return mLoad2DComponent->getOpacity();
			case EType::Cubemap:
				return mLoadCubeComponent->getOpacity();
			default:
				assert(false);
				return 1.0f;
		}
	}


	float TexturePreviewAPIComponentInstance::getRotate() const
	{
		switch (getType())
		{
		case EType::Cubemap:
			return mLoadCubeComponent->getRotation();
		case EType::Texture2D:
			return mLoad2DComponent->getRotation();
		default:
			assert(false);
			return 0.0f;
		}
	}


	void TexturePreviewAPIComponentInstance::processWindowEvents(nap::InputService& inputService, nap::RenderWindow& window)
	{
		switch (getType())
		{
		case EType::Texture2D:
			mLoad2DComponent->processWindowEvents(inputService, window);
			break;
		case EType::Cubemap:
			mLoadCubeComponent->processWindowEvents(inputService, window);
			break;
		case EType::None:
			break;
		default:
			assert(false);
			break;
		}
	}


	void TexturePreviewAPIComponentInstance::draw(RenderService& renderService, RenderWindow& window)
	{
		switch (getType())
		{
		case EType::Texture2D:
			mLoad2DComponent->draw(renderService, window);
			break;
		case EType::Cubemap:
			mLoadCubeComponent->draw(renderService, window);
			break;
		case EType::None:
			break;
		default:
			assert(false);
			break;
		}
	}


	void TexturePreviewAPIComponentInstance::setOpacity(float alpha)
	{
		switch (getType())
		{
			case EType::Texture2D:
				mLoad2DComponent->setOpacity(alpha);
				break;
			case EType::Cubemap:
				mLoadCubeComponent->setOpacity(alpha);
				break;
			default:
				assert(false);
				break;
		}
	}


	void TexturePreviewAPIComponentInstance::setRotate(float speed)
	{
		switch (getType())
		{
			case EType::Cubemap:
				mLoadCubeComponent->setRotation(speed);
				break;
			case EType::Texture2D:
				mLoad2DComponent->setRotation(speed);
				break;
			default:
				assert(false);
				break;
		}
	}


	void TexturePreviewAPIComponentInstance::clear(const nap::APIEvent& apiEvent)
	{
		mSelectedType = EType::None;
	}


	void TexturePreviewAPIComponentInstance::changeTheme(const nap::APIEvent& apiEvent)
	{
		auto theme_arg = apiEvent.getArgumentByName(TexturePreviewAPIComponent::changeThemeArg1);
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
