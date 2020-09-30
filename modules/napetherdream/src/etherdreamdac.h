#pragma once

// External Includes
#include <nap/device.h>
#include <rtti/factory.h>
#include <thread>
#include <mutex>
#include <atomic>

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
	 * Represents an Etherdream DAC object, the DAC is connected to an ILDA supported laser.
	 * Every DAC has a unique DAC name, index and point rate associated with it.
	 * The index is set after Service Registration. If the DAC is available to the system
	 * it will automatically be connected on initialization. On destruction a possible active connection is 
	 * closed. When the DAC is unavailable to the system the DAC is still is a valid resource but not active.
	 * This object manages it's own connection to the DAC
	 */
	class NAPAPI EtherDreamDac : public Device
	{
		friend class EtherDreamService;
		RTTI_ENABLE(Device)
	public:
		// Default constructor
		EtherDreamDac() = default;

		// Constructor used by factory
		EtherDreamDac(EtherDreamService& service);

		/**
		 * Initializes this DAC and registers it with the etherdream service.
		 * If the DAC is not connected or unavailable this call will fail and block
		 * further execution of the program.
		 */
		virtual bool start(utility::ErrorState& errorState) override;

		/**
		 * Stops the DAC and unregisters it from the etherdream service.
		 */
		virtual void stop() override;

		/**
		 *	Set the points for this dac to write
		 */
		void setPoints(std::vector<EtherDreamPoint>& points);
		
		/**
		 * @return if the DAC is connected
		 */
		bool isConnected() const;

		/**
		 * @return last read / write status	
		 */
		EtherDreamInterface::EStatus getStatus() const;

		std::string	mDacName;				///< Property: 'DacName' Unique name of the DAC, used for finding the device on the network
		int	mPointRate = 30000;				///< Property: 'PointRate' The amount of points per second the connected laser is allowed to draw (property)
		bool mAllowFailure = true;			///< Property: 'AllowFailure' Allows initialization to succeed when the DAC can't be found on the network or can't be connected to

	private:
		// The etherdream service
		EtherDreamService*	mService = nullptr;

		// The DAC system index, -1 if not available
		int	 mIndex = -1;

		// Thread used to write frames
		std::mutex						mWriteMutex;
		std::thread						mWriteThread;
		std::atomic<bool>				mStopWriting = { false };
		std::vector<EtherDreamPoint>	mPoints;

		// Thread that writes frame to laser when available
		void							writeThread();

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
		 * If the etherdream dac is connected	
		 */
		bool mConnected;

		/**
		 * Last available DAC communication state	
		 */
		std::atomic<EtherDreamInterface::EStatus> mStatus;
	};

	using DacObjectCreator = rtti::ObjectCreator<EtherDreamDac, EtherDreamService>;
}
