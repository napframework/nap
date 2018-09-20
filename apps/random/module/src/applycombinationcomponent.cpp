#include "applycombinationcomponent.h"

// External Includes
#include <entity.h>
#include <triangleiterator.h>

// nap::applycombinationcomponent run time class definition 
RTTI_BEGIN_CLASS(nap::ApplyCombinationComponent)
	RTTI_PROPERTY("Bitmap",		&nap::ApplyCombinationComponent::mBitmap,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Mesh",		&nap::ApplyCombinationComponent::mMesh,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Influence",	&nap::ApplyCombinationComponent::mInfluence,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Brightness", &nap::ApplyCombinationComponent::mBrightness,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::applycombinationcomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ApplyCombinationComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void ApplyCombinationComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{

	}


	bool ApplyCombinationComponentInstance::init(utility::ErrorState& errorState)
	{
		// Store bitmap pointer (contains the recently rendered combination pixels
		mBitmap = getComponent<ApplyCombinationComponent>()->mBitmap.get();

		// Store the mesh, which receives the updated color information
		mMesh = getComponent<ApplyCombinationComponent>()->mMesh.get();

		// Copy default values
		mBrightness = getComponent<ApplyCombinationComponent>()->mBrightness;
		mInfluence  = getComponent<ApplyCombinationComponent>()->mInfluence;

		return true;
	}


	void ApplyCombinationComponentInstance::update(double deltaTime)
	{
		// If the bitmap is empty, ie: hasn't been downloaded yet, we skip this step
		// This occurs when the first frame hasn't been rendered yet
		if (mBitmap->empty())
			return;

		// Get the model we want to color
		nap::ArtnetMeshFromFile& mesh = *mMesh;

		// Get the instance
		nap::MeshInstance& mesh_instance = mesh.getMeshInstance();

		// UV attribute we use to sample
		VertexAttribute<glm::vec3>& uv_data = mesh.getUVAttribute();

		// Position attribute used for blending to white
		VertexAttribute<glm::vec3>& pos_data = mesh.getPositionAttribute();

		// Color attribute we use to sample
		VertexAttribute<glm::vec4>& color_data = mesh.getColorAttribute();

		// Will hold the rgb colors applied to the mesh
		RGBColorFloat rgb_colorf;

		// Make pixel we use to query data from bitmap
		auto source_pixel = mBitmap->makePixel();
		assert(mBitmap->mType == Bitmap::EDataType::BYTE);
		
		TriangleIterator triangle_iterator(mesh_instance);
		auto converter = source_pixel->getConverter(rgb_colorf);
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
			int x_pixel = static_cast<float>(mBitmap->getWidth()  - 1) * uv_avg.x;
			int y_pixel = static_cast<float>(mBitmap->getHeight() - 1) * uv_avg.y;

			// retrieve pixel value and convert in to rgb float color
			mBitmap->getPixel(x_pixel, y_pixel, *source_pixel);
			BaseColor::convertColor(*source_pixel, rgb_colorf, converter);

			// Set the color data used to display the mesh in the viewport
			glm::vec4 mesh_color = glm::vec4(
				rgb_colorf.getRed()	  * mBrightness,
				rgb_colorf.getGreen() * mBrightness,
				rgb_colorf.getBlue()  * mBrightness,
				1.0f);

			triangle.setVertexData(color_data, mesh_color);
		}

		nap::utility::ErrorState error;
		if (!mesh_instance.update(error))
		{
			assert(false);
		}
	}
}