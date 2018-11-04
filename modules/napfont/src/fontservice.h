#pragma once

// External Includes
#include <nap/service.h>

namespace nap
{
	/**
	 * Manages the font rendering library.
	 */
	class NAPAPI FontService : public Service
	{
		RTTI_ENABLE(Service)
	public:
		/**
		 *	Default constructor
		 */
		FontService(ServiceConfiguration* configuration);

		/**
		 *	Explicitly frees the font API
		 */
		virtual ~FontService() override;

	protected:
		/**
		 * Initialize font related functionality
		 */
		virtual bool init(utility::ErrorState& error) override;

		/**
		 *	Shuts down all font related functionality
		 */
		virtual void shutdown() override;

	private:
		void* mFreetypeLib = nullptr;		///< Handle to the FreeType library instance
	};
}
