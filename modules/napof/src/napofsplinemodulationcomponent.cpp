#include <nap/entity.h>
#include <nap/logger.h>

#include <napofsplinemodulationcomponent.h>
#include <napofsplinecomponent.h>
#include <Utils/nofUtils.h>

namespace nap
{
	/**
	@brief Constructor
	**/
	OFSplineLFOModulationComponent::OFSplineLFOModulationComponent()
	{
		mIndex.valueChangedSignal.connect(mIndexChanged);
		mPreviousTime = ofGetElapsedTimef();
	}


	/**
	@brief Updates the spline verts along the normal based on the lfo type
	**/
	void OFSplineLFOModulationComponent::onUpdate()
	{
		// Update time
		float current_time = ofGetElapsedTimef();
		float diff_time = current_time - mPreviousTime;
		mPreviousTime = current_time;

		nap::Entity* entity = this->getParent();
		nap::OFSplineComponent* spline_component = entity->getComponent<nap::OFSplineComponent>();
		if (spline_component == nullptr)
		{
			Logger::warn("LFO modulation component can't find spline component");
			return;
		}

		// Get current time
		mTime += (diff_time * mSpeed.getValue());

		// Figure out final offset
		float offset = 1.0f - fmod(mTime + gClamp<float>(mOffset.getValue(), 0.0f, 1.0f),1.0f);

		// Fetch spline
		NSpline& spline = spline_component->mSpline.getValueRef();
		float point_count = float(spline.GetPointCount());

		// Get function pointer
		float(*sample_function)(float, float) = nullptr;
		switch (mType.getValue())
		{
		case LfoType::Sine:
			sample_function = &gGetSineWave;
			break;
		case LfoType::Square:
			sample_function = &gGetSquareWave;
			break;
		case LfoType::Triangle:
			sample_function = &gGetTriangleWave;
			break;
		case LfoType::Saw:
			sample_function = &gGetSawWave;
			break;
		default:
			assert(false);
			break;
		}

		// Get current frequency
		float curr_freq = mFrequency.getValue();
		float curr_ampl = mAmplitude.getValue();

		// Get total point count
		for (uint32 i = 0; i < spline.GetPointCount(); i++)
		{
			// Get normal for point
			ofVec3f& normal = spline.GetNormal(i);
			ofVec3f vertex  = spline.GetSourceVertex(i);

			// Get floating point index
			float sample_index = fmod((offset / curr_freq) + (float(i) / point_count), 1.0f);

			// Get data based on lfo type
			float wave_value = gFit(sample_function(sample_index, curr_freq), 0.0f, 1.0f, -1.0f, 1.0f) * curr_ampl;

			// Displace vert along normal
			vertex += normal * wave_value;

			// Set vertex
			spline.GetVertex(i) = vertex;
		}
	}
}

RTTI_DEFINE(nap::OFSplineLFOModulationComponent)
