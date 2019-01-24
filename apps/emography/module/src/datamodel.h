#pragma once

#include "utility/errorstate.h"
#include "database.h"
#include "nap/datetime.h"
#include "utility/dllexport.h"

namespace nap
{
	namespace emography
	{
		class ReadingProcessor;
		class ReadingBase;
		class ReadingSummaryBase;

		/**
		 * The DataModel is a class that manages readings in a hierarchy. Readings are stored in a database and queries can be performed on a large set of readings
		 * for any timerange and on any resolution desired. For instance, reading can be pushed in the datamodel at a very high frequence (hundreds of readings per second),
		 * and the client can query for a week of data at a resolution of 7 steps (which is a day). The query will return 7 values, one for each day in that week. The
		 * value per day is a 'summarized' value of the original reading. How values are summarized can be configured by the client through the Summarize function.
		 *
		 * To accomplish this efficiently, the DataModel creates a hierarchy of data. In the following example, we show a number of raw readings and how the timeline is
		 * divided into 'Levels of Details' (LODs). The summarizing function is an averaging function:
		 *
		 *	Raw reading:	5  3  8  2	1  3  4  6
		 *	LOD 0		   [ 4  ][ 5  ][ 2  ][ 5  ]
		 *	LOD 1		   [   4.5    ][   3.5    ]
		 *  LOD 2		   [           4          ]
		 *
		 * In reality, the LODs do not need to have a two on one mapping. For instance, in reality, LOD0 will collapse data for a full second into its LOD which can be
		 * tens or hundreds of readings. LOD1 is configured as the 'minutes' LOD, which will collapse 60 values of the seconds LOD into a single minutes LOD. The fact
		 * that we have divided this LOD system into the natural seconds, minutes, hours, days and weeks LODs is arbitrary, any hierarchical division would work.
		 *
		 * This LOD system has the benefit that, when a query on a large time range is performed, the system does not need to calculate a 'summary' for large sets
		 * (possibly hundreds of thousands of readings), instead it can find the correct level of detail and draw its data from there, making it very efficient.
		 *
		 * The 'buckets' in each LOD are aligned to a full second, a full minute and so on. This means that when you need to query for a time range that is not aligned
		 * to a bucket, that we need to perform additional work. For example, if you would request a week of data, separated into 7 days, but the start of the week
		 * does not align to a day as stored in the buckets: it is a couple of hours off. This is exactly what happens when dealing with different time zones. The backing
		 * store has separated the data into buckets of a specific timezone. But as the local timezone changes, you want to request data that is unaligned to the data as
		 * stored in the backing store. To solve this, 'unaligned' queries are supported, enabling you to query from any start time to any end time, in any resolution.
		 * As a consequence, the flexibility of querying for any start/end time and resolution also enables you to gradually zoom in and out on the data set.
		 *
		 * The algorithm for performing unaligned queries works as follows. In the following example you can see a query for a timerange (only a single value as output) on an
		 * unaligned time:
		 *
		 *	Query range	                     #----------------------------------------------#
		 *	LOD 0		   [ 1  ][ 1  ][ 4  ][ 5  ][ 2  ][ 5  ][ 6  ][ 4  ][ 3  ][ 3  ][ 6  ][ 4  ]
		 *	LOD 1		   [    0.5   ][   4.5    ][   3.5    ][    5     ][     3    ][    5     ]
		 *  LOD 2		   [         2.5          ][         4.25         ][           4          ]
		 *
		 * This should calculate the average of the series 5, 2, 5, 6, 4, 3, 3 and 6, but we don't want to process all of these values. Instead we take advantage of the higher
		 * level LODs as much as possible. These are the LOD values that we are going to use instead:
		 *
		 *	LOD 0		   [ 5  ]                                   [ 6  ]
		 *	LOD 1		                                 [     3    ]
		 *  LOD 2		         [         4.25         ]
		 *
		 * We cannot just summarize (average) these values because some of these values have a larger weight. The higher LODs represent a different timerange and therefore a
		 * different weight. In the example, we have 8 source values to summarize, so normally each value has a weight of 1/8th. However, the higher LODs represent more values,
		 * so in this example, these are the weights that our series of numbers will be given:
		 *
		 * 		5 		: 1/8
		 *		4.25	: 1/2
		 *		3		: 1/4
		 *		6		: 1/8
		 *
		 * This array of values and their weights is passed to the Summarize function. For the average function this means the following:
		 * 		(5 * 1/8) + (4.25 * 1/2) + (3 * 1/4) + ( 6 * 1/8) = 4.25
		 *
		 * This matches the outcome of the original equation:
		 *		(5 + 2 + 5 + 6 + 4 + 3 + 3 + 6) / 8 = 4.25
		 *
		 * It is also possible that there are periods of 'inactivity': no data is generated for a period of time:
		 *
		 *	Query range	                     #----------------------------#
		 *	LOD 0		   [ 1  ][ 1  ][ 4  ][ 1  ][ 9  ][ x  ][ x  ][ 5  ]
		 *	LOD 1		   [    0.5   ][   4.5    ][    9   ][     5      ]
		 *  LOD 2		   [         2.5          ][         7            ]
		 *
		 * We must take care that the weights are calculated correctly here. In the example we would take the values 1 and 7. The weight for the values should be 1/3 and 2/3,
		 * but the timerange would indicate weights of 1/5 and 4/5. To solve this problem, we maintain a 'number of active seconds' in each LOD that propogates through the LODs:
		 *
		 *	Query range	                     #----------------------------#
		 *	LOD 0		   [ 1  ][ 1  ][ 1  ][ 1  ][ 1  ][ x  ][ x  ][ 1  ]
		 *	LOD 1		   [     2    ][     2    ][    1   ][     1      ]
		 *  LOD 2		   [           4          ][         2            ]
		 *
		 * We can use this value to calculate the correct weights, dealing with periods of inactivity.
		 *
		 */
		class NAPAPI DataModel final
		{
		public:
			enum class EKeepRawReadings : uint8_t
			{
				Enabled,
				Disabled
			};

