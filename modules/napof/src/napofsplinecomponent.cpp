#include <nap/entity.h>
#include <nap/logger.h>

#include <assert.h>

#include <napofsplinecomponent.h>
#include <napoftransform.h>
#include <napofattributes.h>

// OF Includes
#include <ofGraphics.h>

// std Includes
#include <fmod.h>

namespace nap
{
	/**
	@brief Constructs the spline from a poly line
	**/
	void OFSplineComponent::SetFromPolyLine(const ofPolyline& inPolyline)
	{
		// Make sure it has enough points
		assert(inPolyline.size() >= 2);

		// Set the new poly line
		mSpline.getValueRef().SetPolyLine(inPolyline);

		// Trigger update
		mSpline.emitValueChanged();
	}



	/**
	@brief Draws the Polyline
	**/
	void OFSplineComponent::onDraw()
	{
		// Set to white
		ofSetColor(gDefaultWhiteColor);

		// Get buffer
		const ofVbo& buffer = mSpline.getValue().GetVertexBuffer();

		// Draw buffer
		mSpline.getValueRef().Draw();
	}


	//////////////////////////////////////////////////////////////////////////


	/**
	@brief Constructor
	**/
	OFSplineSelectionComponent::OFSplineSelectionComponent()
	{
		mSplineType.connectToValue(mTypeChangedSlot);
		mSplineSize.connectToValue(mSizeChangedSlot);
		mSplineCount.connectToValue(mCountChangedSlot);
	}



	/**
	@brief Creates a new spline and sets it on all the spline components
	**/
	void OFSplineSelectionComponent::CreateAndUpdateSpline()
	{
		Entity* parent = getParent();
		assert(parent != nullptr);

		nap::OFSplineComponent* spline_comp = mSpline.get();
		if (spline_comp == nullptr)
		{
			nap::Logger::warn(*this, "unable to resolve spline component");
			return;
		}

		// Spline to fill
		NSpline out_spline;
		switch (mSplineType.getValue())
		{
		case SplineType::Circle:
			gCreateCircle(mSplineSize.getValue(), mSplineCount.getValue(),  gOrigin, out_spline);
			break;
		case SplineType::Square:
			gCreateSquare(mSplineSize.getValue(), mSplineCount.getValue(),  gOrigin, out_spline);
			break;
		case SplineType::Triangle:
			gCreateEqualTriangle(mSplineSize.getValue(), mSplineCount.getValue(),  gOrigin, out_spline);
			break;
		case SplineType::Line:
			gCreateLine(mSplineSize.getValue(),   mSplineCount.getValue(),  gOrigin, out_spline);
			break;
		case SplineType::Hexagon:
			gCreateHexagon(mSplineSize.getValue(), mSplineCount.getValue(), gOrigin, out_spline);
			break;
		default:
			assert(false);
			break;
		}

		// Set it
		spline_comp->mSpline.setValue(out_spline);
	}



	//////////////////////////////////////////////////////////////////////////

	/**
	@brief Color constructor
	**/
	OFSplineColorComponent::OFSplineColorComponent()
	{
		mPreviousTime = ofGetElapsedTimef();
	}


