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
	};

	/** 
	 * A parameter container serves as a container for sets of parameters. It is used to group parameters together in logical groups.
	 * A container can contain other contains, thus forming a tree structure
	 */
	class NAPAPI ParameterContainer : public Resource
	{
		RTTI_ENABLE(Resource)

	private:
		friend class ParameterService;

		/**
		 * Find a parameter in the current container by name
		 *
		 * @param name The name of the parameter to find
		 * @return The parameter if found. Null otherwise.
		 */
		ResourcePtr<Parameter> findParameter(const std::string& name) const;

		/**
		 * Find a child ParameterContainer with the specified name
		 *
		 * @param name The name of the container to find
		 * @return The container if found. Null otherwise.
		 */
		ResourcePtr<ParameterContainer> findChild(const std::string& name) const;

	public:
		std::vector<ResourcePtr<Parameter>>				mParameters;	///< Property: 'Parameters' the parameters defined in this container
		std::vector<ResourcePtr<ParameterContainer>>	mChildren;		///< Property: 'Children' the child containers of this container
	};
}
