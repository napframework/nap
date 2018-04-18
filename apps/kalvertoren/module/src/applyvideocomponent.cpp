#include "applyvideocomponent.h"

// External Includes
#include <entity.h>
#include <meshutils.h>
#include <mathutils.h>

// nap::applycompositioncomponent run time class definition 
RTTI_BEGIN_CLASS(nap::ApplyVideoComponent)
	RTTI_PROPERTY("VideoRenderer",		&nap::ApplyVideoComponent::mVideoRenderer,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ColorGrid",			&nap::ApplyVideoComponent::mColorGrid,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("VideoColors",		&nap::ApplyVideoComponent::mVideoColors,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("IndexMap",			&nap::ApplyVideoComponent::mIndexMap,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("VideoController",	&nap::ApplyVideoComponent::mVideoController,	nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::applycompositioncomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ApplyVideoComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void ApplyVideoComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{

	}


	bool ApplyVideoComponentInstance::init(utility::ErrorState& errorState)
	{
		if (!ApplyColorComponentInstance::init(errorState))
			return false;

		// Store some members
		ApplyVideoComponent* video_comp = getComponent<ApplyVideoComponent>();
		mColorGrid   = video_comp->mColorGrid.get();
		mVideoColors = video_comp->mVideoColors.get();
		mIndexMap = video_comp->mIndexMap.get();

		// Update palette
		updateSelectedPalette();

		return true;
	}


	void ApplyVideoComponentInstance::applyColor(double deltaTime)
	{
		// Get the bitmap associated with the final composition
		nap::Bitmap& video_bitmap = mVideoRenderer->getBitmap();

		// If the bitmap is empty, ie: hasn't been downloaded yet, we skip this step
		// This occurs when the first frame hasn't been rendered yet
		if (video_bitmap.empty())
			return;

		// Get the model we want to color
		nap::ArtnetMeshFromFile& mesh = getMesh();

		// Get the instance
		nap::MeshInstance& mesh_instance = mesh.getMeshInstance();

		// UV attribute we use to sample
		VertexAttribute<glm::vec3>& uv_data = mesh.getUVAttribute();

		// Position attribute used for blending to white
		VertexAttribute<glm::vec3>& pos_data = mesh.getPositionAttribute();

		// Color attribute we use to set (visible on the display)
		VertexAttribute<glm::vec4>& color_data = mesh.getColorAttribute();

		// Artnet attribute we use to set (send over the network)
		VertexAttribute<glm::vec4>& artnet_data = mesh.getArtnetColorAttribute();

		// Will hold the rgb colors applied to the mesh
		RGBColorFloat rgb_colorf;
		RGBAColorFloat led_colorf;
		RGBColor8 rgb_index_color;

		// Make pixel we use to query data from bitmap
		auto source_pixel = video_bitmap.makePixel();

		// Get brightness
		float brightness = mLightRegulator->getBrightness();

		// Get video brightness
		float video_brightness = mVideoController->getIntensity();

		TriangleIterator triangle_iterator(mesh_instance);
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
			int x_pixel = static_cast<float>(video_bitmap.getWidth() - 1)  * uv_avg.x;
			int y_pixel = static_cast<float>(video_bitmap.getHeight() - 1) * uv_avg.y;

			// retrieve pixel value and convert in to rgb index color
			video_bitmap.getPixel(x_pixel, y_pixel, *source_pixel);
			source_pixel->convert(rgb_index_color);

			// Get the corresponding color palette value
			LedColorPaletteGrid::PaletteColor palette_color = getPaletteColor(rgb_index_color);

			// Convert to rgb display color float
			palette_color.mScreenColor.convert(rgb_colorf);

			// Convert to rgbw led color float
			palette_color.mLedColor.convert(led_colorf);

			// Set the color data used to display the mesh in the viewport
			glm::vec4 mesh_color = glm::vec4(
				rgb_colorf.getRed()	  * brightness * video_brightness,
				rgb_colorf.getGreen() * brightness * video_brightness,
				rgb_colorf.getBlue()  * brightness * video_brightness,
				1.0f);

			// Apply color to mesh
			triangle.setVertexData(color_data, mesh_color);

			// Set the color data that is used to send over artnet
			glm::vec4 artnet_color = glm::vec4(
				led_colorf.getRed()	  * brightness * video_brightness,
				led_colorf.getGreen() * brightness * video_brightness,
				led_colorf.getBlue()  * brightness * video_brightness,
				led_colorf.getAlpha() * brightness * video_brightness);

			// Apply artnet value to mesh
			triangle.setVertexData(artnet_data, artnet_color);
		}

		nap::utility::ErrorState error;
		if (!mesh_instance.update(error))
		{
			assert(false);
		}
	}


	void ApplyVideoComponentInstance::updateSelectedPalette()
	{
		mIndexToPaletteMap.clear();
		std::vector<LedColorPaletteGrid::PaletteColor> palette_colors = mColorGrid->getPalette(*mVideoColors, 0);
		int count = 0;
		for (const auto& index_color : mIndexMap->getColors())
		{
			LedColorPaletteGrid::PaletteColor palette_color = count < palette_colors.size() ? palette_colors[count] : palette_colors.back();
			mIndexToPaletteMap.emplace(std::make_pair(index_color, palette_color));
			count++;
		}
	}


	nap::LedColorPaletteGrid::PaletteColor ApplyVideoComponentInstance::getPaletteColor(const IndexMap::IndexColor& indexColor) const
	{
		// Perform a direct lookup (fastest)
		auto it = mIndexToPaletteMap.find(indexColor);
		if (it != mIndexToPaletteMap.end())
			return it->second;

		// Iterate over all the colors and find the closest match
		float distance = math::max<float>();
		const LedColorPaletteGrid::PaletteColor* closest_color = nullptr;
		for (const auto& ccolor : mIndexToPaletteMap)
		{
			float cdist = ccolor.first.getDistance(indexColor);
			if (cdist < distance)
			{
				distance = cdist;
				closest_color = &(ccolor.second);
			}
		}
		return *closest_color;
	}

}