	/**
	@brief Updates the colors for the spline component
	**/
	void OFSplineColorComponent::onUpdate()
	{
		// Calculate time difference
		float current_time = ofGetElapsedTimef();
		float diff_time = current_time - mPreviousTime;
		mPreviousTime = current_time;

		// Update time
		mTime += diff_time * mCycleSpeed.getValue();

		// Get spline component
		nap::OFSplineComponent* spline_component = mSpline.get();
		if (spline_component == nullptr)
		{
			nap::Logger::warn(*this, "unable to find spline component on parent");
			return;
		}

		// Make sure we have some colors
		if (mColors.getValue().size() == 0)
		{
			ofLogWarning("SplineColorComponent") << "No colors specified!";
			return;
		}

		// Get spline
		NSpline& spline = spline_component->mSpline.getValueRef();
		
		// Get color data
		SplineColorData& color_data = spline.GetColorDataRef();
		std::vector<ofFloatColor>& cur_colors = mColors.getValueRef();

		// Get count
		float buffer_count = float(color_data.size());
		float color_count  = float(cur_colors.size());

		// Buffer offset
		float clamped_offset = gClamp<float>(mOffset.getValueRef(), 0.0f, 1.0f);
		

		//////////////////////////////////////////////////////////////////////////

		// Holds the normalized color indent values
		float color_diff = 1.0f / color_count;

		for (uint i = 0; i < color_data.size(); i++)
		{
			// Get normalized index
			float idx = float(i) / buffer_count;
			idx = pow(idx, mFrequencyPower.getValue());

			// Add offset and multiply with frequency to repeat
			idx += (clamped_offset + mTime);
			idx = fmod(idx * mFrequency.getValue(), 1.0f);

			// Get color index
			int color_idx = int(idx * color_count);

			// If we step, set it to the current color
			if (mStep.getValue())
			{
				color_data[i] = cur_colors[color_idx];
				continue;;
			}

			// Otherwise we're interpolating between two different colors
			// Get the next available color and blend
			int next_color_idx = color_idx + 1 >= cur_colors.size() ? 0 : color_idx + 1;

			// Blend between the two
			float blend_min = color_diff * float(color_idx);
			float blend_max = color_diff * float(next_color_idx);

			// Map to 0 - 1
			float range = gFit(idx, blend_min, blend_max, 0.0f, 1.0f);

			// Set interpolated color data
			color_data[i].r = ofLerp(cur_colors[color_idx].r, cur_colors[next_color_idx].r, range);
			color_data[i].g = ofLerp(cur_colors[color_idx].g, cur_colors[next_color_idx].g, range);
			color_data[i].b = ofLerp(cur_colors[color_idx].b, cur_colors[next_color_idx].b, range);
		}
	}

	
	/**
	@brief Updates the color and vertex buffers
	**/
	void OFSplineUpdateGPUComponent::onUpdate()
	{
		Entity* parent = this->getParent();
		OFSplineComponent* spline_component = parent->getComponent<OFSplineComponent>();
		if (spline_component == nullptr)
		{
			Logger::warn("Can't find spline on entity, unable to update GPU buffers");
			return;
		}

		NSpline& spline = spline_component->mSpline.getValueRef();
		spline.UpdateVBO(NSpline::DataType::COLOR);
		spline.UpdateVBO(NSpline::DataType::VERTEX);
	}


	//////////////////////////////////////////////////////////////////////////


	// Constructor
	OFSplineFromFileComponent::OFSplineFromFileComponent()
	{
		mFile.valueChangedSignal.connect(mFileChangedSlot);
		mSplineCount.valueChangedSignal.connect(mCountChangedSlot);
	}


	// Creates and updates the spline
	void OFSplineFromFileComponent::createAndUpdateSpline()
	{
		if (mFile.getValue().empty())
			return;

		// Check if it exists
		ofFile spline_file(mFile.getValue());
		if (!spline_file.exists())
		{
			nap::Logger::warn("file does not exist: %s", spline_file.getAbsolutePath().c_str());
			return;
		}

		// Make sure we can load
		if (spline_file.getExtension() != "svg")
		{
			nap::Logger::warn("unable to load file: %s, not of type .svg", spline_file.getAbsolutePath().c_str());
			return;
		}

		// Make sure we can find the spline component to set the new spline on
		nap::OFSplineComponent* spline = mSpline.get();
		if (spline == nullptr)
		{
			nap::Logger::warn(*this, "unable to find parent spline");
			return;
		}

		// Create it
		NSpline new_spline;
		gCreateSplineFromFile(spline_file.getAbsolutePath(), mSplineCount.getValue(), new_spline);
		if (new_spline.GetPointCount() == 0)
		{
			nap::Logger::warn("unable to load spline from file, point count is 0: %s", spline_file.getAbsolutePath().c_str());
			return;
		}

		// Set the just loaded spline
		spline->mSpline.setValue(new_spline);
	}
}

// Define components
RTTI_DEFINE(nap::OFSplineComponent)
RTTI_DEFINE(nap::OFSplineSelectionComponent)
RTTI_DEFINE(nap::OFSplineColorComponent)
RTTI_DEFINE(nap::OFSplineUpdateGPUComponent)
RTTI_DEFINE(nap::OFSplineFromFileComponent)