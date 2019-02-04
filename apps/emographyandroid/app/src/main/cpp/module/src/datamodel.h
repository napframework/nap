#pragma once

#include "emographyreading.h"

#include <database.h>
#include <nap/datetime.h>
#include <utility/dllexport.h> 
#include <mutex>
#include <nap/resource.h>
#include <utility/errorstate.h>

namespace nap
{
	namespace emography
	{
		class ReadingProcessor;
		class ReadingBase;
		class ReadingSummaryBase;
		class DataModelInstance;


		//////////////////////////////////////////////////////////////////////////
		// Resource
		//////////////////////////////////////////////////////////////////////////

		/**
		 * JSON serializable emography data model. 
		 * Creates and manages an instance of the data model internally.
		 * Use this resource to declare an emography data-model in JSON.
		 */
		class NAPAPI DataModel : public nap::Resource
		{
			RTTI_ENABLE(nap::Resource)
		public:
			
			/**
			 * Selects whether the original 'raw' reading are kept for storage in the database.
			 */
			enum class EKeepRawReadings : uint8_t
			{
				Enabled		= 1,
				Disabled	= 0
			};

			/**
			 * Default constructor
			 */
			DataModel() = default;

			/**
			 * Destroys the data-model	
			 */
			virtual ~DataModel();

			/**
			 * Creates the database and initializes the data model.
			 * TODO: This can't initialize the data model currently because of path appendix!
			 * Call init on the instance on startup of the application.
			 */
			virtual bool init(utility::ErrorState& errorState) override;

			/**
			 * Creates the database and initializes the data model based on the given database path.
			 * @return if initialization succeeded or failed.
			 */
			bool init(rtti::Factory& factory, const std::string& dbPath, utility::ErrorState& error);

			/**
			 * @return the data model that manages all the emography readings.
			 */
			const DataModelInstance& getInstance() const						{ return *mInstance; }

			/**
			 * @return the data model that manages all the emography readings.
			 */
			DataModelInstance& getInstance()									{ return *mInstance; }

			EKeepRawReadings mKeepRawReadings = EKeepRawReadings::Disabled;		///< Property: 'KeepRawReadings' if raw readings are kept in memory.

		private:
			std::unique_ptr<DataModelInstance> mInstance = nullptr;
		};


		//////////////////////////////////////////////////////////////////////////
		// Instance
		//////////////////////////////////////////////////////////////////////////

