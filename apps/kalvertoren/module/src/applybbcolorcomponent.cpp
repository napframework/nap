#include "applybbcolorcomponent.h"

// External Includes
#include <entity.h>
#include <meshutils.h>
#include <mathutils.h>

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
		nap::VertexAttribute<glm::vec4>& color_attr = mesh.getColorAttribute();
		nap::VertexAttribute<glm::vec3>& position_attr = mesh.getPositionAttribute();
		nap::VertexAttribute<glm::vec4>& artnet_attr = mesh.getArtnetColorAttribute();

		int triangle_count = getTriangleCount(mesh.getMeshInstance());

		nap::TriangleDataPointer<glm::vec4> tri_color_data;
		nap::TriangleDataPointer<glm::vec3> tri_posit_data;
		nap::TriangleDataPointer<glm::vec4> tri_artne_data;

		for (int triangle = 0; triangle < triangle_count; triangle++)
		{
			// Get current cd values
			getTriangleValues(mesh.getMeshInstance(), triangle, color_attr, tri_color_data);

			// Get current artnet values
			getTriangleValues(mesh.getMeshInstance(), triangle, artnet_attr, tri_artne_data);

			// Get current position values
			getTriangleValues(mesh.getMeshInstance(), triangle, position_attr, tri_posit_data);

			// Get avg position value
			glm::vec3 avg_pos(0.0f, 0.0f, 0.0f);
			for (auto& pos : tri_posit_data)
			{
				avg_pos += *pos;
			}
			avg_pos /= 3;

			float r = pow(math::fit<float>(avg_pos.x, box.getMin().x, box.getMax().x, 0.0f, 1.0f), 2.0);
			float g = pow(math::fit<float>(avg_pos.y, box.getMin().y, box.getMax().y, 0.0f, 1.0f), 2.0);
			float b = pow(math::fit<float>(avg_pos.z, box.getMin().z, box.getMax().z, 0.0f, 1.0f), 2.0);

			// Set rgb for both the mesh and artnet colors on the mesh
			for (int ti = 0; ti < tri_color_data.size(); ti++)
			{
				tri_color_data[ti]->r = r;
				tri_color_data[ti]->g = g;
				tri_color_data[ti]->b = b;

				tri_artne_data[ti]->r = r;
				tri_artne_data[ti]->g = g;
				tri_artne_data[ti]->b = b;
				tri_artne_data[ti]->a = 0.0f;
			}
		}

		nap::utility::ErrorState error;
		if (!mesh.getMeshInstance().update(error))
			assert(false);
	}

}