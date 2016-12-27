#include "resourcelinkattribute.h"
#include "entity.h"
#include "logger.h"
#include "resourcemanager.h"

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// ResourceLinkAttribute
	//////////////////////////////////////////////////////////////////////////

	// Constructs the resource link using a name and type
	ResourceLinkAttribute::ResourceLinkAttribute(AttributeObject* parent, const std::string& name, const RTTI::TypeInfo& type) :
		Attribute<std::string>(parent, name), mType(type)
	{
		valueChanged.connect(linkPathChanged);
	}


	ResourceLinkAttribute::ResourceLinkAttribute()
	{
		valueChanged.connect(linkPathChanged);
	}


	// Returns the resource this link points to
	Resource* ResourceLinkAttribute::getResource()
	{
		if (isDirty)
			isDirty = resolve();

		// Return resource
		return mResource;
	}


	// Update resource 
	void ResourceLinkAttribute::setResource(Resource& resource)
	{
		// Make sure path to resource is valid
		if (resource.getResourcePath().empty())
		{
			nap::Logger::warn("unable to link resource, invalid resource path");
			return;
		}

		if (!resource.getTypeInfo().isKindOf(mType))
		{
			nap::Logger::warn("unable to link resource, invalid resource type: %s", resource.getTypeInfo().getName().c_str());
			return;
		}

		// Update resource
		mResource = &resource;

		// Set new path (forcing resolve update on get)
		setValue(resource.getResourcePath());
	}


	// If there's currently an active link associated with the resource
	bool ResourceLinkAttribute::isLinked() const
	{
		return !(getValue().empty()) || mResource != nullptr;
	}


	// Clear the link
	void ResourceLinkAttribute::clear()
	{
		setValue("");
	}


	// Resolve path and store resource
	bool ResourceLinkAttribute::resolve()
	{
		if (getValue().empty())
		{
			mResource = nullptr;
			return true;
		}

		// Make sure we have a parent
		if (getParentObject() == nullptr)
		{
			nap::Logger::warn("unable to resolve resource path, attribute link has no parent");
			return false;
		}

		// Find root
		nap::Object* root = getRootObject();
		if (!root->getTypeInfo().isKindOf(RTTI_OF(nap::Entity)))
		{
			nap::Logger::warn("unable to resolve resource path, root object is not of type: %s", RTTI_OF(nap::Entity).getName().c_str());
			return false;
		}

		// Cast to entity
		nap::Entity* root_entity = static_cast<nap::Entity*>(root);
		nap::Core& core = root_entity->getCore();

		// Get service
		nap::ResourceManagerService* service = core.getService<nap::ResourceManagerService>();
		if (service == nullptr)
		{
			nap::Logger::fatal("unable to resolve resource path, resource service is not available");
			return false;
		}

		// Find resource
		nap::Resource* resource = service->getResource(getValue());
		if (resource == nullptr)
		{
			nap::Logger::warn("unable to resolve resource path, resource manager does not contain asset with path: %s", getValue().c_str());
			return false;
		}

		// Make sure the type matches the type specified by the link
		if (!resource->getTypeInfo().isKindOf(mType))
		{
			nap::Logger::warn("unable to resolve resource path, resource is not of type: %s", mType.getName().c_str());
			return false;
		}

		// Set resource
		mResource = resource;
		return true;
	}


	// Checks if the path needs to be resolved
	void ResourceLinkAttribute::onLinkPathChanged(nap::AttributeBase& attr)
	{
		// Always resolve path on get when current resource is empty
		if (mResource == nullptr || getValue() != mResource->getResourcePath())
		{
			isDirty = true;
		}
	}
}