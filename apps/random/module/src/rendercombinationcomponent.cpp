#include "rendercombinationcomponent.h"

// External Includes
#include <entity.h>
#include <renderablemeshcomponent.h>
#include <nap/core.h>

// nap::rendercombinationcomponent run time class definition 
RTTI_BEGIN_CLASS(nap::RenderCombinationComponent)
	RTTI_PROPERTY("Target",		&nap::RenderCombinationComponent::mRenderTarget,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Bitmap",		&nap::RenderCombinationComponent::mBitmap,			nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::rendercombinationcomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderCombinationComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void RenderCombinationComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(RenderableMeshComponent));
	}


	bool RenderCombinationComponentInstance::init(utility::ErrorState& errorState)
	{
		// Copy over render target
		mRenderTarget = getComponent<RenderCombinationComponent>()->mRenderTarget.get();

		// Find the renderable mesh component
		mRenderableMesh = &(this->getEntityInstance()->getComponent<RenderableMeshComponentInstance>());

		// Get the render service
		mRenderService = getEntityInstance()->getCore()->getService<nap::RenderService>();

		// Get the bitmap to store rendered result CPU side
		mBitmap = getComponent<RenderCombinationComponent>()->mBitmap.get();

		return true;
	}


	void RenderCombinationComponentInstance::update(double deltaTime)
	{
		// Copy data back into bitmap after render pass
		if (mTransferring)
		{
#ifndef _DEBUG 
			mRenderTarget->getColorTexture().endGetData(*mBitmap);
#else
			mRenderTarget->getColorTexture().getData(*mBitmap);
#endif // DEBUG

		}
		mTransferring = false;
	}


	void RenderCombinationComponentInstance::render(OrthoCameraComponentInstance& orthoCamera)
	{
		mRenderService->clearRenderTarget(mRenderTarget->getTarget());

		// Find the projection plane and render it to the back-buffer
		std::vector<nap::RenderableComponentInstance*> components_to_render;
		components_to_render.emplace_back(mRenderableMesh);

		// Render clouds plane to clouds texture
		mRenderService->renderObjects(mRenderTarget->getTarget(), orthoCamera, components_to_render);
		
#ifndef _DEBUG
		mRenderTarget->getColorTexture().startGetData();
#endif // !_DEBUG
		mTransferring = true;
	}

}