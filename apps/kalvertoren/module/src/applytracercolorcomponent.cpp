#include "applytracercolorcomponent.h"

// External Includes
#include <entity.h>
#include <meshutils.h>
#include <mathutils.h>
#include <triangleiterator.h>

// nap::applytracercolorcomponent run time class definition 
RTTI_BEGIN_CLASS(nap::ApplyTracerColorComponent)
	// Put additional properties here
RTTI_END_CLASS

// nap::applytracercolorcomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ApplyTracerColorComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void ApplyTracerColorComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{

	}


	bool ApplyTracerColorComponentInstance::init(utility::ErrorState& errorState)
	{
		if (!ApplyColorComponentInstance::init(errorState))
			return false;
		return true;
	}


	void ApplyTracerColorComponentInstance::applyColor(double deltaTime)
	{
		// Increment time
		mChannelTime += mManualSelect ? 0.0f : (deltaTime*(mChannelSpeed*4.0));

		// Get channel, if manual selection is turned on use the actual selected channel, otherwise time based value
		int selected_channel = mManualSelect ? mSelectedChannel : static_cast<int>(mChannelTime) % 512;

		ArtnetMeshFromFile& mesh = getMesh();

		// This is the channel we want to compare against, makes sure that the we take
		// in to account the offset of channels associated with a mesh, so:
		// no offset means starting at 0 where 1 2 and 3 are considered to be part of the
		// same triangle. With an offset of 1, 2 3 and 4 are considered to be part of the
		// same triangle. 
		mCurrentChannel = selected_channel - ((selected_channel - mesh.mChannelOffset) % 4);

		const VertexAttribute<int>& channel_data = mesh.getChannelAttribute();
		VertexAttribute<glm::vec4>& color_data = mesh.getColorAttribute();
		VertexAttribute<glm::vec4>& artn_data = mesh.getArtnetColorAttribute();

		float brightness = mLightRegulator->getBrightness();

		TriangleIterator triangle_iterator(mesh.getMeshInstance());
		while (!triangle_iterator.isDone())
		{
			Triangle triangle = triangle_iterator.next();
			int channel_number = channel_data[triangle.firstIndex()];

			glm::vec4 mesh_color = channel_number == mCurrentChannel ? 
				glm::vec4(brightness, brightness, brightness, 1.0f) : 
				glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			
			glm::vec4 artn_color = channel_number == mCurrentChannel ?
				glm::vec4(brightness, brightness, brightness, brightness) :
				glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

			triangle.setVertexData(color_data, mesh_color);
			triangle.setVertexData(artn_data, artn_color);
		}

		nap::utility::ErrorState error;
		if (!mesh.getMeshInstance().update(error))
		{
			assert(false);
		}
	}


	void ApplyTracerColorComponentInstance::setSpeed(float speed)
	{
		mChannelSpeed = speed;
		setManual(false);
	}


	void ApplyTracerColorComponentInstance::selectChannel(int channel)
	{
		mSelectedChannel = nap::math::min<int>(channel, 511);
		setManual(true);
	}

}