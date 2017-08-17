#pragma once

// External Includes
#include <rtti/rttiobject.h>
#include <rtti/factory.h>
#include <thread>
#include <mutex>

// Local Includes
#include "etherdreaminterface.h"

// Forward Declares
namespace nap
{
	class EtherDreamService;
};

namespace nap
{
	/**
	 * Represents an Etherdream DAC object, the DAC is connected to an ILDA supported laser
	 * Every DAC has a unique DAC name, index and point rate associated with it.
	 * The index is set after Service Registration. If the DAC is available to the system
	 * it will automatically be connected on initialization. On destruction a possible active connection is 
	 * closed. When the DAC is unavailable to the system the DAC is still is a valid resource but not active
	 * This object manages it's own connection to the DAC
	 */
	class NAPAPI EtherDreamDac : public rtti::RTTIObject
	{
		enum class NAPAPI EConnectionStatus : int
		{
			CONNECTED			= 0,	// The DAC is available and connected
			CONNECTION_ERROR	= 1,	// The DAC is available but a connection could not be established
			UNAVAILABLE			= 2,	// The DAC is not available (not found in the system)
		};

		friend class EtherDreamService;
		RTTI_ENABLE(rtti::RTTIObject)
	public:
		// Default constructor
		EtherDreamDac() = default;

		// Constructor used by factory
		EtherDreamDac(EtherDreamService& service);

		// Default destructor
		virtual ~EtherDreamDac();

		/**
		 * Initializes this DAC and registers it with the etherdream service
		 * If the DAC is not connected or unavailable this call will fail and block
		 * further execution of the program.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 *	@return current DAC connection status
		 */
		EConnectionStatus getConnectionStatus() const;

		/**
		 *	@return if the DAC is connected
		 */
		bool isConnected() const;

		/**
		 *	Set the points for this dac to write
		 */
		void setPoints(std::vector<EtherDreamPoint>& points);

		// Unique name of the dac (property), used for finding the device on the network
		std::string	mDacName;

		// The amount of points per second the connected laser is allowed to draw (property)
		int	mPointRate = 30000;

	private:
		// The etherdream service
		EtherDreamService*	mService = nullptr;

		// Current DAC status
		EConnectionStatus mStatus = EConnectionStatus::UNAVAILABLE;

		// The DAC system index, -1 if not available
		int	 mIndex = -1;

		// Thread used to write frames
		std::mutex						mWriteMutex;
		std::thread						mWriteThread;
		bool							mStopWriting = false;
		void							writeThread();
		std::vector<EtherDreamPoint>	mPoints;

		/**
		 *	Signals the laser write thread to stop writing data and exit
		 */
		void exitWriteThread();

		/**
		* Write a frame
		* @param data, all the points to write
		* @param npoints, number of points to write
		*/
		bool writeFrame(EtherDreamPoint* data, uint npoints);

		/**
		* @return current DAC read / write status
		*/
		EtherDreamInterface::EStatus getWriteStatus() const;

		/**
		 *	If the etherdream is running and pumping out frames
		 */
		bool mIsRunning = false;
	};

	using DacObjectCreator = rtti::ObjectCreator<EtherDreamDac, EtherDreamService>;
}
