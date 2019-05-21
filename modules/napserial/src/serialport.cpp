// Local Includes
#include "serialport.h"

// External Includes
#include <nap/logger.h>
#include <serial/serial.h>

RTTI_BEGIN_ENUM(nap::ESerialByteSize)
	RTTI_ENUM_VALUE(nap::ESerialByteSize::Five,			"Five"),
	RTTI_ENUM_VALUE(nap::ESerialByteSize::Six,			"Six"),
	RTTI_ENUM_VALUE(nap::ESerialByteSize::Seven,		"Seven"),
	RTTI_ENUM_VALUE(nap::ESerialByteSize::Eight,		"Eight")
RTTI_END_ENUM

RTTI_BEGIN_ENUM(nap::ESerialFlowControl)
	RTTI_ENUM_VALUE(nap::ESerialFlowControl::None,		"None"),
	RTTI_ENUM_VALUE(nap::ESerialFlowControl::Hardware,	"Hardware"),
	RTTI_ENUM_VALUE(nap::ESerialFlowControl::Software,	"Software")
RTTI_END_ENUM

RTTI_BEGIN_ENUM(nap::ESerialParity)
	RTTI_ENUM_VALUE(nap::ESerialParity::None,			"None"),
	RTTI_ENUM_VALUE(nap::ESerialParity::Odd,			"Odd"),
	RTTI_ENUM_VALUE(nap::ESerialParity::Even,			"Even"),
	RTTI_ENUM_VALUE(nap::ESerialParity::Mark,			"Mark"),
	RTTI_ENUM_VALUE(nap::ESerialParity::Space,			"Space")
RTTI_END_ENUM

RTTI_BEGIN_ENUM(nap::ESerialStopBits)
	RTTI_ENUM_VALUE(nap::ESerialStopBits::One,			"One"),
	RTTI_ENUM_VALUE(nap::ESerialStopBits::Two,			"Two"),
	RTTI_ENUM_VALUE(nap::ESerialStopBits::OnePointFive, "1.5")
RTTI_END_ENUM

