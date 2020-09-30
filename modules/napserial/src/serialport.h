#pragma once

// External Includes
#include <nap/device.h>
#include <string.h>
#include <nap/numeric.h>
#include <nap/signalslot.h>

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
		 * Contains the error message when a serial operation fails.
		 */
		struct Error
		{
			/**
			 * All the serial error types
			 */
			enum class EType : int
			{
				NoError = 0,				///< No serial error
				IOError = 1,				///< Serial IO (read / write) error
				SerialError = 2,			///< Generic serial error
				PortError = 3				///< Port isn't open
			};

			/**
			 * @return true if the previous operation failed and generated an error
			 */
			bool failed()								{ return mType != EType::NoError; }

			/**
			 * @return the message associated with this error
			 */
			const std::string& getMessage()				{ return mMessage; }

			/**
			 * @return the status of this error
			 */
			EType getType()								{ return mType; }

			/**
			 * Clears the error message, automatically called by all serial operations.
			 */
			void clear();

			EType mType = EType::NoError;				///< Status associated with this error
			std::string mMessage = "";					///< Message associated with this error
		};

		/**
		 * Default constructor
		 */
		SerialPort();

		// Stops the device
		virtual ~SerialPort();

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
		 * Returns if the port has been opened successfully and is ready to be communicated with.
		 * The port won't be open when startup failed and 'AllowFailure' is turned on.
		 * @return if the port is open and ready to communicate with.
		 */
		bool isOpen() const;

		/**
		 * Read a given amount of bytes from the serial port into the given buffer.
		 * The read function will return when the number of requested bytes was read or when a timeout occurs.
		 * A timeout occurs when the inter-byte timeout or when the read timeout has expired.
		 * An error is generated when an exception is thrown.
		 * The buffer is automatically resized to hold the requested number of bytes.
		 * @param buffer the buffer that will hold the read values.
		 * @param count number of bytes to read. Buffer is resized to fit this number of bytes.
		 * @param error contains the error if the operation fails.
		 * @return the total number of bytes read.
		 */
		uint32 read(std::vector<uint8>& buffer, uint32 count, SerialPort::Error& error);

		/**
		 * Read a given amount of bytes from the serial port into the given buffer.
		 * The read function will return when the number of requested bytes was read or when a timeout occurs.
		 * A timeout occurs when the inter-byte timeout or when the read timeout has expired.
		 * An error is generated when an exception is thrown.
		 * The size of the buffer is used to determine the number of bytes to read.
		 * @param buffer the buffer that will hold the read values.
		 * @param error contains the error if the operation fails.
		 * @return the total number of bytes read.
		 */
		uint32 read(std::vector<uint8>& buffer, SerialPort::Error& error);

		/**
		 * Read a given amount of bytes from the serial port into the given buffer.
		 * The read function will return when the number of requested bytes was read or when a timeout occurs.
		 * A timeout occurs when the inter-byte timeout or when the read timeout has expired.
		 * An error is generated when an exception is thrown.
		 * @param buffer the buffer that will hold the read values, must be of size count.
		 * @param count number of bytes to read.
		 * @param error contains the error if the operation fails.
		 * @return the total number of bytes read.
		 */
		uint32 read(uint8* buffer, uint32 count, SerialPort::Error& error);

		/**
		 * Read a given amount of bytes from the serial port into the given buffer.
		 * The read function will return when the number of requested bytes was read or when a timeout occurs.
		 * A timeout occurs when the inter-byte timeout or when the read timeout has expired.
		 * An error is generated when an exception is thrown.
		 * @param buffer empty string to hold the read values.
		 * @param count number of bytes to read.
		 * @param error contains the error if the operation fails.
		 * @return the total number of bytes read.
		 */
		uint32 read(std:: string& buffer, uint32 count, SerialPort::Error& error);

		/**
		 * Read a given amount of bytes from the serial port and return a string containing the data.
		 * @param count the total number of bytes to read.
		 * @param error contains the error if the operation fails.
		 * @return a string that contains the data read from the port.
		 */
		std::string read(uint32 count, SerialPort::Error& error);

		/**
		 * Reads in a line or until a given delimiter has been processed.
		 * @param buffer empty string that will contain the read data.
		 * @param length maximum character length of the line
		 * @param eol end of line delimiter.
		 * @param error contains the error if the operation fails.
		 * @return number of bytes read.
		 */
		uint32 readLine(std::string& buffer, uint32 length, const std::string& eol, SerialPort::Error& error);

		/**
		 * Reads in a line or until a given delimiter has been processed.
		 * @param length maximum character length of the line.
		 * @param eol end of line delimiter.
		 * @param error contains the error if the operation fails.
		 * @return string that contains the line.
		 */
		std::string readLine(uint32 length, const std::string& eol, SerialPort::Error& error);

		/**
		 * Reads multiple lines until the serial port times out.
		 * This requires a timeout > 0 before it can be run. It will read until a timeout occurs and return a list of strings.
		 * @param length maximum character length of all lines combined.
		 * @param eol end of line delimiter that is used to separate individual strings.
		 * @param error contains the error if the operation fails.
		 * @return list of strings.
		 */
		std::vector<std::string> readLines(uint32 length, const std::string& eol, SerialPort::Error& error);

		/**
		 * Write a buffer to the serial port.
		 * @param data pointer to the data that is written, must be of size count.
		 * @param count number of bytes to write
		 * @param error contains the error if the operation fails.
		 * @return number of bytes actually written
		 */
		uint32 write(const uint8* data, uint32 count, SerialPort::Error& error);

		/**
		 * Write a buffer to the serial port.
		 * @param data data that is written
		 * @param error contains the error if the operation fails.
		 * @return number of bytes actually written
		 */
		uint32 write(const std::vector<uint8>& data, SerialPort::Error& error);

		/**
		 * Write a string to the serial port.
		 * @param data string that is written
		 * @param error contains the error if the operation fails.
		 * @return number of bytes actually written
		 */
		uint32 write(const std::string& data, SerialPort::Error& error);

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
		 * Signal emitted when the serial port is destructed.
		 */
		Signal<SerialPort*> destructed;

		std::string mPortName = "COM1";									///< Property: "PortName" the serial port to open and communicate with. Something like 'COM1' on windows and '/dev/ttyS0' on linux.
		int mBaudRate = 9600;											///< Property: "BaudRate" the baud rate of the serial port.
		ESerialByteSize mByteSize = ESerialByteSize::Eight;				///< Property: "ByteSize" size of each byte in the serial transmission of data.
		ESerialFlowControl mFlowControl = ESerialFlowControl::None;		///< Property: "FlowControl" type of flow control that is used.
		ESerialParity mParity = ESerialParity::None;					///< Property: "Parity" parity method.
		ESerialStopBits mStopBits = ESerialStopBits::One;				///< Property: "StopBits" number of stop bits used.
		int mReadTimeout  = 0;											///< Property: "ReadTimeout" the time in ms until a timeout occurs after a call to read is made. 0 = non blocking mode
		int mWriteTimeout = 0;											///< Property: "WriteTimeout" the time in ms until a timeout occurs after a call to write is made. 0 = non blocking mode
		int mInterByteTimeout = 0;										///< Property: "InterByteTimeout" the max amount of time in ms between receiving bytes that can pass before a timeout occurs. Setting this to 0 will prevent inter byte timeouts.
		bool mAllowFailure = false;										///< Property: "AllowFailure" when set to true opening the port is allowed to fail on startup.
	private:
		std::unique_ptr<serial::Serial> mSerialPort;
		
	};
}
