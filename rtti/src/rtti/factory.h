#pragma once

// RTTI includes
#include "rttiobject.h"
#include "utility/dllexport.h"

namespace nap
{
	namespace rtti
	{
		class RTTIObject;

		/**
		 * Derive from this object to supply custom constructor arguments to objects.
		 */
		class NAPAPI IObjectCreator
		{
		public:
			virtual ~IObjectCreator() = default;

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
		* Allows easy construction of object @Object using a single argument @T.
		*/
		template <typename Object, typename T>
		class ObjectCreator : public rtti::IObjectCreator
		{
		public:
			/**
			* Constructor
			* @param service: the service to associate with this object creator
			*/
			ObjectCreator(T& argument) :
				mArgument(argument) { }

			/**
			* @return type to create
			*/
			rtti::TypeInfo getTypeToCreate() const override		{ return RTTI_OF(Object); }

			/**
			* @return Creates the object with the
			*/
			virtual rtti::RTTIObject* create() override			{ return new Object(mArgument); }

		private:
			T& mArgument;
		};


		/**
		 * Manages IObjectCreators.
		 */
		class NAPAPI Factory
		{
		public:
			Factory() = default;
			Factory(const Factory&) = delete;
			Factory& operator=(const Factory&) = delete;

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
