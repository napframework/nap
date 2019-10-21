#pragma once

// Local Includes
#include "flexdevice.h"

// External Includes
#include <nap/resource.h>
#include <nap/resourceptr.h>

namespace nap
{
	/**
	 * Allows for linking the flexblock device (algorithm) to a specific output.
	 * Base class for all other type of adapters.
	 */
	class NAPAPI FlexAdapter : public Resource
	{
		friend class FlexDevice;

		RTTI_ENABLE(Resource)
	public:
		virtual ~FlexAdapter();

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Enables / Disables this adapter
		 * When the adapter is disabled onCompute won't be called.
		 * @param value enable / disable the adapter
		 */
		void setEnabled(bool value)								{ mEnabled = value; }

		bool mEnabled = false;									///< Property: 'Enabled' if the the adapter is enabled and forward events
		nap::ResourcePtr<FlexDevice> mFlexDevice = nullptr;		///< Property 'FlexDevice' The flexblock algorithm

	protected:
		/**
		 * Called in derived classes when the flexblock device completed a compute cycle
		 * Note that this will be called from the same thread as the flexblock algorithm,
		 * It is your responsibility to act accordingly, do not stall this thread!
		 * @param device the flexblock algorithm
		 */
		virtual void onCompute(const FlexDevice& device) = 0;

	private:
		/**
		 * Called by the flex device when a compute cycle completed
		 */
		void compute();
	};
}
