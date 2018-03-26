#include "applycompositioncomponent.h"

// External Includes
#include <entity.h>
#include <meshutils.h>
#include <triangleiterator.h>

// nap::applycompositioncomponent run time class definition 
RTTI_BEGIN_CLASS(nap::ApplyCompositionComponent)
	RTTI_PROPERTY("CompositionRenderer",	&nap::ApplyCompositionComponent::mCompositionRenderer,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ColorPaletteComponent",	&nap::ApplyCompositionComponent::mColorPaletteComponent,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ShowIndexColors",		&nap::ApplyCompositionComponent::mShowIndexColors,			nap::rtti::EPropertyMetaData::Default)
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
		return true;
	}


	void ApplyCompositionComponentInstance::applyColor(double deltaTime)
	{
		// Get the bitmap associated with the final composition
		nap::Bitmap& mBitmap = mCompositionRenderer->getBitmap();

		// If the bitmap is empty, ie: hasn't been downloaded yet, we skip this step
		// This occurs when the first frame hasn't been rendered yet
		if (mBitmap.empty())
			return;

		// Get the model we want to color
		nap::ArtnetMeshFromFile& mesh = getMesh();

		// Get the instance
		nap::MeshInstance& mesh_instance = mesh.getMeshInstance();

		// UV attribute we use to sample
		VertexAttribute<glm::vec3>& uv_data = mesh.getUVAttribute();

		// Color attribute we use to sample
		VertexAttribute<glm::vec4>& color_data  = mesh.getColorAttribute();
		VertexAttribute<glm::vec4>& artnet_data = mesh.getArtnetColorAttribute();

		// Will hold the rgb colors applied to the mesh
		RGBColorFloat rgb_colorf;
		RGBAColorFloat led_colorf;
		RGBColor8 rgb_index_color;

		// Make pixel we use to query data from bitmap
		auto source_pixel = mBitmap.makePixel();
		assert(mBitmap.mType == Bitmap::EDataType::BYTE);

		// Get brightness
		float brightness = mLightRegulator->getBrightness();

		TriangleIterator triangle_iterator(mesh.getMeshInstance());
		while (!triangle_iterator.isDone())
		{
			Triangle triangle = triangle_iterator.next();

			// Average uv values
			glm::vec3 uv_avg{ 0.0f, 0.0f, 0.0f };
			TriangleData<glm::vec3> uv_vertex_data = triangle.getVertexData(uv_data);
			uv_avg += uv_vertex_data.first();
			uv_avg += uv_vertex_data.second();
			uv_avg += uv_vertex_data.third();
			uv_avg /= 3.0f;

			// Convert to pixel coordinates
			int x_pixel = static_cast<float>(mBitmap.getWidth()  - 1) * uv_avg.x;
			int y_pixel = static_cast<float>(mBitmap.getHeight() - 1) * uv_avg.y;

			// retrieve pixel value and convert in to rgb index color
			mBitmap.getPixel(x_pixel, y_pixel, *source_pixel);
			source_pixel->convert(rgb_index_color);

			// Get the corresponding color palette value
			LedColorPaletteGrid::PaletteColor palette_color = mColorPaletteComponent->getPaletteColor(rgb_index_color);

			// Get the color we want to display on the mesh
			const RGBColor8& color_to_convert = mShowIndexColors ? rgb_index_color : palette_color.mScreenColor;
			color_to_convert.convert(rgb_colorf);

			// Get the associated LED color
			palette_color.mLedColor.convert(led_colorf);

			// Set the color data used to display the mesh in the viewport
			glm::vec4 mesh_color = glm::vec4(
				rgb_colorf.getRed()	  * brightness,
				rgb_colorf.getGreen() * brightness,
				rgb_colorf.getBlue()  * brightness,
				1.0f);
			
			triangle.setVertexData(color_data, mesh_color);

			// Set the color data that is used to send over artnet
			glm::vec4 artnet_color = glm::vec4(
				led_colorf.getRed()	  * brightness,
				led_colorf.getGreen() * brightness,
				led_colorf.getBlue()  * brightness,
				led_colorf.getAlpha() * brightness);
			
			triangle.setVertexData(artnet_data, artnet_color);
		}

		nap::utility::ErrorState error;
		if (!mesh_instance.update(error))
		{
			assert(false);
		}
	}
}