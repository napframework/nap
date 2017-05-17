#pragma once

// RTTI includes
#include <rtti/rtti.h>

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
			virtual RTTIObject* create(rtti::TypeInfo) = 0;
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
			void addObjectCreator(rtti::TypeInfo typeInfo, std::unique_ptr<IObjectCreator> objectCreator)
			{
				mCreators.insert(std::make_pair(typeInfo, std::move(objectCreator)));
			}

			/**
			 * Creates an object. If there is an existing type mapping, it will use the ObjectCreator for
			 * that type. If there isn't, the default constructor will be used.
			 * @return instance of the type.
			 * @param typeInfo: the type to create an instance of.
			 */
			RTTIObject* create(rtti::TypeInfo typeInfo)
			{
				CreatorMap::iterator creator = mCreators.find(typeInfo);
				if (creator == mCreators.end())
				{
					return typeInfo.create<RTTIObject>();
				}
				else
				{
					return creator->second->create(typeInfo);
				}
			}

		private:
			using CreatorMap = std::unordered_map<rtti::TypeInfo, std::unique_ptr<IObjectCreator>>;
			CreatorMap mCreators;
		};

	} //< End Namespace nap

}
