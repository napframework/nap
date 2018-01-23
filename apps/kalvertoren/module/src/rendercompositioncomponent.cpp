#include "rendercompositioncomponent.h"

// External Includes
#include <entity.h>
#include <renderservice.h>
#include <nap/core.h>

// nap::rendercompositioncomponent run time class definition 
RTTI_BEGIN_CLASS(nap::RenderCompositionComponent)
	RTTI_PROPERTY("TargetOne",				&nap::RenderCompositionComponent::mTargetA,					nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("TargetTwo",				&nap::RenderCompositionComponent::mTargetB,					nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("CompositionComponent",	&nap::RenderCompositionComponent::mCompositionComponent,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("RenderComponent",		&nap::RenderCompositionComponent::mRenderableComponent,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("CameraComponent",		&nap::RenderCompositionComponent::mCameraComponent,			nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::rendercompositioncomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderCompositionComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void RenderCompositionComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{

	}


	bool RenderCompositionComponentInstance::init(utility::ErrorState& errorState)
	{
		mTargetA = getComponent<RenderCompositionComponent>()->mTargetA.get();
		mTargetB = getComponent<RenderCompositionComponent>()->mTargetB.get();
		activeTarget = mTargetA;
		nextTarget = mTargetB;

		mRenderService = getEntityInstance()->getCore()->getService<RenderService>();
		if (!errorState.check(mRenderService != nullptr, "Unable to find render service: %s", this->mID.c_str()))
			return false;

		return true;
	}


	void RenderCompositionComponentInstance::update(double deltaTime)
	{
		if (mTransferring)
		{
#ifndef _DEBUG 
			activeTarget->getColorTexture().endGetData(mPixmap);
#else
			activeTarget->getColorTexture().getData(mPixmap);
#endif // DEBUG

		}
		mTransferring = false;
	}


	void RenderCompositionComponentInstance::render()
	{
		nap::CompositionInstance& comp = mCompositionComponent->getSelection();

		// Assign inputs for first pass
		assert(comp.getLayerCount() > 0);
		inputA = &(comp.getLayer(0).getTexture());
		inputB = comp.getLayerCount() > 1 ? &(comp.getLayer(1).getTexture()) : inputA;

		// Set the active and next target
		activeTarget = mTargetA;
		nextTarget   = mTargetB;
		
		// Render first pass to target a
		renderPass(*inputA, *inputB, *activeTarget);

		// Ping pong successive render layers
		for (int i = 2; i < comp.getLayerCount(); i++)
		{
			// The texture of the current active target is the base layer of the next render pass
			inputA = &(activeTarget->getColorTexture());
			inputB = &(comp.getLayer(i).getTexture());

			// Swap targets, currently active target is the one we labeled next
			RenderTarget* active_placeholder = activeTarget;
			activeTarget = nextTarget;

			// Now render
			renderPass(*inputA, *inputB, *activeTarget);

			// Assign target for next round to be placeholder
			nextTarget = active_placeholder;
		}

		// Start pixel data transfer so reading it later is performent
#ifndef _DEBUG 
		activeTarget->getColorTexture().startGetData();
#endif // !DEBUG
		mTransferring = true;
	}

	nap::BaseTexture2D& RenderCompositionComponentInstance::getTexture()
	{
		return activeTarget->getColorTexture();
	}


	nap::Pixmap& RenderCompositionComponentInstance::getPixmap()
	{
		return mPixmap;
	}


	void RenderCompositionComponentInstance::renderPass(BaseTexture2D& inputA, BaseTexture2D& inputB, RenderTarget& target)
	{
		// Get plane to render
		RenderableMeshComponentInstance& render_plane = *mRenderableComponent;

		// Uniform texture inputs
		UniformTexture2D& texa = render_plane.getMaterialInstance().getOrCreateUniform<UniformTexture2D>("imageA");
		UniformTexture2D& texb = render_plane.getMaterialInstance().getOrCreateUniform<UniformTexture2D>("imageB");
		texa.setTexture(inputA);
		texb.setTexture(inputB);

		// Clear target we want to render in to
		mRenderService->clearRenderTarget(target.getTarget(), opengl::EClearFlags::COLOR | opengl::EClearFlags::DEPTH);

		// Render off screen surface
		std::vector<nap::RenderableComponentInstance*> obj_to_render = { &render_plane };
		mRenderService->renderObjects(target.getTarget(), *mCameraComponent, obj_to_render);
	}

}