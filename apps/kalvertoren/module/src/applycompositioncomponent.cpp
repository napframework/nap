#include "applycompositioncomponent.h"

// External Includes
#include <entity.h>
#include <meshutils.h>

// nap::applycompositioncomponent run time class definition 
RTTI_BEGIN_CLASS(nap::ApplyCompositionComponent)
	RTTI_PROPERTY("CompositionRenderer",	&nap::ApplyCompositionComponent::mCompositionRenderer,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ColorPaletteComponent",	&nap::ApplyCompositionComponent::mColorPaletteComponent,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ShowIndexColors",		&nap::ApplyCompositionComponent::mShowIndexColors,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Intensity",				&nap::ApplyCompositionComponent::mIntensity,				nap::rtti::EPropertyMetaData::Default)
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

		// Copy if we want to show index colors
		mShowIndexColors = getComponent<ApplyCompositionComponent>()->mShowIndexColors;

		// Copy intensity
		mIntensity = getComponent<ApplyCompositionComponent>()->mIntensity;

		return true;
	}


	void ApplyCompositionComponentInstance::applyColor(double deltaTime)
	{
		// Get the pixmap associated with the final composition
		nap::Pixmap& mPixmap = mCompositionRenderer->getPixmap();

		// If the pixmap is empty, ie: hasn't been downloaded yet, we skip this step
		// This occurs when the first frame hasn't been rendered yet
		if (mPixmap.empty())
			return;

		// Get the model we want to color
		nap::ArtnetMeshFromFile& mesh = getMesh();

		// Get the instance
		nap::MeshInstance& mesh_instance = mesh.getMeshInstance();

		// UV attribute we use to sample
		nap::VertexAttribute<glm::vec3>& uv_attr = mesh.getUVAttribute();

		// Color attribute we use to sample
		nap::VertexAttribute<glm::vec4>& color_attr = mesh.getColorAttribute();
		nap::VertexAttribute<glm::vec4>& artnet_attr = mesh.getArtnetColorAttribute();

		// Total amount of triangles
		int tri_count = getTriangleCount(mesh.getMeshInstance());
		TriangleDataPointer<glm::vec3> tri_uv_data;
		TriangleDataPointer<glm::vec4> triangle_mesh_color;
		TriangleDataPointer<glm::vec4> triangle_artn_color;

		// Will hold the rgb colors applied to the mesh
		RGBColorFloat rgb_colorf;
		RGBAColorFloat led_colorf;
		RGBColor8 rgb_index_color;

		assert(mPixmap.mType == Pixmap::EDataType::BYTE);
		float mesh_intensity = mShowIndexColors ? 1.0f : mIntensity;

		for (int i = 0; i < tri_count; i++)
		{
			// Get uv coordinates for that triangle
			getTriangleValues<glm::vec3>(mesh_instance, i, uv_attr, tri_uv_data);

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
			mPixmap.getRGBColor<uint8>(x_pixel, y_pixel, rgb_index_color);
			
			// Get the corresponding color palette value
			LedColorPaletteGrid::PaletteColor palette_color = mColorPaletteComponent->getPaletteColor(rgb_index_color);

			// Get the color we want to display on the mesh
			const RGBColor8& color_to_convert = mShowIndexColors ? rgb_index_color : palette_color.mScreenColor;
			color_to_convert.convert(rgb_colorf);

			// Get the associated LED color
			palette_color.mLedColor.convert(led_colorf);

			getTriangleValues<glm::vec4>(mesh_instance, i, color_attr, triangle_mesh_color);
			getTriangleValues<glm::vec4>(mesh_instance, i, artnet_attr, triangle_artn_color);

			// iterate over every vertex in the triangle and set the colors
			for (int ti = 0; ti < triangle_mesh_color.size(); ti++)
			{
				triangle_mesh_color[ti]->r = rgb_colorf.getRed()	* mesh_intensity;
				triangle_mesh_color[ti]->g = rgb_colorf.getGreen()	* mesh_intensity;
				triangle_mesh_color[ti]->b = rgb_colorf.getBlue()	* mesh_intensity;
				triangle_mesh_color[ti]->a = 1.0;

				triangle_artn_color[ti]->r = led_colorf.getRed()	* mIntensity;
				triangle_artn_color[ti]->g = led_colorf.getGreen()	* mIntensity;
				triangle_artn_color[ti]->b = led_colorf.getBlue()	* mIntensity;
				triangle_artn_color[ti]->a = led_colorf.getAlpha()	* mIntensity;
			}
		}

		nap::utility::ErrorState error;
		if (!mesh_instance.update(error))
		{
			assert(false);
		}
	}
}