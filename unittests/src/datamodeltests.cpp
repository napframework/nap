#include "utils/catch.hpp"
#include "nap/datetime.h"
#include "emographystress.h"
#include "datamodel.h"
#include "emographysummaryfunctions.h"
#include "utility/fileutils.h"

#define VERIFY_ERROR_STATE(expr)				\
	if (!(expr))								\
	{											\
		Info(errorState.toString().c_str());	\
		FAIL();									\
	}


static bool sInitDataModel(nap::emography::DataModel& dataModel, const std::string& path, nap::utility::ErrorState& errorState)
{
	if (!dataModel.init(path, nap::emography::DataModel::EKeepRawReadings::Disabled, errorState))
		return false;

	return dataModel.registerType<nap::emography::StressIntensity>(&nap::emography::gAveragingSummary<nap::emography::StressIntensity>, errorState);
}


std::unique_ptr<nap::emography::DataModel> sCreateDataModel(const std::string& databasePath, nap::rtti::Factory& factory, nap::utility::ErrorState& errorState)
{
	if (!errorState.check(!nap::utility::fileExists(databasePath) || nap::utility::deleteFile(databasePath), "Failed to delete test database"))
		return nullptr;

	std::unique_ptr<nap::emography::DataModel> data_model = std::make_unique<nap::emography::DataModel>(factory);
	if (!sInitDataModel(*data_model, databasePath, errorState))
		return nullptr;

	return data_model;
}


TEST_CASE("Emography DataModel - Single Sample Query", "[single sample]")
{
	nap::rtti::Factory factory;

	std::string database_path = "SingleValueTest.db";
	nap::utility::ErrorState errorState;

	std::unique_ptr<nap::emography::DataModel> data_model = sCreateDataModel(database_path, factory, errorState);
	VERIFY_ERROR_STATE(data_model != nullptr)

	nap::TimeStamp now = nap::getCurrentTime();

	float value = 50.0f;

	std::unique_ptr<nap::emography::StressIntensityReading> intensityReading = std::make_unique<nap::emography::StressIntensityReading>(value, now);
	VERIFY_ERROR_STATE(data_model->add(*intensityReading, errorState))
	VERIFY_ERROR_STATE(data_model->flush(errorState))

	std::vector<std::unique_ptr<nap::emography::ReadingSummaryBase>> objects;
	VERIFY_ERROR_STATE(data_model->getRange<nap::emography::StressIntensityReading>(now, now.toSystemTime() + nap::Milliseconds(1000), 1, objects, errorState))
	VERIFY_ERROR_STATE(errorState.check(objects.size() == 1, "Expected one second of data"))

	nap::emography::StressIntensityReadingSummary* reading = rtti_cast<nap::emography::StressIntensityReadingSummary>(objects[0].get());
	VERIFY_ERROR_STATE(errorState.check(reading != nullptr, "Expected StressIntensityReading object"))
	VERIFY_ERROR_STATE(errorState.check(reading->mObject.mValue == value, "Incorrect value returned"))
	VERIFY_ERROR_STATE(errorState.check(reading->mNumSecondsActive == 1, "Expected a single second of active data"))
}


TEST_CASE("Emography DataModel - Aligned Minute Query", "[aligned-minute]")
{
	using namespace nap;
	using namespace nap::emography;

	nap::rtti::Factory factory;

	std::string database_path = "AlignedMinuteQuery.db";
	nap::utility::ErrorState errorState;

	std::unique_ptr<nap::emography::DataModel> data_model = sCreateDataModel(database_path, factory, errorState);
	VERIFY_ERROR_STATE(data_model != nullptr)

	nap::SystemTimeStamp time(Seconds(0));
	nap::SystemTimeStamp start = time;

	int total = 0;
	for (int i = 0; i != 60; ++i)
	{
		total += i;
		std::unique_ptr<StressIntensityReading> intensityReading = std::make_unique<StressIntensityReading>(i, time);
		VERIFY_ERROR_STATE(data_model->add(*intensityReading, errorState))

		time += Seconds(1);
	}
	float average = total / 60.0f;

	VERIFY_ERROR_STATE(data_model->flush(errorState))

	std::vector<std::unique_ptr<ReadingSummaryBase>> objects;
	VERIFY_ERROR_STATE(data_model->getRange<StressIntensityReading>(start, start + Seconds(60), 1, objects, errorState))
	VERIFY_ERROR_STATE(errorState.check(objects.size() == 1, "Expected one value"))

	StressIntensityReadingSummary* reading = rtti_cast<StressIntensityReadingSummary>(objects[0].get());
	VERIFY_ERROR_STATE(errorState.check(reading != nullptr, "Expected StressIntensityReading object"))
	VERIFY_ERROR_STATE(errorState.check(std::abs(reading->mObject.mValue - average) < 0.001, "Incorrect value returned"))
	VERIFY_ERROR_STATE(errorState.check(reading->mNumSecondsActive == 60, "Expected a single second of active data"))
}


