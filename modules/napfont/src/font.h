#pragma once

// External Includes
#include <nap/resource.h>
#include <rtti/factory.h>

// Forward Declares
namespace nap
{
	class FontService;
}


namespace nap
{
	/**
	 * Font Resource
	 */
	class NAPAPI Font : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		// Default constructor
		Font() = default;

		// Constructor used by factory registered in the font service
		Font(FontService& service);

		// Destructor
		virtual ~Font();

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		std::string mFont;					///< Property: 'Font' path to the font on disk

	private:
		void* mFace = nullptr;				///< Handle to the free-type face object
		void* mFreetypeLib = nullptr;		///< Handle to the free-type library
		FontService* mService = nullptr;	///< Handle to the service that manages the font library
	};

	// Object creator used for constructing the the OSC receiver
	using FontObjectCreator = rtti::ObjectCreator<Font, FontService>;
}
