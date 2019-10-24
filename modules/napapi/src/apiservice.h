#pragma once

// Local Includes
#include "apievent.h"

// External Includes
#include <nap/service.h>
#include <queue>
#include <mutex>
#include <nap/signalslot.h>

namespace nap
{
	// Forward Declares
	class APIComponentInstance;

	/**
	 * Offers a C-Style interface that can be used to send and receive data from a running NAP application.
	 * Use this interface to wrap a REST-like interface around your NAP application.
	 *
	 * Use the various utility functions, such as sendInt() and sendIntArray() to send data to a running NAP application.
	 * All 'send' methods copy the arguments by value, making it hard to leak memory.
	 * The copied data is converted (moved) into a nap::APIEvent and forwarded to an interested nap::APIComponent.
	 * Install listeners on an api component to provide logic when a function is called through this interface.
	 *
	 * Messages with variable number of arguments can be constructed and sent using sendMessage().
	 * This call accepts a JSON formatted string that can contain multiple API messages as a bundle, 
	 * where each message contains a variable number of arguments. See sendMessage() for more information.
	 * 
	 * When sending data to an app a unique nap::APIEvent is created. Events aren't processed immediately but stored in a queue.
	 * This ensures that the recording and processing of api events is thread-safe, similar to how osc events are processed.
	 * To process all the recorded api events call processEvents(). processEvents() is called for you automatically when using an application event handler.
	 * 
	 * This service can also be used to dispatch events to an external environment, often as a reply to a previously received message.
	 * In order for an external environment to receive an event it needs to listen to the messageDispatched signal.
	 * Events are dispatched immediately, without being queued.
	 * 
	 * To dispatch an event from a NAP application, often in reply to a previously received message, use dispatchEvent().
	 * Sending messages to a NAP application is completely thread safe.
	 * Dispatching messages from a running NAP application is also completely thread safe.
	 * When processing events manually by calling processEvents() it is recommended to do that from the thread that initializes and shut down NAP.
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
		 * Processing of the generated event is deferred until processEvents() is called.
		 * @param id method associated with float value.
		 * @param value the float value to send.
		 * @param error contains the error if sending fails.
		 * @return if sending succeeded
		 */
		bool sendFloat(const char* id, float value, utility::ErrorState* error);
		
		/**
		 * Send a single string value to a NAP application.
		 * Processing of the generated event is deferred until processEvents() is called.
		 * @param id method associated with string value.
		 * @param value the string to send.
		 * @param error contains the error if sending fails.
		 * @return if sending succeeded
		 */
		bool sendString(const char* id, const char* value, utility::ErrorState* error);
		
		/**
		 * Sends a single int value to a NAP application.
		 * Processing of the generated event is deferred until processEvents() is called.
		 * @param id method associated with int value.
		 * @param value the int to send.
		 * @param error contains the error if sending fails.
		 * @return if sending succeeded
		 */		
		bool sendInt(const char* id, int value, utility::ErrorState* error);
		
		/**
		 * Sends a single byte value to a NAP application.
		 * Processing of the generated event is deferred until processEvents() is called.
		 * @param id method associated with byte value.
		 * @param value the byte to send.
		 * @param error contains the error if sending fails.
		 * @return if sending succeeded
		 */
		bool sendByte(const char* id, nap::uint8 value, utility::ErrorState* error);

		/**
		 * Sends a single bool value to a NAP application.
		 * Processing of the generated event is deferred until processEvents() is called.
		 * @param id method associated with bool value.
		 * @param value the bool to send.
		 * @param error contains the error if sending fails.
		 * @return if sending succeeded
		 */
		bool sendBool(const char* id, bool value, utility::ErrorState* error);

		/**
		 * Sends a single long value to a NAP application.
		 * Processing of the generated event is deferred until processEvents() is called.
		 * @param id method associated with long value.
		 * @param value the long to send.
		 * @param error contains the error if sending fails.
		 * @return if sending succeeded
		 */
		bool sendLong(const char* id, int64_t value, utility::ErrorState* error);
		
		/**
		 * Sends a single char value to a NAP application.
		 * Processing of the generated event is deferred until processEvents() is called.
		 * @param id method associated with char value.
		 * @param value the char to send.
		 * @param error contains the error if sending fails.
		 * @return if sending succeeded
		 */
		bool sendChar(const char* id, char value, utility::ErrorState* error);
		
		/**
		 * Sends a single signal to a NAP application
		 * Processing of the generated event is deferred until processEvents() is called.
		 * @param id method associated with signal.
		 * @param error contains the error if sending fails.
		 * @return if sending succeeded
		 */
		bool send(const char* id, utility::ErrorState* error);
		
