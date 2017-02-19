// of includes
#include <napofservice.h>

// nap of includes
#include <napofsplinecomponent.h>
#include <napofsimplecamcomponent.h>
#include <napofsimpleshapecomponent.h>
#include <napoftracecomponent.h>
#include <napofsplinemodulationcomponent.h>
#include <napoflagcomponent.h>
#include <napofframebuffercomponent.h>
#include <napofvideocomponent.h>
#include <napofimagecomponent.h>
#include <napofsoundcomponent.h>
#include <napoflayoutplanecomponent.h>
#include <napofspritecomponent.h>

static const ofMatrix4x4 sIdentityMatrix;

namespace nap
{
	// Register types for core that are of interest to this service (serviceable)
	// All derived types are considered of interest
	void OFService::sRegisterTypes(Core& inCore, const Service& inService)
	{
		inCore.registerType(inService, RTTI_OF(OFTransform));
		inCore.registerType(inService, RTTI_OF(OFRenderableComponent));
		inCore.registerType(inService, RTTI_OF(OFUpdatableComponent));
		inCore.registerType(inService, RTTI_OF(OFSimpleCamComponent));
		inCore.registerType(inService, RTTI_OF(OFMaterial));
	}


	//////////////////////////////////////////////////////////////////////////
	// Filters
	//////////////////////////////////////////////////////////////////////////


	/**
	Checks if the parent entity has a drawable component, if so, this object is not selected (not top)
	**/
	bool OFService::sHasTopXform(Object& inObject, Core& inCore)
	{
        assert(inObject.getTypeInfo().isKindOf<Component>());
        
        auto inComponent = static_cast<Component*>(&inObject);
        
		// Get the entity that owns the component
		Entity* comp_parent = inComponent->getParent();

		// If the parent is the root, we're at top level, component is valid
		if (comp_parent == &inCore.getRoot())
			return true;

		// Otherwise find the owning entity
		Entity* entity_parent = static_cast<Entity*>(comp_parent->getParent());

		// If the parent entity has a drawable component, it's not the top drawable component -> return false
		return !entity_parent->hasComponent<OFTransform>();
	}


	// Occurs when a component registers itself with this service
	void OFService::objectRegistered(Object& inObject)
	{
		setIsDirty(inObject);

		// Add visibility callback -> forces update on display list
		if (inObject.getTypeInfo().isKindOf(RTTI_OF(OFRenderableComponent)))
		{
			OFRenderableComponent& render_comp = static_cast<OFRenderableComponent&>(inObject);
			render_comp.mEnableDrawing.valueChanged.connect(mDisplayChanged);
		}
	}


	// Occurs when a component is removed from the service
	// In this case we want to update the display list if the type is valid
	void OFService::objectRemoved(Object& inObject)
	{
		setIsDirty(inObject);
	}


	// If the display or xform needs to be updated lists need to be updated
	void OFService::setIsDirty(const Object& inObject)
	{
		if (isDirtyXform && isDirtyDrawing)
			return;

		RTTI::TypeInfo info = inObject.getTypeInfo();
		if (info.isKindOf<OFTransform>())
		{
			isDirtyXform = true;
		}
		if (!info.isKindOf(RTTI_OF(OFMaterial)) && !info.isKindOf(RTTI_OF(OFUpdatableComponent)))
		{
			isDirtyDrawing = true;
		}
	}


	// Sorts the items based on distance
	void OFService::sortBasedOnDistance(const ofVec3f& inPos, DisplayList& inItems)
	{
		// Sort on distance from current cam
		std::sort(inItems.begin(), inItems.end(), [&inPos](const auto& a, const auto& b)
		{
			OFTransform* xform_a = a->getParent()->template getComponent<OFTransform>();
			OFTransform* xform_b = b->getParent()->template getComponent<OFTransform>();

			float d_a = abs((inPos - xform_a->getGlobalTransform().getTranslation()).z);
			float d_b = abs((inPos - xform_b->getGlobalTransform().getTranslation()).z);

			return d_a > d_b;
		});
	}


