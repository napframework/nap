#include "applybbcolorcomponent.h"

// External Includes
#include <entity.h>
#include <meshutils.h>
#include <mathutils.h>
#include "TriangleIterator.h"

// nap::boundscolorcomponent run time class definition 
RTTI_BEGIN_CLASS(nap::ApplyBBColorComponent)
	// Put additional properties here
RTTI_END_CLASS

// nap::boundscolorcomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ApplyBBColorComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void ApplyBBColorComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{

	}


	bool ApplyBBColorComponentInstance::init(utility::ErrorState& errorState)
	{
		if (!ApplyColorComponentInstance::init(errorState))
			return false;
		return true;
	}


	void ApplyBBColorComponentInstance::applyColor(double deltaTime)
	{
		nap::ArtnetMeshFromFile& mesh = getMesh();

		const math::Box& box = mesh.getBoundingBox();

		// Get attributes necessary to color based on bounds
		const VertexAttribute<glm::vec3>& position_data = mesh.getPositionAttribute();
		VertexAttribute<glm::vec4>& color_data = mesh.getColorAttribute();
		VertexAttribute<glm::vec4>& artnet_data = mesh.getArtnetColorAttribute();

		TriangleShapeIterator shape_iterator(mesh.getMeshInstance());
		while (!shape_iterator.isDone())
		{
			glm::ivec3 indices = shape_iterator.next();

			// Get avg position value
			glm::vec3 avg_pos(0.0f, 0.0f, 0.0f);
			avg_pos += position_data[indices[0]];
			avg_pos += position_data[indices[1]];
			avg_pos += position_data[indices[2]];
			avg_pos /= 3;

			float r = pow(math::fit<float>(avg_pos.x, box.getMin().x, box.getMax().x, 0.0f, 1.0f), 2.0);
			float g = pow(math::fit<float>(avg_pos.y, box.getMin().y, box.getMax().y, 0.0f, 1.0f), 2.0);
			float b = pow(math::fit<float>(avg_pos.z, box.getMin().z, box.getMax().z, 0.0f, 1.0f), 2.0);

			// Set rgb for the mesh
			color_data[indices[0]] = glm::vec4(r, g, b, color_data[indices[0]].a);
			color_data[indices[1]] = glm::vec4(r, g, b, color_data[indices[1]].a);
			color_data[indices[2]] = glm::vec4(r, g, b, color_data[indices[2]].a);

			// Set rgb for arnet
			artnet_data[indices[0]] = glm::vec4(r, g, b, 0.0f);
			artnet_data[indices[1]] = glm::vec4(r, g, b, 0.0f);
			artnet_data[indices[2]] = glm::vec4(r, g, b, 0.0f);
		}

		nap::utility::ErrorState error;
		if (!mesh.getMeshInstance().update(error))
			assert(false);
	}

}