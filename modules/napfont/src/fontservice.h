#pragma once

// External Includes
#include <nap/service.h>

namespace nap
{
	// Forward Declares
	class Font;

	/**
	 * Manages the font rendering library.
	 */
	class NAPAPI FontService : public Service
	{
		friend class FontInstance;
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

		/**
		* Registers all objects that need a specific way of construction, in this case a font resource
		* @param factory the factory to register the object creators with
		*/
		virtual void registerObjectCreators(rtti::Factory& factory) override;

		/**
		 * Returns the handle to the free-type library, only accessible by resources in this module
		 * Represents a FT_Library object
		 * @return the handle to the free-type library
		 */
		void* getHandle() const;

	private:
		void* mFreetypeLib = nullptr;		///< Handle to the FreeType library instance
	};
}