		/**
		 * Sends a user constructed api event to a NAP application.
		 * Processing of the generated event is deferred until processEvents() is called.
		 * @param apiEvent the event to forward to the running application
		 * @param error contains the error if sending fails
		 * @return if sending succeeded
		 */
		bool sendEvent(APIEventPtr apiEvent, utility::ErrorState* error);

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
		 * Processing of the generated event is deferred until processEvents() is called.
		 *
		 * Example of a simple API Message in JSON:
		 * ~~~~~
		 * {
				"Objects":
		 *		[
		 *			{
		 *				"Type": "nap::APIMessage",
		 *				"Name": "updateView",
		 *				"mID": "0284761",
		 *				"Arguments":
		 *				[
		 *					{
		 *						"Type": "nap::APILong",
		 *						"mID": "startTime",
		 *						"Value": 2000
		 *					},
		 *					{
		 *						"Type": "nap::APILong",
		 *						"mID": "endTime",
		 *						"Value": 3000
		 *					},
		 *					{
		 *						"Type": "nap::APIInt",
		 *						"mID": "samples",
		 *						"Value": 30
		 *					}
		 *				]
		 *			}
		 *		]
		 *	}
		 * ~~~~~ 
		 * Note that the arguments can be of any type as defined in apivalue.h, including arrays.
		 * Multiple messages can be combined inside the Objects array.
		 * Every api message in that array is converted into an event and given to the nap application.
		 * 
		 * @param json the json string to parse and extract messages from.
		 * @param error contains the error if sending fails.
		 * @return if sending succeeded
		 */
		bool sendMessage(const char* json, utility::ErrorState* error);

		/**
		 * Sends an array of ints to a NAP application, a copy of the data in the array is made.
		 * Processing of the generated event is deferred until processEvents() is called.
		 * @param id method associated with int array.
		 * @param array the array data to send.
		 * @param length the number of elements in the array.
		 * @param error contains the error if sending fails.
		 * @return if sending succeeded
		 */
		bool sendIntArray(const char* id, int* array, int length, utility::ErrorState* error);
		
		/**
		 * Sends an array of floats to a NAP application, a copy of the data in the array is made.
		 * Processing of the generated event is deferred until processEvents() is called.
		 * @param id method associated with float array.
		 * @param array the array data to send.
		 * @param length the number of elements in the array.
		 * @param error contains the error if sending fails.
		 * @return if sending succeeded
		 */
		bool sendFloatArray(const char* id, float* array, int length, utility::ErrorState* error);
		
		/**
		 * Sends an array of bytes to a NAP application, a copy of the data in the array is made.
		 * Processing of the generated event is deferred until processEvents() is called.
		 * @param id method associated with byte array.
		 * @param array the array data to send.
		 * @param length the number of elements in the array.
		 * @param error contains the error if sending fails.
		 * @return if sending succeeded
		 */
		bool sendByteArray(const char* id, uint8_t* array, int length, utility::ErrorState* error);
		
		/**
		 * Sends an array of string values to a NAP application, a copy of the data in the array is made.
		 * Processing of the generated event is deferred until processEvents() is called.
		 * @param id method associated with string array.
		 * @param array the array data to send.
		 * @param length the number of elements in the array.
		 * @param error contains the error if sending fails.
		 * @return if sending succeeded
		 */
		bool sendStringArray(const char* id, const char** array, int length, utility::ErrorState* error);

		/**
		 * Processes all given api commands.
		 * This is called automatically on update but can be called manually from an external environment when there is no application loop.
		 * All events are recorded before being processed, allowing for thread safe execution of the api events.
		 * When calling this manually, call it from the same thread that initializes and shuts down NAP.
		 */
		void processEvents();

		/**
		 * Dispatches an event to an external environment.
		 * NAP applications often dispatch events as a reply to a previously received message, after processing.
		 * Events are dispatched immediately, there is no dispatch queue.
		 * In order for an external environment to receive this message at least one 
		 * type of EventDispatcher needs to be declared in json.
		 * The given event is destroyed after calling this function.
		 * @param apiEvent the event to send to the external environment.
		 */
		void dispatchEvent(nap::APIEventPtr apiEvent);

		/**
		 * Listen to this signal in your external environment to receive outgoing NAP api events.
		 * An event is often dispatched as a reply to a previously received message.
		 */
		nap::Signal<const APIEvent&> eventDispatched;

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

		/**
		 * Consumes and pushes all api events to registered api components
		 */
		virtual void update(double deltaTime) override;

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

		/**
		 * Consumes all recorded events thread safe
		 * @param outEvents the consumed events
		 */
		void consumeEvents(std::queue<APIEventPtr>& outEvents);

		// All the api components currently available to the system
		std::vector<APIComponentInstance*> mAPIComponents;

		// All the api events to process
		std::queue<APIEventPtr> mAPIEvents;

		// Mutex associated with setting / getting api events
		std::mutex	mEventMutex;

		// Mutex associated with component registration and iteration
		std::mutex mComponentMutex;
	};
}