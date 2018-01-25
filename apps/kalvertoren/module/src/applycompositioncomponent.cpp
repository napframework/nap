#include "applycompositioncomponent.h"

// External Includes
#include <entity.h>
#include <meshutils.h>
#include "TriangleIterator.h"

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
		VertexAttribute<glm::vec3>& uv_data = mesh.getUVAttribute();

		// Color attribute we use to sample
		VertexAttribute<glm::vec4>& color_data  = mesh.getColorAttribute();
		VertexAttribute<glm::vec4>& artnet_data = mesh.getArtnetColorAttribute();

		// Will hold the rgb colors applied to the mesh
		RGBColorFloat rgb_colorf;
		RGBAColorFloat led_colorf;
		RGBColor8 rgb_index_color;

		// Make pixel we use to query data from bitmap
		auto source_pixel = mPixmap.makePixel();
		assert(mPixmap.mType == Pixmap::EDataType::BYTE);
		float mesh_intensity = mShowIndexColors ? 1.0f : mIntensity;

		TriangleShapeIterator shape_iterator(mesh.getMeshInstance());
		while (!shape_iterator.isDone())
		{
			glm::ivec3 indices = shape_iterator.next();

			// Average uv values
			glm::vec3 uv_avg{ 0.0f, 0.0f, 0.0f };
			uv_avg += uv_data[indices[0]];
			uv_avg += uv_data[indices[1]];
			uv_avg += uv_data[indices[2]];
			uv_avg /= 3.0f;

			// Convert to pixel coordinates
			int x_pixel = static_cast<float>(mPixmap.getWidth()  - 1) * uv_avg.x;
			int y_pixel = static_cast<float>(mPixmap.getHeight() - 1) * uv_avg.y;

			// retrieve pixel value and convert in to rgb index color
			mPixmap.getPixel(x_pixel, y_pixel, *source_pixel);
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
				rgb_colorf.getRed()	  * mesh_intensity, 
				rgb_colorf.getGreen() * mesh_intensity, 
				rgb_colorf.getBlue()  * mesh_intensity, 
				1.0f);
			
			color_data[indices[0]] = mesh_color;
			color_data[indices[1]] = mesh_color;
			color_data[indices[2]] = mesh_color;

			// Set the color data that is used to send over artnet
			glm::vec4 artnet_color = glm::vec4(
				led_colorf.getRed()	  * mesh_intensity, 
				led_colorf.getGreen() * mesh_intensity, 
				led_colorf.getBlue()  * mesh_intensity, 
				led_colorf.getAlpha() * mIntensity);
			
			artnet_data[indices[0]] = artnet_color;
			artnet_data[indices[1]] = artnet_color;
			artnet_data[indices[2]] = artnet_color;
		}

		nap::utility::ErrorState error;
		if (!mesh_instance.update(error))
		{
			assert(false);
		}
	}
}