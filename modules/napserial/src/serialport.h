#pragma once

// External Includes
#include <nap/device.h>
#include <string.h>
#include <nap/numeric.h>

// Forward declares
namespace serial
{
	class Serial;
}

namespace nap
{
	/**
	 * Defines the possible byte sizes for the serial port
	 */
	enum class ESerialByteSize : int
	{
		Five			= 5,
		Six				= 6,
		Seven			= 7,
		Eight			= 8
	};

	/**
	 * Defines the possible stop bit types for the serial port
	 */
	enum class ESerialStopBits : int
	{
		One				= 1,
		Two				= 2,
		OnePointFive	= 3
	};

	/**
	 * Defines the possible flow control types for the serial port
	 */
	enum class ESerialFlowControl : int
	{
		None			= 0,
		Software		= 1,
		Hardware		= 2
	};

	/**
	 * Defubes the possible parity types for the serial port
	 */
	enum class ESerialParity : int
	{
		None			= 0,
		Odd				= 1,
		Even			= 2,
		Mark			= 3,
		Space			= 4
	};


	/**
	 * Serial port device interface. Creates and opens a serial communication port. 
	 * The port is opened using the specified properties when the device is started.
	 * The serial connection is closed when the device is stopped. Valid address names of the serial port would be
	 * something like 'COM1' on Windows an '/dev/ttyS0' on Linux.
	 * By default all serial communication (reading / writing) is non-blocking. 
	 */
	class NAPAPI SerialPort : public Device
	{
		RTTI_ENABLE(Device)
	public:

		/**
		 * Default constructor
		 */
		SerialPort();

		// Stops the device
		virtual ~SerialPort() override;

		/**
		* Initialize this object after de-serialization.
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Opens the serial port using the associated port properties.
		 * @param errorState contains the error if the serial port can't be opened.
		 * @return if the device started
		 */
		virtual bool start(utility::ErrorState& errorState) override;

		/**
		 * Closes the serial port.
		 */
		virtual void stop() override;

		/**
		 * @return if the port is open and ready to communicate with.
		 */
		bool isOpen() const;

		/**
		 * Read a given amount of bytes from the serial port into the given buffer.
		 * The read function will return when the number of requested bytes was read or when a timeout occurs.
		 * A timeout occurs when the inter-byte timeout or when the read timeout has expired.
		 * An error is generated when an exception is thrown.
		 * @param buffer the buffer that will hold the read values.
		 * @param count number of bytes to read.
		 * @return the total number of bytes read.
		 */
		uint32 read(std::vector<uint8>& buffer, uint32 count = 1);

		/**
		 * Read a given amount of bytes from the serial port into the given buffer.
		 * The read function will return when the number of requested bytes was read or when a timeout occurs.
		 * A timeout occurs when the inter-byte timeout or when the read timeout has expired.
		 * An error is generated when an exception is thrown.
		 * @param buffer the buffer that will hold the read values, must be of size count.
		 * @param count number of bytes to read.
		 * @return the total number of bytes read.
		 */
		uint32 read(uint8* buffer, uint32 count);

		/**
		 * Read a given amount of bytes from the serial port into the given buffer.
		 * The read function will return when the number of requested bytes was read or when a timeout occurs.
		 * A timeout occurs when the inter-byte timeout or when the read timeout has expired.
		 * An error is generated when an exception is thrown.
		 * @param buffer the buffer that will hold the read values, must be of size count.
		 * @param count number of bytes to read.
		 * @return the total number of bytes read.
		 */
		uint32 read(std:: string& buffer, uint32 count = 1);

		/**
		 * Read a given amount of bytes from the serial port and return a string containing the data.
		 * @param count the total number of bytes to read.
		 * @return a string that contains the data read from the port.
		 */
		std::string read(uint32 count);

		/**
		 * Reads in a line or until a given delimiter has been processed.
		 * @param buffer string reference used to store the data.
		 * @param length maximum character length of the line.
		 * @param eol end of line delimiter.
		 * @return number of bytes read.
		 */
		uint32 readLine(std::string& buffer, uint32 length = 65536, const std::string& eol = "\n");

		/**
		 * Reads in a line or until a given delimiter has been processed.
		 * @param length maximum character length of the line.
		 * @param eol end of line delimiter.
		 * @return string that contains the line.
		 */
		std::string readLine(uint32 length = 65536, const std::string& eol = "\n");

