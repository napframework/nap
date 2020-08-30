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
		 * Allows for easy construction of a resource using a single argument of type T.
		 * The template parameter Object specifies the type of resource this creator returns
		 * The template parameter T specifies the type of input argument that is used to construct the new resource, ie:
		 * ObjectCreator<Image, RenderService> specifies an object creator that creates an image that is constructed using
		 * the render service as an input argument.
		 */
		template <typename Object, typename T>
		class ObjectCreator : public rtti::IObjectCreator
		{
		public:
			/**
			* @param argument reference to the object that becomes the first input argument when the resource 
			* is constructed after serialization
			*/
			ObjectCreator(T& argument) :
				mArgument(argument) { }

			/**
			* @return type to create
			*/
			rtti::TypeInfo getTypeToCreate() const override		{ return RTTI_OF(Object); }

			/**
			* @return Constructs the resource using as a first argument the object stored in this class
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
			virtual ~Factory() = default;

			Factory& operator=(const Factory&) = delete;

			/**
			 * Adds association between a type and it's object creator.
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
			 * @return If the type is registered into the Factory, returns true. If the type is not 
			 * registered, the RTTI system is queried if it can be created.
			 */
			bool canCreate(rtti::TypeInfo typeInfo) const;

		protected:
			virtual Object* createDefaultObject(rtti::TypeInfo typeInfo);

		private:
			using CreatorMap = std::unordered_map<rtti::TypeInfo, std::unique_ptr<IObjectCreator>>;
			CreatorMap mCreators;
		};


	} //< End Namespace nap

}
