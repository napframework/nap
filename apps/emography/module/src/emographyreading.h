#pragma once

// Local Includes
#include "emographysnapshot.h"

// External Includes
#include <nap/resource.h>
#include <nap/timestamp.h>

namespace nap
{
	namespace emography
	{
		/* 
		TODO: Fully implement templated container types based on range
		class NAPAPI BaseRangedContainer
		{
		public:
			TimeStamp mStartTime;
			TimeStamp mEndTime;
			int mSamples;
		};

		template<typename T> 
		class RangedContainer : public BaseRangedContainer
		{
		public:

		private:
			std::vector<T> mMembers;
		};

		using SnapshotContainer = RangedContainer<Snapshot>;
		*/

		/**
		 * Represents a set of emography snapshots that spans a particular time range
		 */
		class NAPAPI Reading : public Resource
		{
			RTTI_ENABLE(Resource)
		public:
			virtual ~Reading();

			/**
			* Initialize this object after de-serialization
			* @param errorState contains the error message when initialization fails
			*/
			virtual bool init(utility::ErrorState& errorState) override;

			std::vector<Snapshot> mSnapshots;		///< Property: 'Snapshots' all snapshots associated with this reading
			TimeStamp mStartTime;					///< Property: 'StartTime' Start date associated with snapshots
			TimeStamp mEndTime;						///< Property: 'EndTime' End date associated with snapshots
			int mSamples = 1024;					///< Property: 'Samples' number of snapshots associated with this reading
		};
	}
}
