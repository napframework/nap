#include "sequenceplayercurveinput.h"
#include "sequenceplayerparametersetter.h"
#include "sequenceservice.h"

#include <nap/logger.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SequencePlayerCurveInput)
RTTI_PROPERTY("Parameter", &nap::SequencePlayerCurveInput::mParameter, nap::rtti::EPropertyMetaData::Default)
RTTI_PROPERTY("Use Main Thread", &nap::SequencePlayerCurveInput::mUseMainThread, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	static bool registerObjectCreator = SequenceService::registerObjectCreator([](SequenceService* service)->std::unique_ptr<rtti::IObjectCreator>
	{
		return std::make_unique<SequencePlayerCurveInputObjectCreator>(*service);
	});


	SequencePlayerCurveInput::SequencePlayerCurveInput(SequenceService& service)
		: SequencePlayerInput(service)
	{
	}


	void SequencePlayerCurveInput::update(double deltaTime)
	{
		for(auto* setter : mSetters)
		{
			setter->setValue();
		}
	}


	void SequencePlayerCurveInput::registerParameterSetter(SequencePlayerParameterSetterBase* parameterSetter)
	{
		auto found_it = std::find_if(mSetters.begin(), mSetters.end(), [&](const auto& it)
		{
		  return it == parameterSetter;
		});
		assert(found_it == mSetters.end()); // duplicate entry

		if(found_it == mSetters.end())
		{
			mSetters.emplace_back(parameterSetter);
		}
	}


	void SequencePlayerCurveInput::removeParameterSetter(SequencePlayerParameterSetterBase* parameterSetter)
	{
		auto found_it = std::find_if(mSetters.begin(), mSetters.end(), [&](const auto& it)
		{
			return it == parameterSetter;
		});

		if(found_it != mSetters.end())
		{
			mSetters.erase(found_it);
		}
	}
}