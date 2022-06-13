/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "calendar.h"

// External includes
#include <rtti/rttiutilities.h>
#include <utility/fileutils.h>
#include <rtti/jsonwriter.h>
#include <rtti/jsonreader.h>
#include <rtti/defaultlinkresolver.h>
#include <fstream>
#include <nap/logger.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ICalendar)
RTTI_END_CLASS

// nap::calendar run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Calendar)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Items",			&nap::Calendar::mItems,			nap::rtti::EPropertyMetaData::Required | nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("AllowFailure",	&nap::Calendar::mAllowFailure,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// calendar instance run time class definition
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::CalendarInstance)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// ICalendar
	//////////////////////////////////////////////////////////////////////////

	ICalendar::ICalendar(nap::Core& core) : mCore(core) {  }


	//////////////////////////////////////////////////////////////////////////
	// Calendar
	//////////////////////////////////////////////////////////////////////////

	Calendar::Calendar(nap::Core& core) : ICalendar(core) { }


	Calendar::~Calendar()
	{
		mInstance.reset(nullptr);
	}


	bool Calendar::init(utility::ErrorState& errorState)
	{
		// Create and initialize instance
		mInstance = std::make_unique<CalendarInstance>(mCore);
		if (!mInstance->init(mID.c_str(), mAllowFailure, mItems, errorState))
			return false;

		return true;
	}


	//////////////////////////////////////////////////////////////////////////
	// Instance
	//////////////////////////////////////////////////////////////////////////

	nap::CalendarInstance::CalendarInstance(nap::Core& core) : mCore(core)
	{ }


	bool CalendarInstance::init(const std::string& name, bool allowFailure, CalendarItemList items, utility::ErrorState& error)
	{
		// Calendar name
		if (!error.check(!name.empty(), "No calendar name specified"))
			return false;
		mName = name;

		// Get calendar path
		std::string path = utility::stringFormat("%s/%s/%s.json", mCore.getProjectInfo()->getDataDirectory().c_str(),
			calendarDirectory, getName().c_str());
		mPath = utility::forceSeparator(path);

		// Load calendar if file exists
		if (utility::fileExists(mPath))
		{
			if (load(error))
				return true;

			if (!allowFailure)
				return false;

			// Loads defaults if failure is allowed
			nap::Logger::warn("Unable to load calendar: %s, %s", mPath.c_str(), error.toString().c_str());
			nap::Logger::warn("Loading calendar defaults");
		}

		// Otherwise load default
		mItems.clear();
		mItems.reserve(items.size());
		for (const auto& item : items)
		{
			mItems.emplace_back(rtti::cloneObject(*item, mCore.getResourceManager()->getFactory()));
		}
		return true;
	}


	bool CalendarInstance::removeItem(const std::string& id)
	{
		auto found_it = std::find_if(mItems.begin(), mItems.end(), [&](const auto& it)
		{
		  return it->mID == id;
		});

		if (found_it != mItems.end())
		{
			itemRemoved.trigger(**found_it);
			mItems.erase(found_it);
			return true;
		}
		return false;
	}


	void CalendarInstance::addItem(std::unique_ptr<CalendarItem> item)
	{
		mItems.emplace_back(std::move(item));
		itemAdded.trigger(*mItems.back());
	}


	nap::CalendarItem* CalendarInstance::findByID(const std::string& id)
	{
		auto found_it = std::find_if(mItems.begin(), mItems.end(), [&](const auto& it)
		{
		  return it->mID == id;
		});
		return found_it != mItems.end() ? (*found_it).get() : nullptr;
	}


	nap::CalendarItem* CalendarInstance::findByTitle(const std::string& title)
	{
		auto found_it = std::find_if(mItems.begin(), mItems.end(), [&](const auto& it)
		{
		  return it->mTitle == title;
		});
		return found_it != mItems.end() ? (*found_it).get() : nullptr;
	}


	bool nap::CalendarInstance::load(utility::ErrorState& error)
	{
		nap::Logger::info("loading calendar: %s", getPath().c_str());
		rtti::DeserializeResult result;
		rtti::Factory& factory = mCore.getResourceManager()->getFactory();

		// Read file
		if (!deserializeJSONFile(
			getPath(), rtti::EPropertyValidationMode::DisallowMissingProperties,
			rtti::EPointerPropertyMode::OnlyRawPointers,
			factory, result, error))
			return false;

		// Resolve links
		if (!rtti::DefaultLinkResolver::sResolveLinks(result.mReadObjects, result.mUnresolvedPointers, error))
			return false;

		// Move objects
		mItems.clear();
		mItems.reserve(result.mReadObjects.size());
		for (auto& item : result.mReadObjects)
		{
			std::unique_ptr<CalendarItem> calendar_item = rtti_cast<CalendarItem>(item);
			if (calendar_item == nullptr)
			{
				assert(false);
				continue;
			}
			mItems.emplace_back(std::move(calendar_item));
		}
		return true;
	}


	bool nap::CalendarInstance::save(utility::ErrorState& error)
	{
		nap::rtti::ObjectList resources;
		resources.reserve(mItems.size());
		for (auto& item : mItems)
			resources.emplace_back(item.get());

		// Serialize current set of parameters to json
		rtti::JSONWriter writer;
		if (!rtti::serializeObjects(resources, writer, error))
			return false;

		// Make sure we can write to the directory
		std::string storage_dir = utility::getFileDir(getPath());
		if (!utility::dirExists(storage_dir))
		{
			if (!error.check(utility::makeDirs(storage_dir), "unable to create directory : %s", calendarDirectory))
				return false;
		}

		// Open output file
		std::ofstream output(getPath(), std::ios::binary | std::ios::out);
		if (!error.check(output.is_open() && output.good(), "Failed to open %s for writing", getPath().c_str()))
			return false;

		// Write to disk
		std::string json = writer.GetJSON();
		output.write(json.data(), json.size());
		return true;
	}
}
