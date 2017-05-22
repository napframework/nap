#pragma once

// RTTI includes
#include "rttiobject.h"

namespace nap
{
	namespace rtti
	{
		class RTTIObject;

		/**
		 * Derive from this object to supply custom constructor arguments to objects.
		 */
		class IObjectCreator
		{
		public:
			/**
			* Creates the object specified with getCreationType()
			*/
			virtual RTTIObject* create() = 0;

			/**
			* @return the type this object creates
			*/
			virtual rtti::TypeInfo getTypeToCreate() const = 0;
		};


		/**
		 * Manages IObjectCreators.
		 */
		class Factory
		{
		public:
			/**
			 * Adds association between a type and it's object creator.
			 * @param typeInfo: the RTTI type to create a mapping for.
			 * @param objectCreator: the object that can create instances of the type.
			 */
			void addObjectCreator(std::unique_ptr<IObjectCreator> objectCreator);

			/**
			 * Creates an object. If there is an existing type mapping, it will use the ObjectCreator for
			 * that type. If there isn't, the default constructor will be used.
			 * @return instance of the type.
			 * @param typeInfo: the type to create an instance of.
			 */
			RTTIObject* create(rtti::TypeInfo typeInfo);

		private:
			using CreatorMap = std::unordered_map<rtti::TypeInfo, std::unique_ptr<IObjectCreator>>;
			CreatorMap mCreators;
		};

	} //< End Namespace nap

}
