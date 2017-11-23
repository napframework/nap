#pragma once

// Local Includes
#include <unordered_map>
#include "rtti/typeinfo.h"

namespace nap
{
	namespace rtti
	{
		class RTTIObject;
	}

	namespace utility
	{
		class ErrorState;
	}

	/**
	 * Item class for ObjectGraph usage.
	 * Wraps both an RTTIObject and a File object (by filename).
	 * Uses RTTI traversal to scan pointers to other objects and pointers to files.
	 */
	class RTTIObjectGraphItem
	{
	public:
		using Type = rtti::RTTIObject*;

		enum class EType : uint8_t
		{
			Object,
			File
		};

		/**
		 * Creates a graph item.
		 * @param object Object to wrap in the item that is created.
		 */
		static const RTTIObjectGraphItem create(rtti::RTTIObject* object);

		/**
		 * @return ID of the item. For objects, the ID is the object ID, for files, it is the filename.
		 */
		const std::string getID() const;

		/**
		 * @return EType of the type (file or object).
		 */
		uint8_t getType() const { return (uint8_t)mType; }


		/**
		 * Performs rtti traversal of pointers to both files and objects.
		 * @param pointees Output parameter, contains all objects and files this object points to.
		 * @param errorState If false is returned, contains information about the error.
		 * @return true is succeeded, false otherwise.
		 */
		bool getPointees(std::vector<RTTIObjectGraphItem>& pointees, utility::ErrorState& errorState) const;
		
		EType						mType;							// Type: file or object
		std::string					mFilename;						// If type is file, contains filename
		rtti::RTTIObject*			mObject = nullptr;				// If type is object, contains object pointer
	};

	template<typename ITEM> class ObjectGraph;
	using RTTIObjectGraph = ObjectGraph<RTTIObjectGraphItem>;
}
