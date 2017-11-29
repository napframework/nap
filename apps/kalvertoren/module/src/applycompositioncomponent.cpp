#include "applycompositioncomponent.h"

// External Includes
#include <entity.h>
#include <meshutils.h>

// nap::applycompositioncomponent run time class definition 
RTTI_BEGIN_CLASS(nap::ApplyCompositionComponent)
	RTTI_PROPERTY("CompositionComponent", &nap::ApplyCompositionComponent::mCompositionComponent, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::applycompositioncomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ApplyCompositionComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void ApplyCompositionComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{

	}


	bool ApplyCompositionComponentInstance::init(utility::ErrorState& errorState)
	{
		if (!ApplyColorComponentInstance::init(errorState))
			return false;
		return true;
	}


	void ApplyCompositionComponentInstance::applyColor(double deltaTime)
	{
		// Get texture from active composition
		nap::Composition& comp = mCompositionComponent->getSelection();
		nap::BaseTexture2D& tex = comp.getTexture();

		// Populate the pixmap with the texture data
		tex.getData(mPixmap);

		// Get the model we want to color
		nap::ArtnetMeshFromFile& mesh = getMesh();

		// UV attribute we use to sample
		nap::VertexAttribute<glm::vec3>& uv_attr = mesh.getUVAttribute();

		// Color attribute we use to sample
		nap::VertexAttribute<glm::vec4>& color_attr = mesh.getColorAttribute();

		// Total amount of triangles
		int tri_count = getTriangleCount(mesh.getMeshInstance());
		TriangleDataPointer<glm::vec3> tri_uv_data;
		TriangleData<glm::vec4> new_triangle_color;

		// Will hold the rgb colors applied to the mesh
		RGBColorFloat rgb_color;
		assert(mPixmap.getPixel(0, 0)->getValueType() == RTTI_OF(uint8));

		for (int i = 0; i < tri_count; i++)
		{
			// Get uv coordinates for that triangle
			getTriangleValues<glm::vec3>(mesh.getMeshInstance(), i, uv_attr, tri_uv_data);

			// Average uv values
			glm::vec2 uv_avg{ 0.0,0.0 };
			for (const auto& uv_vertex : tri_uv_data)
			{
				uv_avg.x += uv_vertex->x;
				uv_avg.y += uv_vertex->y;
			}
			uv_avg.x = uv_avg.x / 3.0f;
			uv_avg.y = uv_avg.y / 3.0f;

			// Convert to pixel coordinates
			int x_pixel = static_cast<float>(mPixmap.getWidth()  - 1) * uv_avg.x;
			int y_pixel = static_cast<float>(mPixmap.getHeight() - 1) * uv_avg.y;

			// retrieve pixel value
			mPixmap.getRGBColorData<uint8>(x_pixel, y_pixel).convert(rgb_color);

			// iterate over every vertex in the triangle and set the color
			for (auto& vert_color : new_triangle_color)
			{
				vert_color.r = rgb_color.getRed();
				vert_color.g = rgb_color.getGreen();
				vert_color.b = rgb_color.getBlue();
				vert_color.a = 1.0f;
			}

			setTriangleValues<glm::vec4>(mesh.getMeshInstance(), i, color_attr, new_triangle_color);
		}

		nap::utility::ErrorState error;
		if (!mesh.getMeshInstance().update(error))
		{
			assert(false);
		}
	}
}