			struct WeightedObject
			{
				float							mWeight;
				std::unique_ptr<rtti::Object>	mObject;
			};

			using SummaryFunction = std::function<std::unique_ptr<ReadingSummaryBase>(const std::vector<WeightedObject>&)>;

			DataModel(rtti::Factory& factory);
			~DataModel();

			bool init(const std::string& path, EKeepRawReadings keepRawReadings, utility::ErrorState& errorState);
			bool add(const ReadingBase& object, utility::ErrorState& errorState);
			bool flush(utility::ErrorState& errorState);

			template<class ReadingType>
			bool registerType(const SummaryFunction& summaryFunction, utility::ErrorState& errorState)
			{
				return registerType(RTTI_OF(Reading<ReadingType>), RTTI_OF(ReadingSummary<ReadingType>), summaryFunction, errorState);
			}

			template<class ReadingType>
			bool getRange(TimeStamp startTime, TimeStamp endTime, int numValues, std::vector<std::unique_ptr<ReadingSummaryBase>>& readings, utility::ErrorState& errorState)
			{
				return getRange(RTTI_OF(ReadingType), startTime, endTime, numValues, readings, errorState);
			}

		private:
			bool registerType(const rtti::TypeInfo& readingType, const rtti::TypeInfo& summaryType, const SummaryFunction& summaryFunction, utility::ErrorState& errorState);
			bool getRange(const rtti::TypeInfo& inReadingType, TimeStamp startTime, TimeStamp endTime, int numValues, std::vector<std::unique_ptr<ReadingSummaryBase>>& readings, utility::ErrorState& errorState);

		private:
			using ReadingProcessorMap = std::unordered_map<rtti::TypeInfo, std::unique_ptr<ReadingProcessor>>;
			Database			mDatabase;
			ReadingProcessorMap mReadingProcessors;
			EKeepRawReadings	mKeepRawReadings;
		};
	}
}