		/**
		 * Reads multiple lines until the serial port times out.
		 * This requires a timeout > 0 before it can be run. It will read until a timeout occurs and return a list of strings.
		 * @param length maximum length of combined  lines.
		 * @param eol end of line delimiter that is used to separate individual strings.
		 * @return list of strings.
		 */
		std::vector<std::string> readLines(uint32 length = 65536, std::string eol = "\n");

		/**
		 * Write a buffer to the serial port.
		 * @param data pointer to the data that is written, must be of size count.
		 * @param count number of bytes to write
		 * @return number of bytes actually written
		 */
		uint32 write(uint8* data, uint32 count);

		/**
		 * Write a buffer to the serial port.
		 * @param buffer data that is written
		 * @return number of bytes actually written
		 */
		uint32 write(const std::vector<uint8>& data);

		/**
		 * Write a string to the serial port.
		 * @param string string that is written
		 * @return number of bytes actually written
		 */
		uint32 write(const std::string& data);

		/**
		 * @return the number of characters in the buffer
		 */
		uint32 available();

		/**
		 * Block until serial data can be read or number of ms have elapsed.
		 * The return value is true when the function exits with the port in a readable state, false otherwise
		 * (due to timeout or select interruption).
		 */
		bool waitReadable();

		/* Block for a period of time corresponding to the transmission time of the
		 * number of characters based on the present serial settings. This may be used in con-
		 * junction with waitReadable to read larger blocks of data from the port.
		 * @param count number of cycles to wait
		 */
		void waitByteTimes(uint32 count);

		/**
		 * Flush the input and output buffers
		 */
		void flush();

		/**
		 * Flush the input buffers
		 */
		void flushInput();

		/**
		 * Flush the output buffers
		 */
		void flushOutput();

		/**
		 * Sends the RS-232 break signal.
		 */ 
		void sendBreak(uint32 duration);

		/**
		 * Blocks until CTS, DSR, RI, CD changes or something interrupts it.
		 * @return true if one of the lines has changed, false if something else occurred. 
		 */
		bool waitForChange();

		/**
		 * Set the break condition to the given level.
		 * @param level new break level.
		 */ 
		void setBreak(bool level = true);

		/**
		 * Set the RTS handshaking line to the given level. 
		 * @param level the new RTS level
		 */
		void setRTS(bool level = true);

		/**
		 * Set the DTR handshaking line to the given level. 
		 * @param level the new DTR level.
		 */
		void setDTR(bool level = true);

		/**
		 * @return the current status of the CTS line.
		 */
		bool getCTS();

		/**
		 * @return the current status of the DSR line.
		 */
		bool getDSR();

		/**
		 * @return the current status of the RI line.
		 */
		bool getRI();

		/**
		 * @return the current status of the CD line.
		 */
		bool getCD();

		/**
		 * Returns a handle to the underlying serial object.
		 * Careful: direct communication with the serial device is not recommended.
		 * @return the underlying Serial object.
		 */
		const serial::Serial& getSerial() const					{ return *mSerialPort; }

		/**
		 * Returns a handle to the underlying serial object.
		 * Careful: direct communication with the serial device is not recommended.
		 * @return the underlying Serial object.
		 */
		serial::Serial& getSerial()								{ return *mSerialPort; }

		std::string mPortName = "COM1";									///< Property: "PortName" the serial port to open and communicate with. Something like 'COM1' on windows and '/dev/ttyS0' on linux.
		int mBaudRate = 9600;											///< Property: "BaudRate" the baud rate of the serial port.
		ESerialByteSize mByteSize = ESerialByteSize::Eight;				///< Property: "ByteSize" size of each byte in the serial transmission of data.
		ESerialFlowControl mFlowControl = ESerialFlowControl::None;		///< Property: "FlowControl" type of flow control that is used.
		ESerialParity mParity = ESerialParity::None;					///< Property: "Parity" parity method.
		ESerialStopBits mStopBits = ESerialStopBits::One;				///< Property: "StopBits" number of stop bits used.
		int mReadTimeout  = 0;											///< Property: "ReadTimeout" the time in ms until a timeout occurs after a call to read is made. 0 = non blocking mode
		int mWriteTimeout = 0;											///< Property: "WriteTimeout" the time in ms until a timeout occurs after a call to write is made. 0 = non blocking mode
		int mInterByteTimeout = 0;										///< Property: "InterByteTimeout" the max amount of time in ms between receiving bytes that can pass before a timeout occurs. Setting this to 0 will prevent inter byte timeouts.

	private:
		std::unique_ptr<serial::Serial> mSerialPort = nullptr;
	};
}
