#include "applycompositioncomponent.h"

// External Includes
#include <entity.h>
#include <meshutils.h>

// nap::applycompositioncomponent run time class definition 
RTTI_BEGIN_CLASS(nap::ApplyCompositionComponent)
	RTTI_PROPERTY("CompositionRenderer",	&nap::ApplyCompositionComponent::mCompositionRenderer,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ColorPaletteComponent",	&nap::ApplyCompositionComponent::mColorPaletteComponent,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ShowIndexColors",		&nap::ApplyCompositionComponent::mShowIndexColors,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("BlendToWhite",			&nap::ApplyCompositionComponent::mBlendToWhite,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("BlendAxis",				&nap::ApplyCompositionComponent::mBlendAxis,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("BlendRange",				&nap::ApplyCompositionComponent::mBlendRange,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("BlendPower",				&nap::ApplyCompositionComponent::mBlendPower,				nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::applycompositioncomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ApplyCompositionComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	const RGBAColorFloat ApplyCompositionComponentInstance::mWhiteLedColor(0.941f, 0.941f, 0.941f, 0.941f);
	const RGBColorFloat  ApplyCompositionComponentInstance::mWhiteRGBColor(1.0f, 1.0f, 1.0f);

	void ApplyCompositionComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{

	}


	bool ApplyCompositionComponentInstance::init(utility::ErrorState& errorState)
	{
		if (!ApplyColorComponentInstance::init(errorState))
			return false;

		// Copy and set attributes
		ApplyCompositionComponent* component = getComponent<ApplyCompositionComponent>();
		mShowIndexColors = component->mShowIndexColors;
		blendToWhite(component->mBlendToWhite);
		setBlendRange(component->mBlendRange);
		setBlendAxis(component->mBlendAxis);
		mBlendPower = component->mBlendPower;
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

		// Position attribute used for blending to white
		VertexAttribute<glm::vec3>& pos_data = mesh.getPositionAttribute();

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

			if (mBlendToWhite)
			{
				// Calculate avg vertex position
				glm::vec3 pos_avg{ 0.0f, 0.0f, 0.0f };
				TriangleData<glm::vec3> pos_vertex_data = triangle.getVertexData(pos_data);
				pos_avg += pos_vertex_data.first();
				pos_avg += pos_vertex_data.second();
				pos_avg += pos_vertex_data.third();
				pos_avg /= 3.0f;

				// Members that hold the blend value and blend range
				float pov = 0.0f;
				glm::vec2 pox = { 0.0f,0.0f };

				const math::Box& mesh_box = mesh.getBoundingBox();
				switch (mBlendAxis)
				{
				case 0:
					pov = pos_avg.x;
					pox = { mesh_box.getMin().x, mesh_box.getMax().x };
					break;
				case 1:
					pov = pos_avg.y;
					pox = { mesh_box.getMin().y, mesh_box.getMax().y };
					break;
				case 2:
					pov = pos_avg.z;
					pox = { mesh_box.getMin().z, mesh_box.getMax().z };
				default:
					assert(false);
					break;
				}

				// Calculate normalized lerp value to mix in white
				float norm_v = math::fit<float>(pov, pox.x, pox.y, 0.0f, 1.0f);
				float lerp_v = pow(math::fit<float>(norm_v, mBlendRange.x, mBlendRange.y, 0.0f, 1.0f), mBlendPower);

				// Mix in white for rgb pixel color
				for (int i = 0; i < rgb_colorf.getNumberOfChannels(); i++)
				{
					rgb_colorf[i] = math::lerp<float>(rgb_colorf[i], mWhiteRGBColor[i], lerp_v);
				}

				// Mix white for led pixel
				for (int i = 0; i < led_colorf.getNumberOfChannels(); i++)
				{
					led_colorf[i] = math::lerp<float>(led_colorf[i], mWhiteLedColor[i], lerp_v);
				}
			}

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


	void ApplyCompositionComponentInstance::setBlendAxis(int axis)
	{
		mBlendAxis = math::clamp<int>(axis, 0, 2);
	}


	void ApplyCompositionComponentInstance::setBlendRange(const glm::vec2& range)
	{
		mBlendRange.x = math::clamp<float>(range.x, 0.0f, 1.0f);
		mBlendRange.y = math::clamp<float>(range.y, 0.0f, 1.0f);
	}
}