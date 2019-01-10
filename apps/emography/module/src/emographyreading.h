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
			TimeStamp mStartTime;					///< Property: Start time associated with snapshots
			TimeStamp mEndTime;						///< Property: End time associated with snapshots
		};
	}
}
