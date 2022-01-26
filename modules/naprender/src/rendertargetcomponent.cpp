/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "rendertargetcomponent.h"
#include "rendertarget.h"
#include "renderservice.h"
#include "valuegpubuffer.h"
#include "renderglobals.h"
#include "uniforminstance.h"
#include "renderglobals.h"

// External Includes
#include <entity.h>
#include <glm/gtc/matrix_transform.hpp>
#include <entity.h>
#include <nap/core.h>
#include <orthocameracomponent.h>

RTTI_BEGIN_CLASS(nap::RenderTargetComponent)
	RTTI_PROPERTY("ReferenceWindow",			&nap::RenderTargetComponent::mReferenceWindow,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("OutputTexture",				&nap::RenderTargetComponent::mOutputTexture,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Samples",					&nap::RenderTargetComponent::mRequestedSamples,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ClearColor",					&nap::RenderTargetComponent::mClearColor,					nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderTargetComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS


//////////////////////////////////////////////////////////////////////////

namespace nap
{
	RenderTargetComponentInstance::RenderTargetComponentInstance(EntityInstance& entity, Component& resource) :
		ComponentInstance(entity, resource),
		mRenderTarget(*entity.getCore())
	{ }


	bool RenderTargetComponentInstance::init(utility::ErrorState& errorState)
	{
		if (!ComponentInstance::init(errorState))
			return false;

		// Get resource
		RenderTargetComponent* resource = getComponent<RenderTargetComponent>();

		// Ensure an output texture or reference window is specified
		if (!errorState.check(resource->mOutputTexture != nullptr && resource->mReferenceWindow != nullptr, "Needs either 'OutputTexture' or 'referenceWindow' to be assigned."))
			return false;

		// Sync up with reference window
		if (resource->mReferenceWindow != nullptr)
		{
			// Resize the texture if there is one
			if (resource->mOutputTexture != nullptr)
			{

			}

			// Otherwise create one
			else
			{

			}
		}

		// At this point a texture was specified or created

		// Create the render target, link in the output texture
		mRenderTarget.mClearColor = resource->mClearColor.convert<RGBAColorFloat>();
		mRenderTarget.mColorTexture = resource->mOutputTexture;
		mRenderTarget.mSampleShading = true;
		mRenderTarget.mRequestedSamples = resource->mRequestedSamples;

		// Initialize target
		if (!mRenderTarget.init(errorState))
			return false;

		// Get render service
		mRenderService = getEntityInstance()->getCore()->getService<RenderService>();
		assert(mService != nullptr);

		return true;
	}


	IRenderTarget& RenderTargetComponentInstance::getTarget()
	{
		return mRenderTarget;
	}


	RenderTexture2D& RenderTargetComponentInstance::getOutputTexture()
	{
		return mRenderTarget.getColorTexture();
	}
}
