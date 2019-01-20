#pragma once

#include <nap/service.h>
#include <apievent.h>

namespace nap
{
	// Forward Declares
	class APIComponentInstance;

	/**
	 * Offers a C-Style interface that can be used to send data to a running NAP application.
	 * Use the various utility functions, such as sendInt() and sendIntArray() to send data to a running NAP application.
	 * All available methods copy the data that is given, making it hard to leak memory.
	 * The copied data is converted (moved) into a nap::APIEvent and transferred to a nap::APIComponent.
	 * The api component actually processes the event, either directly or deferred (on update).
	 * Install listeners on an api component to provide logic when a function is called through this interface.
	 */
	class NAPAPI APIService : public Service
	{
		friend class APIComponentInstance;
		RTTI_ENABLE(Service)
	public:
		/**
		 * Default constructor
		 */
		APIService(ServiceConfiguration* configuration);

		/**
		 * Destructor	
		 */
		~APIService() override;
		
		/**
		 * Send a single float value to a NAP application.
		 * @param id method associated with float value.
		 * @param value the float value to send.
		 * @param error contains the error if sending fails.
		 */
		bool sendFloat(const char* id, float value, utility::ErrorState* error);
		
		/**
		 * Send a single string value to a NAP application.
		 * @param id method associated with string value.
		 * @param value the string to send.
		 * @param error contains the error if sending fails.
		 */
		bool sendString(const char* id, const char* value, utility::ErrorState* error);
		
		/**
		 * Sends a single int value to a NAP application.
		 * @param id method associated with int value.
		 * @param value the int to send.
		 * @param error contains the error if sending fails.
		 */		
		bool sendInt(const char* id, int value, utility::ErrorState* error);
		
		/**
		 * Sends a single byte value to a NAP application.
		 * @param id method associated with byte value.
		 * @param value the byte to send.
		 * @param error contains the error if sending fails.
		 */
		bool sendByte(const char* id, nap::uint8 value, utility::ErrorState* error);

		/**
		 * Sends a single bool value to a NAP application.
		 * @param id method associated with bool value.
		 * @param value the bool to send.
		 * @param error contains the error if sending fails.
		 */
		bool sendBool(const char* id, bool value, utility::ErrorState* error);

		/**
		 * Sends a single long value to a NAP application.
		 * @param id method associated with long value.
		 * @param value the long to send.
		 * @param error contains the error if sending fails.
		 */
		bool sendLong(const char* id, long long value, utility::ErrorState* error);
		
		/**
		 * Sends a single char value to a NAP application.
		 * @param id method associated with char value.
		 * @param value the char to send.
		 * @param error contains the error if sending fails.
		 */
		bool sendChar(const char* id, char value, utility::ErrorState* error);
		
		/**
		 * Sends a single signal to a NAP application
		 * @param id method associated with signal.
		 * @param error contains the error if sending fails.
		 */
		bool send(const char* id, utility::ErrorState* error);
		
		/**
		 * Interprets the string as JSON and sends the individual messages as separate events to a NAP application.
		 * This call extracts nap::Message objects from the json stream. 
		 * Each message is converted into an api event that is forwarded to the application.
		 * Every message holds a custom number of nap::APIValue's and is therefore a powerful way to send custom information to a NAP application.
		 *
		 * After extracting all the messages, which translate into separate events, the system checks if the application accepts the data by matching a declared nap::APISignature.
		 * For the app to accept an event the message's 'mID' property must match one of the nap::Signature resources inside the application.
		 * The amount of nap::APIValue's and the type of those values must also match. This ensures the app always receives correct information.
		 * 
		 * @param json the json string to parse and extract messages from.
		 * @param error contains the error if sending fails.
		 */
		bool sendMessage(const char* json, utility::ErrorState* error);

		/**
		 * Sends an array of ints to a NAP application, a copy of the data in the array is made.
		 * @param id method associated with int array.
		 * @param value the array data to send.
		 * @param length the number of elements in the array.
		 * @param error contains the error if sending fails.
		 */
		bool sendIntArray(const char* id, int* array, int length, utility::ErrorState* error);
		
		/**
		 * Sends an array of floats to a NAP application, a copy of the data in the array is made.
		 * @param id method associated with float array.
		 * @param value the array data to send.
		 * @param length the number of elements in the array.
		 * @param error contains the error if sending fails.
		 */
		bool sendFloatArray(const char* id, float* array, int length, utility::ErrorState* error);
		
		/**
		 * Sends an array of bytes to a NAP application, a copy of the data in the array is made.
		 * @param id method associated with byte array.
		 * @param value the array data to send.
		 * @param length the number of elements in the array.
		 * @param error contains the error if sending fails.
		 */
		bool sendByteArray(const char* id, uint8_t* array, int length, utility::ErrorState* error);
		
		/**
		 * Sends an array of string values to a NAP application, a copy of the data in the array is made.
		 * @param id method associated with string array.
		 * @param value the array data to send.
		 * @param length the number of elements in the array.
		 * @param error contains the error if sending fails.
		 */
		bool sendStringArray(const char* id, const char** array, int length, utility::ErrorState* error);

	protected:
		/**
		 * Initialize the API service.
		 * @param error contains the error message if initialization fails.
		 * @return the if initialization succeeded.
		 */
		virtual bool init(utility::ErrorState& error) override;

		/**
		 * Shuts down the API service	
		 */
		virtual void shutdown() override;

	private:
		/**
		 * Called by the api component in order to register itself with the service.
		 * @param apicomponent the component that wants to register itself.
		 */
		void registerAPIComponent(APIComponentInstance& apicomponent);

		/**
		 * Called by the api component to de-register itself with the service
		 * @param apicomponent the api component to de-register.
		 */
		void removeAPIComponent(APIComponentInstance& apicomponent);

		/**
		 * Forwards a new call event to registered API components	
		 * @param apiEvent api call to forward.
		 * @param error contains the error if the call fails.
		 */
		bool forward(APIEventPtr apiEvent, utility::ErrorState& error);

		// All the osc components currently available to the system
		std::vector<APIComponentInstance*> mAPIComponents;
	};
}