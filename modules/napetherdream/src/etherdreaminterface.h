/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <utility/dllexport.h>
#include <stdint.h>
#include <string>
#include <nap/numeric.h>
#include <mutex>

namespace nap
{
	/**
	 * Point that is send to the EtherDream DAC
	 */
	struct NAPAPI EtherDreamPoint
	{
		int16_t X	= 0;			///< Horizontal Position, extreme left = -32768; extreme right = +32767
		int16_t Y	= 0;			///< Vertical Position, extreme bottom = -32768; extreme top = +32767
		int16_t R	= 0;			///< Red Value, 0 = off, +32767 = red
		int16_t G	= 0;			///< Green Value, 0 = off, +32767 = green
		int16_t B	= 0;			///< Blue Value, 0 = off, +32767 = blue
		int16_t I	= 0;			///< Point color number. This value is used as an index into the color palette. It isn't required to set this value
		int16_t AL  = 0;			///< 1 = don't draw, 0 = draw. It isn't required to set this value 
		int16_t AR  = 0;			///< Should be 1 for the last point. It is not required to set this value
	};

	/**
	 * Cross Platform Etherdream DAC hardware interface.
	 * Acts as the main entry point to the Etherdream interface for all supported platforms
	 */
	class NAPAPI EtherDreamInterface final
	{
	public:
		/**
		 * Etherdream DAC status
		 */
		enum class EStatus : int
		{
			ERROR	= -1,				///< DAC is in an erroneous state.
			BUSY	= 0,				///< DAC is busy writing data.
			READY	= 1					///< DAC accepts new data.
		};

	public:
		/**
		 *	Constructor
		 */
		EtherDreamInterface();

		/**
		 *	Destructor
		 */
		~EtherDreamInterface() = default;

		/**
		 * Initialize the Etherdream library
		 * @return if the library has been initialized correctly
		 */
		bool		init();

		/**
		 * @return the number of available Etherdream DAC interfaces
		 * Note that this call closes all active connections. Call only once
		 * after initialization!
		 */
		int			getCount() const;

		/**
		 * Returns the unique name of a specific Etherdream DAC
		 * @return the name of the laser DAC, empty if not found
		 * @param number: the number of the DAC to query
		 */
		std::string	getName(int number) const;

		/**
		 * Open a connection to the DAC at number
		 * @return if the connection has been established
		 * @param number: the number to connect to
		 */
		bool		connect(int number);

		/**
		 * Stop output of DAC at number as soon as the current frame is finished
		 * @param number: the DAC to stop
		 * @return: if the dac was able to be stopped
		 */
		bool		stop(int number);

		/**
		 * Close the TCP connection of the dac at number
		 * @param number: the DAC to disconnect
		 */
		void		disconnect(int number);

		/**
		 *	Close all connections and stop running
		 */
		void		close();

		/**
		* @return if the local buffer of the laser is ready to accept more frames
		*/
		EStatus		getStatus(int number) const;

		/**
		 * Write a laser frame that consists out of n number of points
		 * @param number: the DAC to send the points to
		 * @param data: the points to send over, consisting out of npoints
		 * @param npoints: the number of points to send over
		 * @param pps: the laser display rate or: points per second. 30000 is a common value
		 * @param repeatCount: Number of times to repeat the drawing of the points, must not be 0, 1 is default
		 */
		bool		writeFrame(int number, const EtherDreamPoint* data, uint npoints, uint pps, uint repeatCount);

		/**
		 *	@return etherdream DAC min value
		 */
		static constexpr int16_t etherMin()		{ return  std::numeric_limits<int16_t>::min(); }

		/**
		 *	@return etherdream DAC max value
		 */
		static constexpr int16_t etherMax()		{ return std::numeric_limits<int16_t>::max(); }

	private:
		// Number of available dacs found after initialization
		int			mAvailableDacs = 0;
	};
}
