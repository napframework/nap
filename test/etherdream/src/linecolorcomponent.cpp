// Local Includes
#include "linecolorcomponent.h"

// External Includes
#include <nap/logger.h>
#include <nap/entity.h>

RTTI_BEGIN_CLASS(nap::LineColorComponent)
	RTTI_PROPERTY("Color",				&nap::LineColorComponent::mColor,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("BlendComponent",		&nap::LineColorComponent::mBlendComponent,	nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::LineColorComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
	bool LineColorComponentInstance::init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
	{
		mBlendComponent = getComponent<LineColorComponent>()->mBlendComponent.get();
		mColor = getComponent<LineColorComponent>()->mColor;
		return true;
	}


	void LineColorComponentInstance::update(double deltaTime)
	{
		// Get the line and color attribute we want to update
		nap::PolyLine& line = mBlendComponent->getLine();
		nap::Vec4VertexAttribute& color_attr = line.getColorAttr();

		// Set the new color
		std::vector<glm::vec4> color_data(color_attr.getCount(), mColor);
		color_attr.setData(color_data);

		utility::ErrorState error;
		if (!line.getMeshInstance().update(error))
		{
			nap::Logger::warn("unable to change color: %s", error.toString().c_str());
		}
	}
}