		/**
		 * The DataModel is a class that manages readings in a hierarchy. Readings are stored in a database and queries can be performed on a large set of readings
		 * for any timerange and on any resolution desired. For instance, reading can be pushed in the datamodel at a very high frequency (hundreds of readings per second),
		 * and the client can query for a week of data at a resolution of 7 steps (which is a day). The query will return 7 values, one for each day in that week. The
		 * value per day is a 'summarized' value of the original reading. How values are summarized can be configured by the client through the Summarize function.
		 *
		 * The DataModel is thread safe: you can add data to it on one thread and retrieve data from it on another thread.
		 *
		 * +++++++++++++++++++++++++++++++ The algorithm +++++++++++++++++++++++++++++++
		 *
		 *
		 * To accomplish querying large data sets efficiently, the DataModel creates a hierarchy of data. In the following example, we show a number of raw readings and 
		 * how the timeline is divided into 'Levels of Details' (LODs). The summarizing function is an averaging function:
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
		 *
		 * +++++++++++++++++++++++++++++++ Algorithm: unaligned queries +++++++++++++++++++++++++++++++
		 *
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
		 * +++++++++++++++++++++++++++++++ Algorithm: inactivity +++++++++++++++++++++++++++++++
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
		 *
		 * +++++++++++++++++++++++++++++++ A fully configurable object pipeline +++++++++++++++++++++++++++++++
		 *
		 *
		 * The DataModel is very versatile in what objects are inserted into the DataModel and what objects are returned from the Database. One could have a simple float
		 * value inserted into the DataModel, but have a richer structure returned from the DataModel when a query is performed. For instance, the output object could 
		 * count how many 1s, 2s and 3s were present in the range that was requested. It depends entirely on the Summarize function how the input data is converted into
		 * output data. All three parts of the following pipeline is fully customizable:
		 *
		 *  input -> summary function -> output
		 *
		 * The only restriction is that the DataModel works with ReadingBase and ReadingSummaryBase base classes. The ReadingBase is the 'input' object that couples 'some' value to a
		 * timestamp, and the ReadingSummaryBase is the 'output' object that has a bit of extra metadata that can be used by the DataModel to calculate summaries correctly. You can 
		 * derive from these two objects and provide a summary function that can convert the input to an output. By default, some simple objects are already provided:
		 *
		 *		Reading<T>			Derived from ReadingBase. This couples a templatized value T to the reading. T could be a float, int, or a class containing data.
		 *		ReadingSummary<T>	Derived from ReadingSummaryBase. Also couples the same T to the summary.
		 *
		 * These default objects could have several summarizing functions. A default summarizing function is provided in the form of an averaging function. This is the 
		 * gAveragingSummary function. It expects the T of Reading<T> and ReadingSummary<T> to have an mValue member. Therefore it only works in limited cases where T is a
		 * struct/class having a public mValue member, it doesn't work directly with ints and floats (but it will work with an mValue of any type (float, int, double etc). 
		 * You need to provide a different Summary function to support other cases like multiple members, direct float/int storage etc. And of course in case you don't want
		 * to summarize but to perform other forms of summarizing.
		 *
		 * To register what input/output/summary you would like to use for your value, use RegisterType. This will configure the pipeline for a type.
		 *
		 *
		 * +++++++++++++++++++++++++++++++ Limitations of summary functions +++++++++++++++++++++++++++++++
		 *
		 *
		 * The Summary function is used to collapse a list of data into a single output object. Internally this process will be repeated multiple times to construct a 
		 * hierarchy of data:
		 * 		  input -> summary function -> output level 0 \
		 *													   ----> summary function -> output level 1
		 * 		  input -> summary function -> output level 0 /
		 * 
		 * An important consequence is that not every summarizing function can be used. Any summarizing function that requires the full original list of raw readings will not work.
		 * For example, the mathematic Median function will need to sort the original list and find the middle number. This is a summarizing function that does not work hierarchically.
		 */
		class NAPAPI DataModelInstance final
		{
		public:
			/**
			 * The Summarize function takes list of WeightedObjects. Each object in the list has an associated weight.
			 * Because each object in this list can represent a different amount of time, the weight can be used to summarize correctly.
			 * See the comments above DataModel for a more in-depth explanation.
			 */
			struct WeightedObject
			{
				float							mWeight;		///< Weight representing how much this object should count for the summarize function
				std::unique_ptr<rtti::Object>	mObject;		///< The object as returned from the database
			};

			using SummaryFunction = std::function<std::unique_ptr<ReadingSummaryBase>(const std::vector<WeightedObject>&)>;

			/**
			 * Constructor.
			 * @param factory: The factory used when deserializing objects from the database.
			 */
			DataModelInstance(rtti::Factory& factory);

			/**
			 * Destructor, flushes the DataModel.
			 */
			~DataModelInstance();

			/**
			 * Opens the database.
			 * @param path: Path to the database on disk.
			 * @param keepRawReading: Selects whether the original 'raw' reading should be kept in the database or not.
			 * @param errorState : if the function returns false, contains error information.
			 */
			bool init(const std::string& path, DataModel::EKeepRawReadings keepRawReadings, utility::ErrorState& errorState);

			/**
			 * Adds a single reading to the DataModel. Updates the internal data hierarchy.
			 * @param object: Path to the database on disk.
			 * @param errorState : if the function returns false, contains error information.
			 */
			bool add(const ReadingBase& object, utility::ErrorState& errorState);

