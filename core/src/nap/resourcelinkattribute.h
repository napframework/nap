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
		RTTI_ENABLE(nap::Attribute<std::string>)

	public:
		ResourceLinkAttribute();
		ResourceLinkAttribute(AttributeObject* parent, const std::string& name, const rtti::TypeInfo& type);
		ResourceLinkAttribute(AttributeObject* parent, const std::string& name);

		/**
		* @return the resource this link points to, nullptr if link is not set or invalid
		* This call will resolve the path if a resource is not associated with this attribute
		* @return pointer to the resource, nullptr if not found
		*/
		Resource* getResource() const;

		/**
		 * @return the resource this link points to, nullptr if empty or not valid
		 * This call will resolve the path if a resource is not associated with this attribute
		 */
		template<typename T>
		T* getResource() const;

		/**
		 * @return type of the resource this object links to
		 */
		rtti::TypeInfo getResourceType() const						{ return mType; }

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
		 * Set the resource link type
		 * @param type, type of resource this link is allowed to point to
		 */
		void setResourceType(const rtti::TypeInfo& type);

		/**
		 * @return if the the resource is currently linked
		 * A resource is linked when a path is to a resource is given or the resource is resolved
		 */
        bool isLinked() const;

		/**
		 * Clears the link path
		 */
		void clear();

	private:
		// Type of the resource this link should point to
		rtti::TypeInfo mType = RTTI_OF(nap::Resource);

		// Internally managed resource
		mutable Resource* mResource = nullptr;

		/**
		* Resolves the link using the asset manager service
		* @return if the path has been resolved correctly (in to nullptr when path is empty or resource)
		*/
		bool resolve() const;

		/**
		* Listener for path changes
		* When the path changes the resource needs to be re-resolved on getting the resource
		* Instead of checking every time by name, use a listener to trigger a resolve
		*/
		void onLinkPathChanged(nap::AttributeBase& attr);
		NSLOT(linkPathChanged, AttributeBase&, onLinkPathChanged)
		mutable bool isDirty = true;
	};

	//////////////////////////////////////////////////////////////////////////
	// Template definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	T* ResourceLinkAttribute::getResource() const
	{
		Resource* resource = getResource();
		if (resource == nullptr)
			return nullptr;
		return static_cast<T*>(resource);
	}
}
