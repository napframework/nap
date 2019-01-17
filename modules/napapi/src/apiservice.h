#pragma once

#include <nap/service.h>

namespace nap
{
	/**
	 * Main interface to the NAP API library and associated calls.
	 * Offers a C-Style interface for forwarding external library calls to a running application.
	 */
	class NAPAPI APIService : public Service
	{
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
		 * Send a single float value to NAP.
		 * @param action method associated with float value.
		 * @param value float value.
		 */
		bool sendFloat(const char* action, float value);
		
		/**
		 * Send a single string value to NAP.
		 * @param action method associated with string value.
		 * @param value the string as char array.
		 */
		bool sendString(const char* action, const char* value);
		
		/*
		bool sendInt(const char* action, int value);
		bool sendByte(const char* action, nap::uint8 value);
		bool sendBool(const char* action, bool value);
		bool sendLong(const char* action, long long value);
		bool sendChar(const char* action, char value);
		bool send(const char* action);
		bool sendJSON(const char* action, const char** json);

		bool sendIntArray(const char* action, int* array, int length);
		bool sendFloatArray(const char* action, float* array, int length);
		bool sendByteArray(const char* action, uint8_t* array, int length);
		bool sendStringArray(const char* action, const char**, int* length);
		*/

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
	};
}