			/**
			 * Updates the DataModel up until the last processed reading for each object.
			 * @param errorState : if the function returns false, contains error information.
			 */
			bool flush(utility::ErrorState& errorState);

			/**
			 * Registers an object type with a summary function. This function uses the default Reading<T> and ReadingSummary<T> classes to wrap
			 * the ReadingType. To use a custom ReadingBase and ReadingSummary derived class, use the non-templatized version of registerType.
			 * See 'a fully configurable object pipeline' in the DataModel class comments for more details.
			 * @param summaryFunction: The summary function used to convert a reading to a summary.
			 * @param errorState : if the function returns false, contains error information.
			 */
			template<class ReadingType>
			bool registerType(const SummaryFunction& summaryFunction, utility::ErrorState& errorState)
			{
				return registerType(RTTI_OF(Reading<ReadingType>), RTTI_OF(ReadingSummary<ReadingType>), summaryFunction, errorState);
			}

			/**
			 * Registers all three parts (in -> summary -> out) of the object pipeline for an object type.
			 * See 'a fully configurable object pipeline' in the DataModel class comments for more details.
			 * @param readingType: The ReadingBase-derived object used as 'input'
			 * @param summaryType: The ReadingSummaryBase-derived object used as 'output'
			 * @param summaryFunction: The summary function used to convert a reading (input) to a summary (output).
			 * @param errorState : if the function returns false, contains error information.
			 */
			bool registerType(const rtti::TypeInfo& readingType, const rtti::TypeInfo& summaryType, const SummaryFunction& summaryFunction, utility::ErrorState& errorState);

			/**
			 * Queries the DataModel for retrieving summarized data for a time range. Any start- and end time can be requested, as well as a resolution: in how many steps
			 * the data is returned. 
			 * @param startTime: The start time of the range (inclusive).
			 * @param endTime: The end time of the range (exclusive)
			 * @param numValues: how many summarized values are to be returned, the number of 'steps'.
			 * @param numValues: readings: the deserialized objects from the database.
			 * @param errorState : if the function returns false, contains error information.
			 */
			template<class ReadingType>
			bool getRange(TimeStamp startTime, TimeStamp endTime, int numValues, std::vector<std::unique_ptr<ReadingSummaryBase>>& readings, utility::ErrorState& errorState)
			{
				return getRange(RTTI_OF(ReadingType), startTime, endTime, numValues, readings, errorState);
			}

			/**
			 * Clears all data in the datamodel: all rows in the database tables and the internal state.
			 */
			template<class ReadingType>
			bool clearData(utility::ErrorState& errorState)
			{
				return clearData(RTTI_OF(ReadingType), errorState);
			}

			/**
			 * Returns the timestamp for the last processed reading of this type.
			 */
			template<class ReadingType>
			TimeStamp getLastReadingTime() const
			{
				return getLastReadingTime(RTTI_OF(ReadingType));
			}

		private:
			bool getRange(const rtti::TypeInfo& inReadingType, TimeStamp startTime, TimeStamp endTime, int numValues, std::vector<std::unique_ptr<ReadingSummaryBase>>& readings, utility::ErrorState& errorState);
			TimeStamp getLastReadingTime(const rtti::TypeInfo& inReadingType) const;
			bool clearData(const rtti::TypeInfo& inReadingType, utility::ErrorState& errorState);

		private:
			using ReadingProcessorMap = std::unordered_map<rtti::TypeInfo, std::unique_ptr<ReadingProcessor>>;
			Database					mDatabase;				///< Database used as backing store
			ReadingProcessorMap			mReadingProcessors;		///< A map holding all the internal reading processors for all object types
			DataModel::EKeepRawReadings mKeepRawReadings;		///< Flag indicating whether raw data should be kept and discarded
			mutable std::mutex			mLock;					///< Mutex used to guard concurrent use of the datamodel
		};
	}
}