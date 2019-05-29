#pragma once

#include <component.h>
#include <serialport.h>
#include <nap/resourceptr.h>
#include <thread>
#include <atomic>
#include <mutex>

namespace nap
{
	class FlexBlockSerialComponentInstance;

	/**
	 *	FlexBlockSerialComponent
	 */
	class NAPAPI FlexBlockSerialComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(FlexBlockSerialComponent, FlexBlockSerialComponentInstance)
	public:
		ResourcePtr<SerialPort> mSerialPort;

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;
	};


	/**
	 * FlexBlockSerialComponentInstance	
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

		void start(utility::ErrorState& error);

		void stop();

		void write(std::string data);

		const long getUpdateIntervalMs() 
		{
			return mThreadUpdateIntervalMs;
		}

		void setUpdateIntervalMs(long interval)
		{
			mThreadUpdateIntervalMs = interval;
		}
	protected:
		void writeThreadFunc();
	protected:
		
		ResourcePtr<SerialPort>		mSerialPort;

		std::atomic_bool						mIsRunning = false;
		std::thread								mWriteThread;
		std::vector<std::string>				mWriteBuffer;
		std::mutex								mWriteBufferMutex;
		std::atomic_long						mThreadUpdateIntervalMs = 10;
	};
}
