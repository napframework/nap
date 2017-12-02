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

		mRenderService = getEntityInstance()->getCore()->getService<RenderService>();
		if (!errorState.check(mRenderService != nullptr, "Unable to find render service: %s", this->mID.c_str()))
			return false;

		return true;
	}


	void RenderCompositionComponentInstance::render()
	{
		RenderCompositionComponentInstance& comp_render = getEntityInstance()->getComponent<RenderCompositionComponentInstance>();

		// Clear target A
		mRenderService->clearRenderTarget(mTargetA->getTarget(), opengl::EClearFlags::COLOR | opengl::EClearFlags::DEPTH);

		// Get plane to render
		RenderableMeshComponentInstance& render_plane = *mRenderableComponent;

		UniformTexture2D& texa = render_plane.getMaterialInstance().getOrCreateUniform<UniformTexture2D>("imageA");
		UniformTexture2D& texb = render_plane.getMaterialInstance().getOrCreateUniform<UniformTexture2D>("imageB");

		nap::Composition& comp = mCompositionComponent->getSelection();

		texa.setTexture(comp.getLayer(0).getTexture());
		texb.setTexture(comp.getLayer(1).getTexture());

		// Get camera to render
		OrthoCameraComponentInstance& ortho_cam = *mCameraComponent;

		// Render offscreen surface
		std::vector<nap::RenderableComponentInstance*> blaat;
		blaat.emplace_back(&render_plane);
		mRenderService->renderObjects(mTargetA->getTarget(), ortho_cam, blaat);
	}

	nap::BaseTexture2D& RenderCompositionComponentInstance::getTexture()
	{
		return mTargetA->getColorTexture();
	}
}