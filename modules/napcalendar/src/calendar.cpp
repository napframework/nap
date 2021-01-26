// Local includes
#include "calendar.h"

// External includes
#include <rtti/rttiutilities.h>

// calendar usage type
RTTI_BEGIN_ENUM(nap::Calendar::EUsage)
	RTTI_ENUM_VALUE(nap::Calendar::EUsage::Dynamic, "Dynamic"),
	RTTI_ENUM_VALUE(nap::Calendar::EUsage::Static,	"Static")
RTTI_END_ENUM

// nap::calendar run time class definition 
RTTI_BEGIN_CLASS(nap::Calendar)
	RTTI_PROPERTY("Usage",	&nap::Calendar::mUsage,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Items",	&nap::Calendar::mItems,		nap::rtti::EPropertyMetaData::Required | nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

// calendar instance run time class definition
RTTI_BEGIN_CLASS(nap::CalendarInstance)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{

	Calendar::~Calendar()
	{
		mInstance.reset(nullptr);
	}

	bool Calendar::init(utility::ErrorState& errorState)
	{
		// Create and initialize instance
		mInstance = std::make_unique<CalendarInstance>();
		if (!mInstance->init(*this, errorState))
			return false;

		return true;
	}


	//////////////////////////////////////////////////////////////////////////
	// Instance
	//////////////////////////////////////////////////////////////////////////

	nap::CalendarInstance::CalendarInstance() { }


	bool CalendarInstance::init(const Calendar& resource, utility::ErrorState& error)
	{
		switch (resource.mUsage)
		{
			case Calendar::EUsage::Static:
			{
				rtti::Factory factory;
				mItems.reserve(resource.mItems.size());
				for (const auto& item : resource.mItems)
				{
					mItems.emplace_back(rtti::cloneObject(*item, factory));
				}
				break;
			}
			case Calendar::EUsage::Dynamic:
			{
				// Load from file
				break;
			}
			default:
			{
				assert(false);
				error.fail("unknown calendar usage case");
				return false;
			}
		}

		return true;
	}
}
