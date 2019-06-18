// local includes
#include "sequenceelement.h"

// external includes
#include <rtti/rttiutilities.h>
#include <rtti/jsonreader.h>
#include <rtti/defaultlinkresolver.h>
#include <rtti/jsonwriter.h>
#include <rtti/epropertyvalidationmode.h>
#include <utility/fileutils.h>
#include <fstream>

RTTI_BEGIN_CLASS(nap::timeline::SequenceElement)
// Put additional properties here

RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////

namespace nap
{
	namespace timeline
	{
		SequenceElement::~SequenceElement() { }


		bool SequenceElement::init(utility::ErrorState& errorState)
		{
			if (!errorState.check(mDuration >= 0.0f,
				"Duration must be bigger or equal then 0 %s", this->mID.c_str()))
				return false;

			if (mUsePreset)
			{
				rtti::Factory factory;

				bool success = rtti::readJSONFile(std::string(mPreset), rtti::EPropertyValidationMode::DisallowMissingProperties, rtti::EPointerPropertyMode::OnlyRawPointers, factory, mPresetReadResult, errorState);
				if (!errorState.check(success,
					"Error loading preset %s", this->mID.c_str()))
					return false;

				// Resolve links
				if (!rtti::DefaultLinkResolver::sResolveLinks(mPresetReadResult.mReadObjects, mPresetReadResult.mUnresolvedPointers, errorState))
					return false;

				//
				if (!errorState.check(mPresetReadResult.mReadObjects.size() > 0,
					"Empty preset %s", this->mID.c_str()))
					return false;

				// first object must be parametergroup
				std::unique_ptr<rtti::Object>& object = mPresetReadResult.mReadObjects[0];

				// 
				if (!errorState.check(object->get_type().is_derived_from<ParameterGroup>(),
					"Object must be derived from ParameterGroup %s", this->mID.c_str()))
					return false;

				// get group
				ParameterGroup* group = rtti_cast<ParameterGroup>(object.get());

				mEndParameters.clear();
				for (int i = 0; i < group->mParameters.size(); i++)
				{
					mEndParameters.push_back(group->mParameters[i].get());
				}
			}

			return true;
		}


		bool SequenceElement::process(double time, std::vector<Parameter*>& outParameters)
		{
			return (time >= mStartTime && time < mStartTime + mDuration);
		}


		void SequenceElement::setStartParameters(const std::vector<Parameter*>& startParameters)
		{
			mStartParameters = startParameters;
		}
	}
}