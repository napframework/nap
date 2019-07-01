#pragma once

#include <component.h>
#include <serialport.h>
#include <nap/resourceptr.h>
#include <thread>
#include <atomic>
#include <mutex>
#include <queue>
#include <nap/signalslot.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	class FlexBlockSerialComponentInstance;

	/**
	  * FlexBlockSerialComponentInstance
	  * FlexBlockSerial component holds a reference to the serial port and is responsible for 
	  * starting a thread which writes to the serial port.
	  */
	class NAPAPI FlexBlockSerialComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(FlexBlockSerialComponent, FlexBlockSerialComponentInstance)
	public:
		ResourcePtr<SerialPort> mSerialPort; ///< Property: 'Serial Port' reference to the serial port

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;
	};

	//////////////////////////////////////////////////////////////////////////

	/**
	 * FlexBlockSerialComponentInstance	
	 * Instance of FlexBlockSerialComponent
	 */
	class NAPAPI FlexBlockSerialComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		FlexBlockSerialComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		~FlexBlockSerialComponentInstance();

		/**
		 * Initialize FlexBlockSerialComponentInstance based on the FlexBlockSerialComponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the FlexBlockSerialComponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Starts the serial write thread
		 */
		void start(utility::ErrorState& error);

		/**
		 * Stops the serial write thread
		 */
		void stop();

		/**
		 * put string in write buffer
		 * @param data string being put in buffer
		 */ 
		void write(std::string data);

		/**
		 * Sets the write buffer size
		 * This is necessary when data is being put in write buffer at a faster rate
		 * the write thread is writing to serial
		 * @param int max size of write buffer
		 */
		void setMaxWriteBufferSize(const int size) { mMaxWriteBufferSize = size; }

		/**
		 * Sets update interval in milliseconds
		 * @param interval the interval in ms
		 */
		void setUpdateIntervalMs(const long interval) { mThreadUpdateIntervalMs = interval; }

		/**
		 * @return max write buffer size
		 */
		const int getMaxWriteBufferSize() const { return mMaxWriteBufferSize; }

		/**
		 * @return return update interval
		 */
		const long getUpdateIntervalMs() const { return mThreadUpdateIntervalMs; }
	protected:
		void writeThreadFunc();
		void consumeBuffer(std::deque<std::string>& outBuffer);
		void onSerialPortDestroy(SerialPort* port);
	protected:
		Slot<SerialPort*>						mDestroySlot;

		ResourcePtr<SerialPort>					mSerialPort;

        std::atomic_bool						mIsRunning =  { false };
		std::thread								mWriteThread;
		std::deque<std::string>					mWriteBuffer;
		std::mutex								mWriteBufferMutex;
        std::atomic_long						mThreadUpdateIntervalMs = { 10 };
		int										mMaxWriteBufferSize = 4;
	};
}
