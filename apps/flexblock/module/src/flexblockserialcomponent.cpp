// Local Includes
#include "flexblockserialcomponent.h"

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

		mDestroySlot = Slot<SerialPort*>(this, &FlexBlockSerialComponentInstance::onSerialPortDestroy);

		mSerialPort->mIsBeingDeconstructed.connect(mDestroySlot);

		return true;
	}


	void FlexBlockSerialComponentInstance::onSerialPortDestroy(SerialPort * port)
	{
		stop();
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

		mWriteBuffer.emplace_back(data);

		if (mWriteBuffer.size() > mMaxWriteBufferSize)
		{
			mWriteBuffer.erase(mWriteBuffer.begin()+ mMaxWriteBufferSize, mWriteBuffer.end());
		}
	}


	void FlexBlockSerialComponentInstance::consumeBuffer(std::deque<std::string>& outBuffer)
	{
		std::lock_guard<std::mutex> l(mWriteBufferMutex);

		outBuffer.swap(mWriteBuffer);
	}


	void FlexBlockSerialComponentInstance::writeThreadFunc()
	{
		SerialPort::Error error;

		while (mIsRunning)
		{
			std::deque<std::string> writeBuffer;
			consumeBuffer(writeBuffer);

			if (mSerialPort->isOpen())
			{
				for (const auto& message : writeBuffer)
				{
					mSerialPort->write(message, error);
				}
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(mThreadUpdateIntervalMs));
		}
	}
}
