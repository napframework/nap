#include "applytracercolorcomponent.h"

// External Includes
#include <entity.h>
#include <meshutils.h>
#include <mathutils.h>
#include "TriangleIterator.h"

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

		const std::vector<int>& channel_data = mesh.getChannelAttribute().getData();
		std::vector<glm::vec4>& color_data = mesh.getColorAttribute().getData();
		std::vector<glm::vec4>& artn_data = mesh.getArtnetColorAttribute().getData();

		TriangleShapeIterator shape_iterator(mesh.getMeshInstance());
		while (!shape_iterator.isDone())
		{
			glm::ivec3 indices = shape_iterator.next();

			int channel_number = channel_data[indices[0]];

			glm::vec4 color = channel_number == mCurrentChannel ? glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) : glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
			color_data[indices[0]] = color;
			color_data[indices[1]] = color;
			color_data[indices[2]] = color;

			artn_data[indices[0]] = color;
			artn_data[indices[1]] = color;
			artn_data[indices[2]] = color;
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