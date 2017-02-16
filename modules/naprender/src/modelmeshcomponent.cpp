#include "modelmeshcomponent.h"

namespace nap
{	
	// Resolves the link if necessary and returns the mesh
	opengl::Mesh* ModelMeshComponent::getMesh() const
	{
		// Don't do anything if we don't have a valid link
		if (!modelResource.isLinked())
			return nullptr;

		// Get resource
		ModelResource* model = modelResource.getResource<ModelResource>();
		if (model == nullptr)
		{
			nap::Logger::warn("unable to draw mesh: %s, unable to resolve model", this->getName().c_str());
			return nullptr;
		}

		// Get mesh
		opengl::Mesh* draw_mesh = model->getMesh(static_cast<unsigned int>(meshIndex.getValue()));
		if (draw_mesh == nullptr)
		{
			nap::Logger::warn("unable to draw mesh: %s, model index: %d can't be resolved", this->getName().c_str(), meshIndex.getValue());
			return nullptr;
		}
		return draw_mesh;
	}
} // nap

RTTI_DEFINE(nap::ModelMeshComponent)