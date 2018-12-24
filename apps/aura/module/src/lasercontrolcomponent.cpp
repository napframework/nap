#include "lasercontrolcomponent.h"
#include "renderablemeshcomponent.h"

// External Includes
#include <entity.h>
#include <nap/core.h>
#include <nap/resourcemanager.h>
#include <depthsorter.h>
#include <renderable2dtextcomponent.h>

RTTI_BEGIN_CLASS(nap::LaserConfiguration)
	RTTI_PROPERTY("Target",		&nap::LaserConfiguration::mTarget,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("LaserID",	&nap::LaserConfiguration::mLaserID,	nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::lasercontroller run time class definition 
RTTI_BEGIN_CLASS(nap::LaserControlComponent)
	RTTI_PROPERTY("Lasers",		&nap::LaserControlComponent::mLaserConfigurations,	nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::lasercontrollerInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::LaserControlInstanceComponent)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void LaserControlComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{

	}


	bool LaserControlInstanceComponent::init(utility::ErrorState& errorState)
	{
		// Get all compounds to use and create laser prototypes from
		nap::LaserControlComponent* resource = getComponent<LaserControlComponent>();
		mLaserConfigurations = resource->mLaserConfigurations;

		// Get resource manager that is used to spawn the new entity
		ResourceManager& resource_manager = *getEntityInstance()->getCore()->getResourceManager();

		if (!errorState.check(mLaserConfigurations.size() * 2 == getEntityInstance()->getEntity()->mChildren.size(), "Number of laser compounds does not match the laser entity children"))
			return false;

		// Get total number of lasers to create
		int laser_count = static_cast<int>(mLaserConfigurations.size());
		int current_count(0);
		int cols(2);
		int rows = laser_count / cols;
		
		for (int index = 0; index < mLaserConfigurations.size(); ++index)
		{
			auto& configuration = mLaserConfigurations[index];
			EntityInstance* laser_entity = getEntityInstance()->getChildren()[index * 2];
			EntityInstance* frame_entity = getEntityInstance()->getChildren()[index * 2 + 1];

			// Store for future use
			mLaserEntityMap.emplace(std::make_pair(configuration.mLaserID, laser_entity));
			mLaserConfigurationMap.emplace(std::make_pair(configuration.mLaserID, configuration));
			mLaserFrameMap.emplace(std::make_pair(configuration.mLaserID, frame_entity));

			// Get transform
			TransformComponentInstance* frame_xform = frame_entity->findComponent<TransformComponentInstance>();
			if (!errorState.check(frame_xform != nullptr, "frame entity doesn't have a transform component"))
				return false;

			// Now figure out it's position
			int current_col = static_cast<float>(current_count % cols);
			int current_row = static_cast<float>(current_count / cols);

			// Calculate offset
			float scale = frame_xform->getUniformScale();
			float offset = scale / 2.0f;

			// Calculate xform
			float tx = ((current_col * scale) + offset) - ((cols * scale) / 2);
			float ty = (rows * scale) - ((current_row * scale) + offset) - ((rows * scale) / 2);
			frame_xform->setTranslate(glm::vec3(tx, ty, 0.0f));

			// Get text component and 
			std::vector<Renderable2DTextComponentInstance*> text_comps;
			laser_entity->getComponentsOfTypeRecursive<Renderable2DTextComponentInstance>(text_comps);
			if (!errorState.check(text_comps.size() == 1, "%s: unable to find 2d text render component!", this->mID.c_str()))
				return false;

			// Set text when drawn
			if (!text_comps[0]->setText(utility::stringFormat("Laser: %d", configuration.mLaserID), errorState))
				return false;

			// Position text in center based on render buffer size
			glm::ivec2 text_loc = configuration.mTarget->getTarget().getSize();
			text_loc = text_loc / 2;

			// Compensate for y, because y is not centered, only x
			text_loc.y -= text_comps[0]->getBoundingBox().getHeight() / 2;
			text_comps[0]->setLocation(text_loc);

			current_count++;
		}

		return true;
	}


	nap::EntityInstance* LaserControlInstanceComponent::getLaserEntity(int id)
	{
		auto it = mLaserEntityMap.find(id);
		if (it == mLaserEntityMap.end())
			return nullptr;
		return it->second;
	}


	void LaserControlInstanceComponent::renderToLaserBuffers(nap::PerspCameraComponentInstance& camera, RenderService& renderer)
	{
		// Make sure we're rendering offscreen surfaces to primary window
		renderer.getPrimaryWindow().makeCurrent();
		
		// I want nice looking lines
		opengl::enableLineSmoothing(true);

		for (auto& it : mLaserEntityMap)
		{
			// Get all available components to render
			std::vector<RenderableComponentInstance*> components_to_render;
			std::vector<RenderableMeshComponentInstance*> mesh_components_to_render;
			std::vector<Renderable2DTextComponentInstance*> text_to_render;

			// Get entity
			nap::EntityInstance* entity = it.second;
			
			// Get all components to render
			entity->getComponentsOfTypeRecursive<RenderableMeshComponentInstance>(mesh_components_to_render);
			components_to_render.reserve(mesh_components_to_render.size());
			for (auto& comp : mesh_components_to_render)
				components_to_render.emplace_back(comp);
			
			// Get compound and clear target we want to render to
			const LaserConfiguration& configuration = mLaserConfigurationMap[it.first];

			// Clear target
			renderer.clearRenderTarget(configuration.mTarget->getTarget());
			
			// Render all meshes to target
			renderer.renderObjects(configuration.mTarget->getTarget(), camera, components_to_render);

			// Get text renderable component
			entity->getComponentsOfTypeRecursive<Renderable2DTextComponentInstance>(text_to_render);
			text_to_render[0]->draw(configuration.mTarget->getTarget());
		}
	}


	void LaserControlInstanceComponent::renderFrames(nap::RenderWindow& window, nap::PerspCameraComponentInstance& camera, RenderService& renderer)
	{
		// Get all available components to render
		std::vector<RenderableComponentInstance*> components_to_render;
		
		for (auto& it : mLaserFrameMap)
		{
			// Get component to render and frame to set
			nap::EntityInstance* entity = it.second;
			RenderableMeshComponentInstance& renderable_comp = entity->getComponent<RenderableMeshComponentInstance>();

			// Get texture from previously rendered backbuffer
			nap::Texture2D& tex = mLaserConfigurationMap[it.first].mTarget->getColorTexture();

			// Set it to frame uniform
			nap::UniformTexture2D& frame_tex = renderable_comp.getMaterialInstance().getOrCreateUniform<nap::UniformTexture2D>("mFrame");
			frame_tex.setTexture(tex);
			
			// Add for rendering
			components_to_render.emplace_back(&renderable_comp);
		}

		// Render all frames at once
		renderer.renderObjects(window.getBackbuffer(), camera, components_to_render);
	}
}