#include "lasercontrolcomponent.h"

// External Includes
#include <nap/entity.h>
#include <nap/core.h>
#include <nap/resourcemanager.h>
#include <depthsorter.h>

// Local Includes
#include "makeprototypecomponent.h"

// nap::lasercontroller run time class definition 
RTTI_BEGIN_CLASS(nap::LaserControlComponent)
	RTTI_PROPERTY("LaserCompounds",		&nap::LaserControlComponent::mLaserCompounds,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("PrototypeEntity",	&nap::LaserControlComponent::mLaserPrototype,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FrameEntity",		&nap::LaserControlComponent::mFrameEntity,		nap::rtti::EPropertyMetaData::Required)
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


	bool LaserControlInstanceComponent::init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
	{
		// Get all compounds to use and create laser prototypes from
		nap::LaserControlComponent* resource = getComponent<LaserControlComponent>();
		mLaserCompounds = resource->mLaserCompounds;
		
		// Get resource manager that is used to spawn the new entity
		ResourceManagerService& resource_manager = *getEntityInstance()->getCore()->getService<nap::ResourceManagerService>();

		// Get total number of lasers to create
		int laser_count = static_cast<int>(mLaserCompounds.size());
		int current_count(0);
		int cols(2);
		int rows = laser_count / cols;

		for (auto& compound : mLaserCompounds)
		{
			auto new_entity = resource_manager.createEntity(*(resource->mLaserPrototype), entityCreationParams, errorState);
			if (new_entity == nullptr)
				return false;
			getEntityInstance()->addChild(*new_entity);

			// Find make prototype component instance
			MakePrototypeComponentInstance* prototype_component = new_entity->findComponent<MakePrototypeComponentInstance>();
			if (!errorState.check(prototype_component != nullptr, "laser entity doesn't have a make prototype component"))
				return false;

			// Make sure we don't have one with the same id
			if (mLaserEntityMap.find(compound->mLaserID) != mLaserEntityMap.end())
				return errorState.check(false, "laser with id %s already exists", compound->mLaserID);

			// Populate with laser compound settings
			if (!prototype_component->setup(*(compound), errorState))
				return false;

			// Store for future use
			mLaserEntityMap.emplace(std::make_pair(compound->mLaserID, new_entity.get()));
			mLaserCompoundMap.emplace(std::make_pair(compound->mLaserID, compound.get()));

			// Create the laser frame that will show the renderer laser canvas
			auto new_frame_entity = resource_manager.createEntity(*(resource->mFrameEntity), entityCreationParams, errorState);
			if (new_frame_entity == nullptr)
				return false;
			getEntityInstance()->addChild(*new_frame_entity);

			// Get transform
			TransformComponentInstance* frame_xform = new_frame_entity->findComponent<TransformComponentInstance>();
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
			float ty = ((current_row * scale) + offset) - ((rows * scale) / 2);

			frame_xform->setTranslate(glm::vec3(tx, ty, 0.0f));

			// Store for future use
			mLaserFrameMap.emplace(std::make_pair(compound->mLaserID, new_frame_entity.get()));

			// Increment count
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

		for (auto& it : mLaserEntityMap)
		{
			// Get all available components to render
			std::vector<RenderableComponentInstance*> components_to_render;

			// Get entity
			nap::EntityInstance* entity = it.second;
			
			// Get all components to render
			entity->getComponentsOfTypeRecursive<RenderableComponentInstance>(components_to_render);

			// Get compound and clear target we want to render to
			LaserCompound* matching_compound = mLaserCompoundMap[it.first];

			// Sort objects to render
			//std::vector<nap::RenderableComponentInstance*> sorted_objects;
			renderer.sortObjects(components_to_render, camera);

			// Clear target
			renderer.clearRenderTarget(matching_compound->mTarget->getTarget());
			
			// Render to target
			renderer.renderObjects(matching_compound->mTarget->getTarget(), camera, components_to_render);
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
			nap::BaseTexture2D& tex = mLaserCompoundMap[it.first]->mTarget->getColorTexture();

			// Set it to frame uniform
			nap::UniformTexture2D& frame_tex = renderable_comp.getMaterialInstance().getOrCreateUniform<nap::UniformTexture2D>("mFrame");
			frame_tex.setTexture(tex);
			
			// Add for rendering
			components_to_render.emplace_back(&renderable_comp);
		}

		// Render all frames at once
		renderer.renderObjects(*(window.getWindow()->getBackbuffer()), camera, components_to_render);
	}
}