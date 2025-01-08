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
#include <renderglobals.h>

// nap::loadtexturecomponent run time class definition 
RTTI_BEGIN_CLASS(napkin::LoadTextureComponent)
	RTTI_PROPERTY("Frame2DTextureComponent",	&napkin::LoadTextureComponent::mFrame2DTextureComponent,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FrameCubemapComponent",		&napkin::LoadTextureComponent::mFrameCubemapComponent,		nap::rtti::EPropertyMetaData::Required)
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

		const auto* load_texture_sig = mAPIComponent->findSignature(LoadTextureComponent::loadTextureCmd);
		if (!errorState.check(load_texture_sig != nullptr, "Missing '%s' cmd signature", LoadTextureComponent::loadTextureCmd))
			return false;

		const auto* clear_sig = mAPIComponent->findSignature(LoadTextureComponent::clearCmd);
		if (!errorState.check(clear_sig != nullptr, "Missing '%s' cmd signature", LoadTextureComponent::clearCmd))
			return false;

		const auto* load_mesh_sig = mAPIComponent->findSignature(LoadTextureComponent::loadMeshCmd);
		if (!errorState.check(load_mesh_sig != nullptr, "Missing '%s' cmd signature", LoadTextureComponent::loadMeshCmd))
			return false;

		// Register api cmd listeners
		mAPIComponent->registerCallback(*load_texture_sig, mTextureLoadRequested);
		mAPIComponent->registerCallback(*load_mesh_sig, mMeshLoadRequested);
		mAPIComponent->registerCallback(*clear_sig, mClearRequestedSlot);

		return true;
	}


	void LoadTextureComponentInstance::loadTexture(const nap::APIEvent& apiEvent)
	{
		auto* data_arg = apiEvent.getArgumentByName(LoadTextureComponent::loadTextureArg1);
		assert(data_arg != nullptr);

		// De-serialize JSON
		nap::Core& core = *getEntityInstance()->getCore();
		utility::ErrorState error; rtti::DeserializeResult result;
		if (!rtti::deserializeJSON(data_arg->asString(), rtti::EPropertyValidationMode::DisallowMissingProperties,
			rtti::EPointerPropertyMode::OnlyRawPointers, core.getResourceManager()->getFactory(), result, error))
		{
			error.fail("%s cmd failed", LoadTextureComponent::loadTextureCmd);
			nap::Logger::error(error.toString());
			return;
		}

		// Ensure there's at least 1 object and it's of type texture
		if (result.mReadObjects.size() == 0)
		{
			nap::Logger::error("%s cmd failed: invalid payload", LoadTextureComponent::loadTextureCmd);
			return;
		}

		// Ensure type is Texture2D or Cubemap
		if (!result.mReadObjects[0]->get_type().is_derived_from(RTTI_OF(nap::Texture2D)) &&
			!result.mReadObjects[0]->get_type().is_derived_from(RTTI_OF(nap::TextureCube)))
		{
			nap::Logger::error("%s cmd failed: unsupported texture type", LoadTextureComponent::loadTextureCmd);
			return;
		}

		// Warn if there's more than 1 object and store
		if (result.mReadObjects.size() > 1)
			nap::Logger::warn("%s cmd holds multiple objects, initializing first one...", LoadTextureComponent::loadTextureCmd);

		// Init texture relative to project data directory (thread-safe)
		{
			assert(!mProjectDataDirectory.empty());
			napkin::CWDHandle cwd_handle(mProjectDataDirectory);
			if (!result.mReadObjects[0]->init(error))
			{
				nap::Logger::error(error.toString().c_str());
				return;
			}
		}

		// Select and bind as active texture
		if (result.mReadObjects[0]->get_type().is_derived_from(RTTI_OF(Texture2D)))
		{
			std::unique_ptr<Texture2D> tex = rtti_cast<Texture2D>(result.mReadObjects[0]);
			mFrame2DTextureComponent->load(std::move(tex));
			mSelectedType = EType::Texture2D;
		}
		else
		{
			std::unique_ptr<TextureCube> tex = rtti_cast<TextureCube>(result.mReadObjects[0]);
			assert(tex != nullptr);
			mFrameCubeComponent->load(std::move(tex));
			mSelectedType = EType::Cubemap;
		}

		// Check if we need to re-frame camera if requested
		auto* frame_arg = apiEvent.getArgumentByName(LoadTextureComponent::loadTextureArg2);
		assert(frame_arg != nullptr);
		if (frame_arg->asBool())
			frame();
	}


	void LoadTextureComponentInstance::loadMesh(const nap::APIEvent& apiEvent)
	{
		// Bail if no texture is loaded
		if (getType() == EType::None)
		{
			nap::Logger::warn("%s cmd failed: can't assign mesh, no texture loaded", LoadTextureComponent::loadMeshCmd);
			return;
		}

		// Get mesh data
		auto* data_arg = apiEvent.getArgumentByName(LoadTextureComponent::loadMeshArg1);
		assert(data_arg != nullptr);

		// De-serialize mesh
		nap::Core& core = *getEntityInstance()->getCore();
		utility::ErrorState error; rtti::DeserializeResult result;
		if (!rtti::deserializeJSON(data_arg->asString(), rtti::EPropertyValidationMode::DisallowMissingProperties,
			rtti::EPointerPropertyMode::OnlyRawPointers, core.getResourceManager()->getFactory(), result, error))
		{
			error.fail("%s cmd failed", LoadTextureComponent::loadMeshCmd);
			nap::Logger::error(error.toString());
			return;
		}

		// Ensure type is a mesh
		if (!result.mReadObjects[0]->get_type().is_derived_from(RTTI_OF(nap::IMesh)))
		{
			nap::Logger::error("%s cmd failed: unsupported type", LoadTextureComponent::loadMeshCmd);
			return;
		}

		// Warn if there's more than 1 object and store
		if (result.mReadObjects.size() > 1)
			nap::Logger::warn("%s cmd holds multiple objects, initializing first one...", LoadTextureComponent::loadMeshCmd);

		// Init mesh relative to project data directory (thread-safe)
		{
			assert(!mProjectDataDirectory.empty());
			napkin::CWDHandle cwd_handle(mProjectDataDirectory);
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
				// Make sure the mesh has a uv channel to bind to
				if (!mFrame2DTextureComponent->load(std::move(new_mesh), error))
				{
					nap::Logger::error("%s cmd failed: %s", LoadTextureComponent::loadMeshCmd, error.toString().c_str());
					break;
				}
				break;
			}
			case EType::Cubemap:
			{
				if (!mFrameCubeComponent->load(std::move(new_mesh), error))
				{
					nap::Logger::error("%s cmd failed: %s", LoadTextureComponent::loadMeshCmd, error.toString().c_str());
					break;
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


	const nap::Texture* LoadTextureComponentInstance::getTexture() const
	{
		switch (getType())
		{
		case EType::Texture2D:
			return &mFrame2DTextureComponent->getTexture();
			break;
		case EType::Cubemap:
			return &mFrameCubeComponent->getTexture();
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


	const IMesh* LoadTextureComponentInstance::getMesh() const
	{
		switch (getType())
		{
		case EType::Texture2D:
			return &mFrame2DTextureComponent->getMesh();
			break;
		case EType::Cubemap:
			return &mFrameCubeComponent->getMesh();
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


	void LoadTextureComponentInstance::frame()
	{
		switch (getType())
		{
			case EType::Texture2D:
				mFrame2DTextureComponent->frame();
				break;
			case EType::Cubemap:
				mFrameCubeComponent->frame();
				break;
			default:
				assert(false);
				break;
		}
	}


	float LoadTextureComponentInstance::getOpacity() const
	{
		switch (getType())
		{
			case EType::Texture2D:
				return mFrame2DTextureComponent->getOpacity();
			case EType::Cubemap:
				return mFrameCubeComponent->getOpacity();
			default:
				assert(false);
				return 1.0f;
		}
	}


	float LoadTextureComponentInstance::getRotate() const
	{
		switch (getType())
		{
		case EType::Cubemap:
			return mFrameCubeComponent->getRotation();
		case EType::Texture2D:
			return mFrame2DTextureComponent->getRotation();
		default:
			assert(false);
			return 0.0f;
		}
	}


	void LoadTextureComponentInstance::processWindowEvents(nap::InputService& inputService, nap::RenderWindow& window)
	{
		switch (getType())
		{
		case EType::Texture2D:
			mFrame2DTextureComponent->processWindowEvents(inputService, window);
			break;
		case EType::Cubemap:
			mFrameCubeComponent->processWindowEvents(inputService, window);
			break;
		default:
			assert(false);
			break;
		}
	}


	void LoadTextureComponentInstance::draw(RenderService& renderService, RenderWindow& window)
	{
		switch (getType())
		{
		case EType::Texture2D:
			mFrame2DTextureComponent->draw(renderService, window);
			break;
		case EType::Cubemap:
			mFrameCubeComponent->draw(renderService, window);
			break;
		case EType::None:
			break;
		default:
			assert(false);
			break;
		}
	}


	void LoadTextureComponentInstance::setOpacity(float alpha)
	{
		switch (getType())
		{
			case EType::Texture2D:
				mFrame2DTextureComponent->setOpacity(alpha);
				break;
			case EType::Cubemap:
				mFrameCubeComponent->setOpacity(alpha);
				break;
			default:
				assert(false);
				break;
		}
	}


	void LoadTextureComponentInstance::setRotate(float speed)
	{
		switch (getType())
		{
			case EType::Cubemap:
				mFrameCubeComponent->setRotation(speed);
				break;
			case EType::Texture2D:
				mFrame2DTextureComponent->setRotation(speed);
				break;
			default:
				assert(false);
				break;
		}
	}


	void LoadTextureComponentInstance::clear(const nap::APIEvent& apiEvent)
	{
		mSelectedType = EType::None;
	}
}
