#include "linetracecomponent.h"

#include <nap/entity.h>
#include <mathutils.h>

RTTI_BEGIN_CLASS(nap::TraceProperties)
	RTTI_PROPERTY("Offset",	&nap::TraceProperties::mOffset,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Speed",	&nap::TraceProperties::mSpeed,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Length",	&nap::TraceProperties::mLength,		nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::LineTraceComponent)
	RTTI_PROPERTY("Properties",	&nap::LineTraceComponent::mProperties,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Source",		&nap::LineTraceComponent::mSourceLine,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Target",		&nap::LineTraceComponent::mTargetLine,	nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::LineTraceComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{

	bool LineTraceComponentInstance::init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
	{
		// Copy properties
		mProperties.mSpeed  = math::clamp<float>(getComponent<LineTraceComponent>()->mProperties.mSpeed, 0.0f,1.0f);
		mProperties.mLength = math::clamp<float>(getComponent<LineTraceComponent>()->mProperties.mLength, 0.0f, 1.0f);
		mProperties.mSpeed  = getComponent<LineTraceComponent>()->mProperties.mSpeed;

		// Set lines
		mSourceLine = getComponent<LineTraceComponent>()->mSourceLine.get();
		mTargetLine = getComponent<LineTraceComponent>()->mTargetLine.get();

		return true;
	}


	void LineTraceComponentInstance::update(double deltaTime)
	{

	}

}