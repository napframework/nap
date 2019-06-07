#include "timelinesequenceelement.h"

#include <rtti/rttiutilities.h>
#include <rtti/jsonreader.h>
#include <rtti/defaultlinkresolver.h>
#include <rtti/jsonwriter.h>
#include <rtti/epropertyvalidationmode.h>
#include <utility/fileutils.h>
#include <fstream>


RTTI_DEFINE_BASE(nap::TimelineSequenceElement)

RTTI_BEGIN_CLASS(nap::TimelineSequenceElement)
// Put additional properties here

RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////

namespace nap
{
	TimelineSequenceElement::~TimelineSequenceElement()			{ }


	bool TimelineSequenceElement::init(utility::ErrorState& errorState)
	{
		if (!errorState.check(mDuration >= 0.0f,
			"duration must be bigger or equal then 0 %s", this->mID.c_str()))
			return false;

		if (mUsePreset)
		{
			rtti::Factory factory;
			
			bool success = rtti::readJSONFile("presets/inputs/" + std::string(mPreset), rtti::EPropertyValidationMode::DisallowMissingProperties, factory, mReadPresetResult, errorState);
			if (!errorState.check(success,
				"error loading preset %s", this->mID.c_str()))
				return false;

			// Resolve links
			if (!rtti::DefaultLinkResolver::sResolveLinks(mReadPresetResult.mReadObjects, mReadPresetResult.mUnresolvedPointers, errorState))
				return false;

			//
			if (!errorState.check(mReadPresetResult.mReadObjects.size() > 0,
				"empty preset %s", this->mID.c_str()))
				return false;

			// first object must be parametergroup
			std::unique_ptr<rtti::Object>& object = mReadPresetResult.mReadObjects[0];

			// 
			if (!errorState.check(object->get_type().is_derived_from<ParameterGroup>(),
				"object must be derived from ParameterGroup %s", this->mID.c_str()))
				return false;

			// get group
			ParameterGroup* group = rtti_cast<ParameterGroup>(object.get());

			mParameters.clear();
			for (int i = 0; i < group->mParameters.size(); i++)
			{
				mParameters.push_back(group->mParameters[i].get());
			}
		}
		else
		{
		}

		return true;
	}

	bool TimelineSequenceElement::process(double time, std::vector<Parameter*>& outParameters)
	{
		return (time >= mStartTime && time < mStartTime + mDuration);
	}

	void TimelineSequenceElement::setStartParameters(const std::vector<Parameter*>& startParameters)
	{
		mStartParameters = startParameters;
	}
}