#pragma once

// External Includes
#include <nap/resource.h>

namespace nap
{
	/**
	 * Public RSA Key. Used by the nap::LicenseService to validate a license.  
	 * Override the getKey() method in a derived class to embed the key in your application.
	 * Key must be created using the 'keygen' tool.
	 */
	class NAPAPI PublicKey : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		virtual ~PublicKey();

		/**
		 * Needs to be implemented in derived classes.
		 * Returns a public RSA key, generated using the 'keygen' tool.
		 * @return a public RSA key.
		 */
		virtual std::string getKey() const = 0;
	};
}
