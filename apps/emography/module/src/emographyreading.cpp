#include "emographyreading.h"

// nap::emographyreading run time class definition 
RTTI_BEGIN_CLASS(nap::emography::ReadingBase)
	RTTI_PROPERTY("TimeStamp", &nap::emography::ReadingBase::mTimeStamp, nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

RTTI_BEGIN_CLASS(nap::emography::ReadingSummaryBase)
	RTTI_PROPERTY("TimeStamp", &nap::emography::ReadingSummaryBase::mTimeStamp, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("NumSecondsActive", &nap::emography::ReadingSummaryBase::mNumSecondsActive, nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT


//////////////////////////////////////////////////////////////////////////
