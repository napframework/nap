#include "flexblocksequenceelement.h"

RTTI_BEGIN_ENUM(nap::FlexBlockSequenceElementType)
	RTTI_ENUM_VALUE(nap::FlexBlockSequenceElementType::Stance, "Stance"),
	RTTI_ENUM_VALUE(nap::FlexBlockSequenceElementType::Transition, "Transition")
RTTI_END_ENUM

// nap::FlexBlockSequenceElement run time class definition 
RTTI_BEGIN_CLASS(nap::FlexBlockSequenceElement)
	// Put additional properties here
	RTTI_PROPERTY("SequenceType", &nap::FlexBlockSequenceElement::mType, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Stance", &nap::FlexBlockSequenceElement::mStance, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("NextStance", &nap::FlexBlockSequenceElement::mNextStance, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Duration", &nap::FlexBlockSequenceElement::mDuration, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	FlexBlockSequenceElement::~FlexBlockSequenceElement()			{ }


	bool FlexBlockSequenceElement::init(utility::ErrorState& errorState)
	{
		if (!errorState.check(mStance != nullptr,
			"stance must be set %s", this->mID.c_str()))
			return false;

		if (!errorState.check(!(mType == FlexBlockSequenceElementType::Transition && mNextStance == nullptr),
			"next stance must be set if type is transition %s", this->mID.c_str()))
			return false;

		if (!errorState.check(mDuration >= 0.0f,
			"duration must be bigger or equal then 0 %s", this->mID.c_str()))
			return false;

		return true;
	}
}