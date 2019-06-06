#include "FlexBlockSerialComponent.h"

// External Includes
#include <entity.h>

// nap::FlexBlockSerialComponent run time class definition 
RTTI_BEGIN_CLASS(nap::FlexBlockSerialComponent)
	RTTI_PROPERTY("SerialPort", &nap::FlexBlockSerialComponent::mSerialPort, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::FlexBlockSerialComponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::FlexBlockSerialComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void FlexBlockSerialComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{

	}

	FlexBlockSerialComponentInstance::~FlexBlockSerialComponentInstance()
	{
		if (mIsRunning)
		{
			mIsRunning = false;
			mWriteThread.join();
		}
	}

	bool FlexBlockSerialComponentInstance::init(utility::ErrorState& errorState)
	{
		FlexBlockSerialComponent* resource = getComponent<FlexBlockSerialComponent>();

		mSerialPort = resource->mSerialPort;

		return true;
	}

	void FlexBlockSerialComponentInstance::start(utility::ErrorState& error)
	{
		if (!mIsRunning)
		{
			mIsRunning = true;
			if(!mSerialPort->isOpen())
				mIsRunning = mSerialPort->start(error);

			if( mIsRunning )
				mWriteThread = std::thread(&FlexBlockSerialComponentInstance::writeThreadFunc, this);
		}
	}

	void FlexBlockSerialComponentInstance::stop()
	{
		if (mIsRunning)
		{
			mIsRunning = false;
			mWriteThread.join();
		}

		if( mSerialPort->isOpen() )
			mSerialPort->stop();
	}

	void FlexBlockSerialComponentInstance::write(std::string data)
	{
		std::lock_guard<std::mutex> l(mWriteBufferMutex);

		mWriteBuffer.push_back(data);
		if (mWriteBuffer.size() > 4)
		{
			mWriteBuffer.erase(mWriteBuffer.begin()+4, mWriteBuffer.end());
		}
		
	}

	void FlexBlockSerialComponentInstance::consumeBuffer(std::vector<std::string>& outBuffer)
	{
		std::lock_guard<std::mutex> l(mWriteBufferMutex);

		outBuffer.clear();
		for (int i = 0; i < mWriteBuffer.size(); i++)
		{
			outBuffer.push_back(mWriteBuffer[i]);
		}
		mWriteBuffer.clear();
	}

	void FlexBlockSerialComponentInstance::writeThreadFunc()
	{
		SerialPort::Error error;

		while (mIsRunning)
		{
			std::vector<std::string> writeBuffer;
			consumeBuffer(writeBuffer);
			printf("%i\n", writeBuffer.size());
			for (int i = 0; i < writeBuffer.size(); i++)
			{
				mSerialPort->write(writeBuffer[i], error);
			}
			
			std::this_thread::sleep_for(std::chrono::milliseconds(mThreadUpdateIntervalMs));
		}
	}
}