TEST_CASE("Emography DataModel - Unaligned Minute Query", "[unaligned-minute]")
{
	using namespace nap;
	using namespace nap::emography;

	nap::rtti::Factory factory;

	std::string database_path = "UnalignedMinuteQuery.db";
	nap::utility::ErrorState errorState;

	std::unique_ptr<nap::emography::DataModel> data_model = sCreateDataModel(database_path, factory, errorState);
	VERIFY_ERROR_STATE(data_model != nullptr)

	SystemTimeStamp time(Seconds(0));
	SystemTimeStamp start = time;

	// Create three minutes of data
	for (int i = 0; i != 180; ++i)
	{
		std::unique_ptr<StressIntensityReading> intensityReading = std::make_unique<StressIntensityReading>(i, time);
		VERIFY_ERROR_STATE(data_model->add(*intensityReading, errorState))
		time += Seconds(1);
	}

	VERIFY_ERROR_STATE(data_model->flush(errorState))

	// Request a one and a half minute of unaligned data
	const int num_seconds = 90;
	for (int start_second = 30; start_second != 60; ++start_second)
	{
		// Calc average
		int total = 0;
		for (int i = 0; i != num_seconds; ++i)
			total += start_second + i;

		float average = total / (float)num_seconds;

		std::vector<std::unique_ptr<ReadingSummaryBase>> objects;
		VERIFY_ERROR_STATE(data_model->getRange<StressIntensityReading>(start + Seconds(start_second), start + Seconds(start_second + num_seconds), 1, objects, errorState))
		VERIFY_ERROR_STATE(errorState.check(objects.size() == 1, "Expected one value"))

		StressIntensityReadingSummary* reading = rtti_cast<StressIntensityReadingSummary>(objects[0].get());
		VERIFY_ERROR_STATE(errorState.check(reading != nullptr, "Expected StressIntensityReading object"))
		VERIFY_ERROR_STATE(errorState.check(std::abs(reading->mObject.mValue - average) < 0.001, "Incorrect value returned"))
		VERIFY_ERROR_STATE(errorState.check(reading->mNumSecondsActive == num_seconds, "Expected a single second of active data"))
	}
}


TEST_CASE("Emography DataModel - Inactivity Test", "[inactivity]")
{
	using namespace nap;
	using namespace nap::emography;

	nap::rtti::Factory factory;

	std::string database_path = "Inactivity.db";
	nap::utility::ErrorState errorState;

	std::unique_ptr<nap::emography::DataModel> data_model = sCreateDataModel(database_path, factory, errorState);
	VERIFY_ERROR_STATE(data_model != nullptr)

	SystemTimeStamp time(Seconds(0));
	SystemTimeStamp start = time;

	int total_active = 0;
	int active_count = 0;

	bool active = true;
	int num_inactive_samples = 0;

	struct Average
	{
		int mCount = 0;
		int mTotal = 0;
	};

	const int num_minutes = 3;
	Average minute_averages[num_minutes];

	// Create three minutes of data
	for (int i = 0; i != num_minutes * 60; ++i)
	{
		if (i % 10 != 0)
			active = !active;

		if (active)
		{
			std::unique_ptr<StressIntensityReading> intensityReading = std::make_unique<StressIntensityReading>(i, time);
			VERIFY_ERROR_STATE(data_model->add(*intensityReading, errorState))

			total_active += i;
			active_count++;

			int currentMinute = i / 60;
			minute_averages[currentMinute].mTotal += i;
			minute_averages[currentMinute].mCount++;
		}

		time += Seconds(1);
	}

	float active_average = (float)total_active / (float)active_count;

	VERIFY_ERROR_STATE(data_model->flush(errorState))

	// Single value test
	{
		std::vector<std::unique_ptr<ReadingSummaryBase>> objects;
		VERIFY_ERROR_STATE(data_model->getRange<StressIntensityReading>(start, start + Seconds(num_minutes * 60), 1, objects, errorState))
		VERIFY_ERROR_STATE(errorState.check(objects.size() == 1, "Expected one second of data"))

		StressIntensityReadingSummary* reading = rtti_cast<StressIntensityReadingSummary>(objects[0].get());
		VERIFY_ERROR_STATE(errorState.check(reading != nullptr, "Expected StressIntensityReading object"))
		VERIFY_ERROR_STATE(errorState.check(std::abs(reading->mObject.mValue - active_average) < 0.001, "Incorrect value returned"))
		VERIFY_ERROR_STATE(errorState.check(reading->mNumSecondsActive == active_count, "Wrong number of active seconds returned"))
	}

	// Multi value test
	{
		std::vector<std::unique_ptr<ReadingSummaryBase>> objects;
		VERIFY_ERROR_STATE(data_model->getRange<StressIntensityReading>(start, start + Seconds(num_minutes * 60), num_minutes, objects, errorState))
		VERIFY_ERROR_STATE(errorState.check(objects.size() == num_minutes, "Expected %d items of data", num_minutes))

		for (int i = 0; i < num_minutes; ++i)
		{
			float minute_average = (float)minute_averages[i].mTotal / (float)minute_averages[i].mCount;

			StressIntensityReadingSummary* reading = rtti_cast<StressIntensityReadingSummary>(objects[i].get());
			VERIFY_ERROR_STATE(errorState.check(reading != nullptr, "Expected StressIntensityReading object"))
			VERIFY_ERROR_STATE(errorState.check(std::abs(reading->mObject.mValue - minute_average) < 0.001, "Incorrect value returned"))
			VERIFY_ERROR_STATE(errorState.check(reading->mNumSecondsActive == minute_averages[i].mCount, "Wrong number of active seconds returned"))
		}
	}
}