	// Re-creates the display list based on the items currently available, what is drawable and camera distance
	void OFService::updateDisplayList()
	{
		// Get possible cam components
		std::vector<OFSimpleCamComponent*> cam_components;
		getObjects<OFSimpleCamComponent>(cam_components);

		// Get all renderable components
		std::vector<OFRenderableComponent*> draw_components;
		getObjects<OFRenderableComponent>(draw_components, sObjectIsDrawable);

		// Clear display list
		mBufferDisplayList.clear();

		// Render every cam components buffer
		for (auto& camera : cam_components)
		{
			// Get possible frame buffer objects from camera
			Entity* cam_entity = camera->getParent();
			OFFrameBufferComponent* frame_buffer = cam_entity->getComponent<OFFrameBufferComponent>();

			// Don't handle camera's that don't have a frame-buffer
			if (frame_buffer == nullptr)
				continue;

			// Create cam relative display list
			ofVec3f cam_pos = camera->getGlobalPosition();

			// Sort on distance
			sortBasedOnDistance(cam_pos, draw_components);

			// Emplace back display list
			mBufferDisplayList.emplace(std::make_pair(camera, draw_components));
		}
		
		// Clear default cam list
		mDefaultDisplayList.clear();

		// Don't update display list if no default cam is available
		if (mDefaultCamera == nullptr)
			return;

		// Update default display list
		mDefaultDisplayList = draw_components;
	}
	

	// Sets current display list update mode
	void OFService::setDisplayMode(DisplayMode inMode)
	{
		if (mMode == inMode)
			return;

		mMode = inMode;
		isDirtyDrawing = true;
	}


	/**
	Recursive filter function for drawable components

	Checks if the incoming component is drawable, takes in to account parent entities and their drawable components
	**/
	bool OFService::sObjectIsDrawable(Object& inObject, Core& inCore)
	{
		assert(inObject.getTypeInfo().isKindOf(RTTI_OF(OFRenderableComponent)));
		OFRenderableComponent* renderable_comp = static_cast<OFRenderableComponent*>(&inObject);
		if (!renderable_comp->mEnableDrawing.getValue())
			return false;

		// Valid if parent is root
		Entity* entity = renderable_comp->getParent();

		// Get parent entity, if it doesn't exist it's the root, if so return true
		Entity* parent_entity = entity->getParent();
		if (parent_entity == nullptr)
			return true;
		
		// Otherwise sample the (for now first drawable) component
		// TODO: Make selection based on entities -> drawable, not components
		OFRenderableComponent* new_render = parent_entity->getComponent<OFRenderableComponent>();
        while(!new_render) {
            parent_entity = parent_entity->getParent();
            if (!parent_entity)
                return true;
            new_render = parent_entity->getComponent<OFRenderableComponent>();
        }


		// Otherwise check if the new renderable component is accepted 
		return sObjectIsDrawable(*new_render, inCore);
	}


	//////////////////////////////////////////////////////////////////////////


	// When the drawing mode of a component changes, update the display list
	void OFService::visibilityChanged(AttributeBase& inValue)
	{
		isDirtyDrawing = true;
	}


	/**
	@brief setDefaultCamera

	Sets default camera to use for drawing to screen -> main output
	**/
	void OFService::setDefaultCamera(Entity& inEntity)
	{
		if (!inEntity.hasComponent<OFSimpleCamComponent>())
		{
			Logger::warn("Entity doesn't have a camera attached: " + inEntity.getName());
			removeDefaultCamera();
			return;
		}

		// Set new cam and update display list
		mDefaultCamera = &inEntity;

		// Update display list if lazy
		isDirtyDrawing = true;
	}


	// Removes the default camera
	void OFService::removeDefaultCamera()
	{
		mDefaultCamera = nullptr;
		mDefaultDisplayList.clear();
	}


