// Local Includes
#include "rendercomponent.h"

namespace nap
{
	// On construction adds a material instance
	// Material can't be removed
	RenderableComponent::RenderableComponent()
	{
		// Create and link
		Material* new_material = &addChild<Material>("material");
		new_material->setFlag(nap::ObjectFlag::Removable, false);
		material.setTarget(*new_material);
	}


	// Bind and draw
	void RenderableComponent::draw()
	{
		Material* mat = getMaterial();

		// Fetch material, should always be present
		if (mat == nullptr)
		{
			nap::Logger::warn("unable to resolve material for render-able component: %s", this->getName().c_str());
			return;
		}

		// If we're not dealing with a material that has a resource linked, continue
		// TODO: Apply Default Material
		if (!mat->hasShader())
		{
			nap::Logger::warn(*this, "unable to draw object, no linked material");
			return;
		}
		
		// Bind material
		mat->bind();

		// Upload uniforms
		mat->pushUniforms();

		// Draw
		onDraw();

		// Unbind material
		mat->unbind();
		onPostDraw();
	}


	// Returns the material associated with the render component
	// nullptr if material can't be found
	Material* RenderableComponent::getMaterial()
	{
		return material.getTarget<Material>();
	}

}