// nap::serialport run time class definition 
RTTI_BEGIN_CLASS(nap::SerialPort)
	RTTI_PROPERTY("PortName",			&nap::SerialPort::mPortName,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("BaudRate",			&nap::SerialPort::mBaudRate,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ByteSize",			&nap::SerialPort::mByteSize,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Parity",				&nap::SerialPort::mParity,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("StopBits",			&nap::SerialPort::mStopBits,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FlowControl",		&nap::SerialPort::mFlowControl,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ReadTimeout",		&nap::SerialPort::mReadTimeout,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("WriteTimeout",		&nap::SerialPort::mWriteTimeout,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("InterByteTimeout",	&nap::SerialPort::mInterByteTimeout,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{

	SerialPort::SerialPort()
	{
		// Create port
		mSerialPort = std::make_unique<serial::Serial>();
	}

	SerialPort::~SerialPort()
	{ 
		stop(); 
	}


	bool SerialPort::init(utility::ErrorState& errorState)
	{
		// Ensure baud rate is positive
		if (!errorState.check(mBaudRate > 0, "invalid baud rate specified"))
			return false;

		// Ensure read timeout range is ok
		if (!errorState.check(mReadTimeout >= 0, "invalid read timeout specified"))
			return false;

		// Ensure write timeout range is ok
		if (!errorState.check(mWriteTimeout >= 0, "invalid write timeout specified"))
			return false;

		// Ensure inter byte timeout range is ok
		if (!errorState.check(mInterByteTimeout >= 0, "invalid inter-byte timeout specified"))
			return false;

		// Setup the serial port
		try
		{
			mSerialPort->setBaudrate(static_cast<uint32>(mBaudRate));
			mSerialPort->setPort(mPortName);
			mSerialPort->setBytesize(static_cast<serial::bytesize_t>(mByteSize));
			mSerialPort->setParity(static_cast<serial::parity_t>(mParity));
			mSerialPort->setStopbits(static_cast<serial::stopbits_t>(mStopBits));
			mSerialPort->setFlowcontrol(static_cast<serial::flowcontrol_t>(mFlowControl));
			mSerialPort->setTimeout(static_cast<uint32>(mInterByteTimeout), static_cast<uint32>(mReadTimeout), 0, static_cast<uint32>(mWriteTimeout), 0);
		}
		catch(const std::exception& exception)
		{
			errorState.fail("Failed to setup SerialPort: %s", exception.what());
			return false;
		}

		return true;
	}


	bool SerialPort::start(utility::ErrorState& errorState)
	{
		// Open serial port based on previously initialized settings
		try
		{
			mSerialPort->open();
		}
		catch (const std::exception& exception)
		{
			errorState.fail("Failed to open SerialPort: %s", exception.what());
			return false;
		}
		return true;
	}


	void SerialPort::stop()
	{
		mSerialPort->close();
	}


	bool SerialPort::isOpen() const
	{
		return mSerialPort->isOpen();
	}


	nap::uint32 SerialPort::read(std::vector<uint8>& buffer, uint32 count /*= 1*/)
	{
		try
		{
			return static_cast<uint32>(mSerialPort->read(buffer, static_cast<size_t>(count)));
		}
		catch (const std::exception& exception)
		{
			nap::Logger::error("%s: exception occurred: %s", this->mID.c_str(), exception.what());
			return 0;
		}
	}


	nap::uint32 SerialPort::read(uint8* buffer, uint32 count)
	{
		try
		{
			return static_cast<uint32>(mSerialPort->read(buffer, static_cast<size_t>(count)));
		}
		catch (const std::exception& exception)
		{
			nap::Logger::error("%s: exception occurred: %s", this->mID.c_str(), exception.what());
			return 0;
		}
	}


	nap::uint32 SerialPort::read(std::string& buffer, uint32 count /*= 1*/)
	{
		try
		{
			return static_cast<uint32>(mSerialPort->read(buffer, static_cast<size_t>(count)));
		}
		catch (const std::exception& exception)
		{
			nap::Logger::error("%s: exception occurred: %s", this->mID.c_str(), exception.what());
			return 0;
		}
	}


	std::string SerialPort::read(uint32 count)
	{
		try
		{
			return mSerialPort->read(static_cast<size_t>(count));
		}
		catch (const std::exception& exception)
		{
			nap::Logger::error("%s: exception occurred: %s", this->mID.c_str(), exception.what());
			return "";
		}
	}


	std::string SerialPort::readLine(uint32 length /*= 65536*/, const std::string& eol /*= "\n"*/)
	{
		try
		{
			return mSerialPort->readline(length, eol);
		}
		catch (const std::exception& exception)
		{
			nap::Logger::error("%s: exception occurred: %s", this->mID.c_str(), exception.what());
			return "";
		}
	}


	nap::uint32 SerialPort::readLine(std::string& buffer, uint32 length /*= 65536*/, const std::string& eol /*= "\n"*/)
	{
		try
		{
			return mSerialPort->readline(buffer, length, eol);
		}
		catch (const std::exception& exception)
		{
			nap::Logger::error("%s: exception occurred: %s", this->mID.c_str(), exception.what());
			return 0;
		}
	}


	std::vector<std::string> SerialPort::readLines(uint32 length /*= 65536*/, std::string eol /*= "\n"*/)
	{
		try
		{
			return mSerialPort->readlines(length, eol);
		}
		catch (const std::exception& exception)
		{
			nap::Logger::error("%s: exception occurred: %s", this->mID.c_str(), exception.what());
			return {};
		}
	}


	nap::uint32 SerialPort::write(uint8* data, uint32 count)
	{
		try
		{
			return mSerialPort->write(data, count);
		}
		catch (const std::exception& exception)
		{
			nap::Logger::error("%s: exception occurred: %s", this->mID.c_str(), exception.what());
			return 0;
		}
	}


	nap::uint32 SerialPort::write(const std::vector<uint8>& data)
	{
		try
		{
			return mSerialPort->write(data);
		}
		catch (const std::exception& exception)
		{
			nap::Logger::error("%s: exception occurred: %s", this->mID.c_str(), exception.what());
			return 0;
		}
	}


	nap::uint32 SerialPort::write(const std::string& data)
	{
		try
		{
			return mSerialPort->write(data);
		}
		catch (const std::exception& exception)
		{
			nap::Logger::error("%s: exception occurred: %s", this->mID.c_str(), exception.what());
			return 0;
		}
	}


	void SerialPort::flush()
	{
		mSerialPort->flush();
	}


	void SerialPort::flushInput()
	{
		mSerialPort->flushInput();
	}


	void SerialPort::flushOutput()
	{
		mSerialPort->flushOutput();
	}


	void SerialPort::sendBreak(uint32 duration)
	{
		mSerialPort->sendBreak(static_cast<int>(duration));
	}


	bool SerialPort::waitForChange()
	{
		try
		{
			return mSerialPort->waitForChange();
		}
		catch (const std::exception& exception)
		{
			nap::Logger::error("%s: exception occurred: %s", this->mID.c_str(), exception.what());
			return false;
		}
	}


	void SerialPort::setBreak(bool level /*= true*/)
	{
		mSerialPort->setBreak(level);
	}


	void SerialPort::setRTS(bool level /*= true*/)
	{
		mSerialPort->setRTS(level);
	}


	void SerialPort::setDTR(bool level /*= true*/)
	{
		mSerialPort->setDTR(level);
	}


	bool SerialPort::getCTS()
	{
		return mSerialPort->getCTS();
	}


	bool SerialPort::getDSR()
	{
		return mSerialPort->getDSR();
	}


	bool SerialPort::getRI()
	{
		return mSerialPort->getRI();
	}


	bool SerialPort::getCD()
	{
		return mSerialPort->getCD();
	}


	nap::uint32 SerialPort::available()
	{
		return static_cast<uint32>(mSerialPort->available());
	}


	bool SerialPort::waitReadable()
	{
		return mSerialPort->waitReadable();
	}


	void SerialPort::waitByteTimes(uint32 count)
	{
		mSerialPort->waitByteTimes(count);
	}

}