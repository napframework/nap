#include "rendervideocomponent.h"

// External Includes
#include <entity.h>
#include <renderservice.h>
#include <nap/core.h>
#include <rendertexture2d.h>

// nap::rendervideocomponent run time class definition 
RTTI_BEGIN_CLASS(nap::RenderVideoComponent)
	RTTI_PROPERTY("Player",				&nap::RenderVideoComponent::mVideoPlayer,				nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Target",				&nap::RenderVideoComponent::mTarget,					nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("RenderComponent",	&nap::RenderVideoComponent::mRenderableComponent,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("CameraComponent",	&nap::RenderVideoComponent::mCameraComponent,			nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::rendervideocomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderVideoComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void RenderVideoComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{

	}


	bool RenderVideoComponentInstance::init(utility::ErrorState& errorState)
	{
		// Fetch render service
		mRenderService = getEntityInstance()->getCore()->getService<RenderService>();
		if (!errorState.check(mRenderService != nullptr, "Unable to find render service: %s", this->mID.c_str()))
			return false;

		// Copy over resource links
		mTarget = getComponent<RenderVideoComponent>()->mTarget.get();
		mVideoPlayer = getComponent<RenderVideoComponent>()->mVideoPlayer.get();

		return true;
	}


	void RenderVideoComponentInstance::update(double deltaTime)
	{

	}


	void RenderVideoComponentInstance::render()
	{
		// Get plane to render
		RenderableMeshComponentInstance& render_plane = *mRenderableComponent;

		// Uniform texture inputs
		UniformTexture2D& texy = render_plane.getMaterialInstance().getOrCreateUniform<UniformTexture2D>("yTexture");
		UniformTexture2D& texu = render_plane.getMaterialInstance().getOrCreateUniform<UniformTexture2D>("uTexture");
		UniformTexture2D& texv = render_plane.getMaterialInstance().getOrCreateUniform<UniformTexture2D>("vTexture");

		texy.setTexture(mVideoPlayer->getYTexture());
		texu.setTexture(mVideoPlayer->getUTexture());
		texv.setTexture(mVideoPlayer->getVTexture());

		// Clear target we want to render in to
		mRenderService->clearRenderTarget(mTarget->getTarget(), opengl::EClearFlags::Color | opengl::EClearFlags::Depth);

		// Render off screen surface
		std::vector<nap::RenderableComponentInstance*> obj_to_render = { &render_plane };
		mRenderService->renderObjects(mTarget->getTarget(), *mCameraComponent, obj_to_render);
	}

}