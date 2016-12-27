#pragma once

// Local Includes
#include "coreattributes.h"
#include "resource.h"

namespace nap
{
	/**
	* Links to a resource managed by the resource manager
	* Is able to resolve that link and return the resource this
	* link points to. Constrain the type by giving it a type info
	*/
	class ResourceLinkAttribute : public Attribute<std::string>
	{
		RTTI_ENABLE_DERIVED_FROM(nap::Attribute<std::string>)

	public:
		ResourceLinkAttribute();
		ResourceLinkAttribute(AttributeObject* parent, const std::string& name, const RTTI::TypeInfo& type);

		/**
		* @return the resource this link points to, nullptr if link is not set or invalid
		* This call will resolve the path if a resource is not associated with this attribute
		* @return pointer to the resource, nullptr if not found
		*/
		Resource* getResource();

		/**
		 * @return the resource this link points to, nullptr if empty or not valid
		 * This call will resolve the path if a resource is not associated with this attribute
		 */
		template<typename T>
		T* getResource();

		/**
		 * @return the path to the resource this link points to
		 */
		const std::string& getPath() const							{ return getValue(); }

		/**
		* Sets the target path to the resource
		* is resolved on getting the resource
		*/
		void setResource(const std::string& value)					{ setValue(value); }

		/**
		* Sets the resource this link points to
		* @param resource the resource to link to
		*/
		void setResource(Resource& resource);

		/**
		 * @return if the the resource is currently linked
		 * A resource is linked when a path is to a resource is given or the resource is resolved
		 */
		virtual bool isLinked() const override;

		/**
		 * Clears the link path
		 */
		void clear();

	private:
		// Type of the resource this link should point to
		RTTI::TypeInfo mType = RTTI_OF(nap::Resource);

		// Internally managed resource
		Resource* mResource = nullptr;

		/**
		* Resolves the link using the asset manager service
		* @return if the path has been resolved correctly (in to nullptr when path is empty or resource)
		*/
		bool resolve();

		/**
		* Listener for path changes
		* When the path changes the resource needs to be re-resolved on getting the resource
		* Instead of checking every time by name, use a listener to trigger a resolve
		*/
		void onLinkPathChanged(nap::AttributeBase& attr);
		NSLOT(linkPathChanged, AttributeBase&, onLinkPathChanged)
		bool isDirty = false;
	};

	//////////////////////////////////////////////////////////////////////////
	// Template definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	T* ResourceLinkAttribute::getResource()
	{
		Resource* resource = getResource();
		if (resource == nullptr)
			return nullptr;
		return static_cast<T*>(resource);
	}
}

RTTI_DECLARE(nap::ResourceLinkAttribute)
