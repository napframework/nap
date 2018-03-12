#pragma once

// RTTI includes
#include "object.h"
#include "utility/dllexport.h"

namespace nap
{
	namespace rtti
	{
		class Object;

		/**
		 * Derive from this object to apply custom constructor arguments to resources.
		 * The system uses this object to create a new resource instead of invoking the default constructor after serialization
		 * This allows for the implementation of custom construction behavior of resources after serialization.
		 */
		class NAPAPI IObjectCreator
		{
		public:
			virtual ~IObjectCreator() = default;

			/**
			* Creates the object specified with getCreationType()
			*/
			virtual Object* create() = 0;

			/**
			* @return the type this object creates
			*/
			virtual rtti::TypeInfo getTypeToCreate() const = 0;
		};


		/**
		 * Allows for easy construction of a resource using a single argument.
		 * The template parameter Object specifies the type of resource this creator returns
		 * The template parameter T specifies the type of input argument that is used to construct the new resource, ie:
		 * ObjectCreator<Image, RenderService> specifies an object creator that creates an image that is constructed using
		 * the render service as input argument.
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
			* @return Creates the object with the (...yes?)
			*/
			virtual rtti::Object* create() override			{ return new Object(mArgument); }

		private:
			T& mArgument;
		};


		/**
		 * Manages all the custom defined object creators.
		 * This class is given to a service when the system collects all custom object creators
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
			Object* create(rtti::TypeInfo typeInfo);

			/**
			 * @return If the type in @typeInfo is registered into the Factory, returns true. If the type is not 
			 * registered, the RTTI system is queried if it can be created.
			 */
			bool canCreate(rtti::TypeInfo typeInfo) const;

		private:
			using CreatorMap = std::unordered_map<rtti::TypeInfo, std::unique_ptr<IObjectCreator>>;
			CreatorMap mCreators;
		};

	} //< End Namespace nap

}
