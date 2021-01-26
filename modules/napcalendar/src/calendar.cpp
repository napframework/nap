#include "calendar.h"

// nap::calendar run time class definition 
RTTI_BEGIN_CLASS(nap::Calendar)
	RTTI_PROPERTY("Items",	&nap::Calendar::mItems,	nap::rtti::EPropertyMetaData::Required | nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	bool Calendar::init(utility::ErrorState& errorState)
	{
		return true;
	}
}