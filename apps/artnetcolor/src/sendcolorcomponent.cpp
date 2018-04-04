#include "sendcolorcomponent.h"

// External Includes
#include <entity.h>
#include <mathutils.h>

// nap::sendcolorcomponent run time class definition 
RTTI_BEGIN_CLASS(nap::SendColorComponent)
	RTTI_PROPERTY("Controllers", &nap::SendColorComponent::mControllers, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Span", &nap::SendColorComponent::mSpan, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Number", &nap::SendColorComponent::mNumber, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::sendcolorcomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SendColorComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void SendColorComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(nap::SelectColorComponent));
	}


	bool SendColorComponentInstance::init(utility::ErrorState& errorState)
	{
		// Copy controllers
		for (auto& controller : getComponent<SendColorComponent>()->mControllers)
		{
			mControllers.emplace_back(controller.get());
		}

		// Get list of all colors
		getEntityInstance()->getComponentsOfType<SelectColorComponentInstance>(mColorComps);
		
		// Make sure there are some colors to choose from
		if (!errorState.check(!mColorComps.empty(), "No color components found"))
			return false;

		// Set span
		setSpan(getComponent<SendColorComponent>()->mSpan);

		// Set number of colors
		setNumber(getComponent<SendColorComponent>()->mNumber);

		// All done
		return true;
	}


	void SendColorComponentInstance::update(double deltaTime)
	{
		// get number of dmx channels per color
		int dmx_color_span = mSpan * 4;
		std::vector<uint8> color_data(dmx_color_span, 0);

		// Current color
		int color_idx = 0;

		// Convert colors
		convertColors();

		// Set values in controller
		for (auto& controller : mControllers)
		{
			for(int i = 0; i < 512; i+=dmx_color_span)
			{
				// Get max buffer size
				int buffer_size = math::min<int>(512-i, dmx_color_span);
				color_data.resize(buffer_size);

				// Compose color for span
				for (int c = 0; c < buffer_size; c+=4)
				{
					color_data[c+0] = static_cast<uint8>(static_cast<float>(mColors[color_idx].getRed())   * mIntensity);
					color_data[c+1] = static_cast<uint8>(static_cast<float>(mColors[color_idx].getGreen()) * mIntensity);
					color_data[c+2] = static_cast<uint8>(static_cast<float>(mColors[color_idx].getBlue())  * mIntensity);
					color_data[c+3] = static_cast<uint8>(static_cast<float>(mWhites[color_idx].getRed())   * mIntensity);
				}

				// Send span
				controller->send(color_data, i);

				// Increment color
				assert(mNumber > 0);
				color_idx = ++color_idx % mNumber;
			}
		}
	}


	void SendColorComponentInstance::setSpan(int value)
	{
		mSpan = math::max<int>(value, 1);
	}


	void SendColorComponentInstance::setNumber(int value)
	{
		mNumber = math::min<int>(value, mColorComps.size());
	}


	void SendColorComponentInstance::convertColors()
	{
		mColors.clear();
		mWhites.clear();
		for (auto& color_comp : mColorComps)
		{
			mColors.emplace_back(color_comp->mColor.convert<RGBColor8>());
			mWhites.emplace_back(color_comp->mWhite.convert<RColor8>());
		}
	}

}
