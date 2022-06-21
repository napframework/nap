/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "resource.h"
#include "resourceptr.h"
#include "rtti/objectptr.h"
#include "resource.h"

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Group Interface
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Group interface
	 */
	class NAPAPI IGroup : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		/**
		 * @return group member type
		 */
		virtual rtti::TypeInfo memberType() const = 0;

		/**
		 * @return member property name
		 */
		static constexpr const char* propertyName()				{ return "Members"; }

		/**
		 * @return 'Members' rtti property
		 */
		rttr::property getProperty() const;
	};


	//////////////////////////////////////////////////////////////////////////
	// Group
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Groups together a set of objects of a specific type.
	 */
	template<typename T>
	class Group : public IGroup
	{
		RTTI_ENABLE(IGroup)
	public:
		/**
		 * @return group member type
		 */
		virtual rtti::TypeInfo memberType() const override		{ return RTTI_OF(T); }

		/**
		 * Attempts to find a member in this group with the given ID. 
		 * @param id member ID 
		 * @return member with the given ID, nullptr if not found
		 */
		rtti::ObjectPtr<T> findMember(const std::string& id) const;

		/**
		 * Attempts to find a member in this group, with the given ID, of type M
		 * @param id member ID
		 * @return member with the given ID, nullptr if not found or not of the given type
		 */
		template<typename M>
		rtti::ObjectPtr<M> findMember(const std::string& id);

		std::vector<rtti::ObjectPtr<T>> mMembers;				///< Property: 'Members' The members that belong to this group
	};


	//////////////////////////////////////////////////////////////////////////
	// Group Definitions
	//////////////////////////////////////////////////////////////////////////

	// Default NAP (resource) group.
	using ResourceGroup = Group<Resource>;


	//////////////////////////////////////////////////////////////////////////
	// Template Definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	rtti::ObjectPtr<T> nap::Group<T>::findMember(const std::string& id) const
	{
		const auto found_it = std::find_if(mMembers.begin(), mMembers.end(), [&](const auto& it)
			{
				return it->mID == id;
			});
		return found_it != mMembers.end() ? *found_it : nullptr;
	}


	template<typename T>
	template<typename M>
	rtti::ObjectPtr<M> nap::Group<T>::findMember(const std::string& id)
	{
		return rtti::ObjectPtr<M>(findMember(id));
	}
}


/**
 * Use this macro to register your own group.
 * ~~~~~{.cpp}
 * DEFINE_GROUP(nap::ResourceGroup)
 * ~~~~~ 
 */
#define DEFINE_GROUP(Type)																																	\
	RTTI_BEGIN_CLASS(Type)																																	\
		RTTI_PROPERTY(nap::IGroup::propertyName(), &Type::mMembers, nap::rtti::EPropertyMetaData::Embedded | nap::rtti::EPropertyMetaData::ReadOnly)		\
	RTTI_END_CLASS
