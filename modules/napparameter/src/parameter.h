#pragma once

// External Includes
#include <rtti/rtti.h>
#include <nap/resource.h>
#include <nap/resourceptr.h>

namespace nap
{
	/**
	 * A parameter is a wrapper around a value that can be specified in json.  It can be used to specify 'metadata' on top of a value, such as the min/max value. 
	 * This 'metadata' can then be used to validate the values being set on it and to offer an appropriate UI
	 *
	 * The actual data is located in derived classes; Parameter itself only serves as a base class
	 */
	class NAPAPI Parameter : public Resource
	{
		RTTI_ENABLE(Resource)
	
	public:
		/** 
		 * Set the value for this parameter from another parameter. The incoming value is guaranteed to be of the same parameter type.
		 *
		 * @param value The parameter to set the new value from
		 */
		virtual void setValue(const Parameter& value) = 0;

		/**
		 * Get the display name for this parameter. If this parameter has a name set, that name is used. Otherwise, the ID is used.
		 *
		 * @return The display name to use
		 */
		const std::string getDisplayName() const { return mName.empty() ? mID : mName; }

		std::string mName;		///< Property 'Name': The name of this property. The name is separate from the ID and doesn't have to be unique.
	};

	/** 
	 * A parameter group serves as a group for sets of parameters. It is used to group parameters together in logical groups.
	 * A group can contain other groups, thus forming a tree structure
	 */
	class NAPAPI ParameterGroup : public Resource
	{
		RTTI_ENABLE(Resource)

	private:
		friend class ParameterService;

		/**
		 * Find a parameter in the current group by name
		 *
		 * @param name The name of the parameter to find
		 * @return The parameter if found. Null otherwise.
		 */
		ResourcePtr<Parameter> findParameter(const std::string& name) const;

		/**
		 * Find a child ParameterGroup with the specified name
		 *
		 * @param name The name of the group to find
		 * @return The group if found. Null otherwise.
		 */
		ResourcePtr<ParameterGroup> findChild(const std::string& name) const;

	public:
		std::vector<ResourcePtr<Parameter>>			mParameters;	///< Property: 'Parameters' the parameters defined in this group
		std::vector<ResourcePtr<ParameterGroup>>	mChildren;		///< Property: 'Groups' the child groups of this group
	};
}
