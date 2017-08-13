#pragma once

#include <utility/dllexport.h>
#include <stdint.h>
#include <string>
#include <nap/configure.h>

namespace nap
{
	/**
	 * Point that is send to the EtherDream DAC
	 */
	struct NAPAPI EtherDreamPoint
	{
		int16_t X	= 0;
		int16_t Y	= 0;
		int16_t R	= 0;
		int16_t G	= 0;
		int16_t B	= 0;
		int16_t I	= 0;
		int16_t AL	= 0;
		int16_t AR	= 0;
	};

	/**
	 * Cross Platform Etherdream hardware interface
	 * Acts as the main entry point to the Etherdream interface for all
	 * supported platforms
	 */
	class NAPAPI EtherDreamInterface final
	{
	public:
		enum class NAPAPI EStatus : int
		{
			ERROR	= -1,
			BUSY	= 0,
			READY	= 1,
		};

	public:
		/**
		 *	Constructor
		 */
		EtherDreamInterface();

		/**
		 *	Destructor
		 */
		~EtherDreamInterface()					{ close(); }

		/**
		 * Initialize the Etherdream library
		 * @return if the library has been initialized correctly
		 */
		bool		init();

		/**
		 * @return the number of available Etherdream DAC interfaces
		 */
		int			getCount() const;

		/**
		 * Returns the unique name of a specific Etherdream DAC
		 * @return the name of the laser DAC, empty if not found
		 * @param number: the number of the DAC to query
		 */
		std::string	getName(int number) const;

		/**
		 * Open a connection to the dac at number
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
		 * @param number: the dac to send the points to
		 * @param data: the points to send over, consisting out of npoints
		 * @param npoints: the number of points to send over
		 * @param pps: the laser display rate or: points per second. 30000 is a common value
		 * @param repeatCount: Number of times to repeat the drawing of the points, must not be 0, 1 is default
		 */
		bool		writeFrame(int number, const EtherDreamPoint* data, uint npoints, uint pps, uint repeatCount);
	};
}