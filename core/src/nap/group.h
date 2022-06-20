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
	 * Interface of a group
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
		virtual rtti::TypeInfo memberType() const override		{ return RTTI_OF(T); }
		std::vector<rtti::ObjectPtr<T>> mMembers;				///< Property: 'Members' The members that belong to this group
	};


	//////////////////////////////////////////////////////////////////////////
	// Group Definitions
	//////////////////////////////////////////////////////////////////////////

	// Default NAP (resource) group.
	using ResourceGroup = Group<Resource>;
}


// Use this macro to register your own group
// For example: DEFINE_GROUP(nap::ResourceGroup)
#define DEFINE_GROUP(Type)																																	\
	RTTI_BEGIN_CLASS(Type)																																	\
		RTTI_PROPERTY(nap::IGroup::propertyName(), &Type::mMembers, nap::rtti::EPropertyMetaData::Embedded | nap::rtti::EPropertyMetaData::ReadOnly)		\
	RTTI_END_CLASS
