#pragma once

// Local Includes
#include <unordered_map>
#include "componentresourcepath.h"
#include "rtti/typeinfo.h"

namespace nap
{
	namespace rtti
	{
		class Object;
	}

	/**
	 * Item class for ObjectGraph usage.
	 * Wraps both an RTTIObject and a File object (by filename).
	 * Uses RTTI traversal to scan pointers to other objects and pointers to files.
	 */
	class EntityObjectGraphItem
	{
	public:
		using Type = rtti::Object*;
		using ClonedResourceMap = std::unordered_map<rtti::Object*, std::vector<rtti::Object*>>;
		using ObjectsByTypeMap  = std::unordered_map<rtti::TypeInfo, std::vector<rtti::Object*>>;

		enum class EType : uint8_t
		{
			Object,
			File
		};

		/**
		 * Creates a graph item.
		 * @param object Object to wrap in the item that is created.
		 * @param objectsByType used to track and group objects by type.
		 * @param clonedResourceMap used to track cloned resources.
		 */
		static const EntityObjectGraphItem create(rtti::Object* object, const ObjectsByTypeMap& objectsByType, const ClonedResourceMap& clonedResourceMap);

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
		bool getPointees(std::vector<EntityObjectGraphItem>& pointees, utility::ErrorState& errorState) const;
		
		EType						mType;							// Type: file or object
		std::string					mFilename;						// If type is file, contains filename
		rtti::Object*				mObject = nullptr;				// If type is object, contains object pointer
		const ObjectsByTypeMap*		mObjectsByType = nullptr;		// All objects sorted by type
		const ClonedResourceMap*	mClonedResourceMap = nullptr;	// All cloned resources
	};

	template<typename ITEM> class ObjectGraph;
	using EntityObjectGraph = ObjectGraph<EntityObjectGraphItem>;
}