	/**
	@brief Draw
	Draws all renderable objects to screen
	**/
	void OFService::draw()
	{
		// Force background to black
		ofSetBackgroundColor(ofColor::black);

		// Check if we need to update the display lists
		switch (mMode)
		{
		case DisplayMode::LAZY:
		{
			if (isDirtyDrawing)
			{
				updateDisplayList();
				isDirtyDrawing = false;
			}
			break;
		}
		case DisplayMode::ALWAYS:
		{
			updateDisplayList();
			isDirtyDrawing = false;
			break;
		}
		default:
			break;
		}

		// Render every buffer attached to the camera
		for (auto& buffer_pair : mBufferDisplayList)
		{
			// Get frame buffer component
			OFSimpleCamComponent* cam_comp = buffer_pair.first;
			Entity* entity = cam_comp->getParent();
			if (entity == nullptr)
			{
				nap::Logger::warn("camera has no parent: %s", cam_comp->getName().c_str());
				continue;
			}

			OFFrameBufferComponent* frame_buffer = entity->getComponent<OFFrameBufferComponent>();
			if (frame_buffer == nullptr)
			{
				nap::Logger::warn("camera has no frame buffer: %s", cam_comp->getName().c_str());
				continue;
			}

			// Draw in to frame-buffer using camera
			frame_buffer->begin();
			cam_comp->begin(ofGetCurrentViewport());
			drawComponents(buffer_pair.second);
			cam_comp->end();
			frame_buffer->end();
		}

		// Don't draw to default screen if no camera was given
		if (mDefaultCamera == nullptr)
			return;

		OFSimpleCamComponent* current_cam = mDefaultCamera->getComponent<OFSimpleCamComponent>();
		assert(current_cam != nullptr);

		// Draw with default camera
		current_cam->begin(ofGetCurrentViewport());
		drawComponents(mDefaultDisplayList);
		current_cam->end();
	}


	/**
	@brief Draws @inComponentsToDraw to the currently active buffer

	This is a recursive function, where every entity's children are also drawn
	The children are drawn relative to the transform of the parent entity, creating a matrix stack
	**/
	void OFService::drawComponents(DisplayList& inComponentsToDraw)
	{
		// Draw every single one of the drawable components
		for (OFRenderableComponent* component : inComponentsToDraw)
		{
			// Only draw when enabled
			if (!component->mEnableDrawing.getValue())
				continue;

			// Get owning entity
			Entity* owning_entity = component->getParent();

			// Push matrix
			ofPushMatrix();

			// Get transform component
			OFTransform* xform_component = component->mTransform.get();

			//OFTransform* xform_component = owning_entity->getComponent<OFTransform>();			
			if (xform_component == nullptr)
			{
				Logger::warn("Entity has no " + RTTI_OF(OFTransform).getName() + ": " + owning_entity->getName());
			}
			else
			{
				ofMultMatrix(xform_component->getGlobalTransform());
			}

			// Set correct blending function
			if (component->mAlphaBlending.getValue())
			{
				glEnable(GL_BLEND);
				
				// Get blend functions
				OFBlendType type_one = component->mFirstBlendMode.getValue();
				OFBlendType type_two = component->mSecondBlendMode.getValue();

				// Set new blending mode
				glBlendFunc(gBlendTypes.at(type_one).mGLid, gBlendTypes.at(type_two).mGLid);
			}
			else
			{
				glDisable(GL_BLEND);
			}

			// Get material
			OFMaterial* material = component->mMaterial.get();
			if (material != nullptr)
			{
				material->bind();
				component->draw();
				material->unbind();
			}

			// Post draw
			component->onPostDraw();

			// Pop matrix
			ofPopMatrix();
		}
	}


	/**
	@brief Transforms current space based on incoming xform
	**/
	void OFService::transform(const OFTransform& inTransform)
	{
		ofMultMatrix(inTransform.getGlobalTransform());
	}


	/**
	@brief Updates all updatable components
	**/
	void OFService::update()
	{
		// Update all components
		std::vector<OFUpdatableComponent*> update_components;
		getObjects<OFUpdatableComponent>(update_components);
		for (auto& c : update_components)
		{
			if(!c->mEnableUpdates.getValue())
				continue;
			c->update();
		}

		// Get top transforms if dirty and populate xform list for every child
		if (isDirtyXform)
		{
			getObjects<OFTransform>(mTopXforms, sHasTopXform);
			for (auto& xform : mTopXforms)
			{
				xform->fetchChildTransforms();
			}
		}

		// Set global transform and forward matrix updates
		for (auto& top_xform : mTopXforms)
		{
			// Get parent
			Entity* parent = top_xform->getParent();
			assert(parent != nullptr);

			// Set global transform to be local transform for top level components
			top_xform->update(sIdentityMatrix);
		}

		// Make sure next iteration transform components are not ordered again
		isDirtyXform = false;
	}
}

// Define service
RTTI_DEFINE(nap::OFService)
