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
	namespace group
	{
		constexpr const char* members  = "Members";						///< Default group members property name
		constexpr const char* children = "Children";					///< Default group children property name
	}


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
		 * @param memberType the group member type
		 * @param membersName members rtti property name
		 * @param childrenName children rtti property name
		 */
		IGroup(rtti::TypeInfo memberType, std::string&& membersName, std::string&& childrenName) :
			mMemberType(memberType),
			mMembersPropertyName(std::move(membersName)),
			mChildrenPropertyName(std::move(childrenName))				{ }

		/**
		 * @return group member type
		 */
		rtti::TypeInfo getMemberType() const							{ return mMemberType; }

		/**
		 * @return 'Members' rtti property
		 */
		rttr::property getMembersProperty() const;

		/**
		 * @return 'Children' rtti property
		 */
		rttr::property getChildrenProperty() const;

		/**
		 * @return member property name
		 */
		const std::string& membersPropertyName() const					{ return mMembersPropertyName; }

		/**
		 * @return children property name
		 */
		const std::string& childrenPropertyName() const					{ return mChildrenPropertyName; }

	private:
		rtti::TypeInfo mMemberType;
		std::string mMembersPropertyName;
		std::string mChildrenPropertyName;
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
		 * Constructs a group with members of type T
		 */
		Group() : IGroup(RTTI_OF(T), group::members, group::children) { }

		/**
		 * Initialize this group
		 * @param errorState contains the error if initialization fails
		 * @return if initialization succeeded
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Attempts to find a member in this group with the given ID. 
		 * @param id member ID
		 * @param attempt to find member in child groups as well
		 * @return member with the given ID, nullptr if not found
		 */
		rtti::ObjectPtr<T> findObject(const std::string& id) const;

		/**
		 * Attempts to find a member in this group and all child groups with the given ID. 
		 * @param id member ID
		 * @return member with the given ID, nullptr if not found
		 */
		rtti::ObjectPtr<T> findObjectRecursive(const std::string& id) const;

		/**
		 * Attempts to find a member in this group, with the given ID, as type M
		 *
		 *~~~~~{.cpp}
		 * auto window = mGroup->findObject<nap::RenderWindow>("Window0");
		 *~~~~~
		 * 
		 * @param id member ID
		 * @return member with the given ID, nullptr if not found or not of the given type
		 */
		template<typename M>
		rtti::ObjectPtr<M> findObject(const std::string& id) const;

		/**
		 * Attempts to find a member in this group, and all child groups, with the given ID as type M.
		 * 
		 *~~~~~{.cpp}
		 * auto window = mGroup->findObjectRecursive<nap::RenderWindow>("Window0");
		 *~~~~~
		 * 
		 * @param id member ID
		 * @return member with the given ID, nullptr if not found or not of the given type
		 */
		template<typename M>
		rtti::ObjectPtr<M> findObjectRecursive(const std::string& id) const;

		std::vector<rtti::ObjectPtr<T>> mMembers;					///< Property: 'Members' The members that belong to this group
		std::vector<rtti::ObjectPtr<Group<T>>> mChildren;			///< Property: 'Children' The sub groups

	private:
		std::unordered_map<std::string, T*> mMap;					///< Maps ID to member for faster lookup
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
	bool nap::Group<T>::init(utility::ErrorState& errorState)
	{
		mMap.reserve(mMembers.size());
		for (const auto& member : mMembers)
		{
			mMap.emplace(std::make_pair(member->mID, member.get()));
		}
		return true;
	}

	template<typename T>
	rtti::ObjectPtr<T> nap::Group<T>::findObject(const std::string& id) const
	{
		// Find in this group
		auto it = mMap.find(id);
		if (it != mMap.end())
			return rtti::ObjectPtr<T>(it->second);
		return nullptr;
	}

	template<typename T>
	rtti::ObjectPtr<T> nap::Group<T>::findObjectRecursive(const std::string& id) const
	{
		// Find in this group
		rtti::ObjectPtr<T> object = findObject(id);
		if (object != nullptr)
			return object;

		// Find in sub-groups
		for (const auto& child : mChildren)
		{
			rtti::ObjectPtr<T> object = child->findObject(id);
			if (object == nullptr)
				continue;
			return object;
		}
		return nullptr;
	}

	template<typename T>
	template<typename M>
	rtti::ObjectPtr<M> nap::Group<T>::findObject(const std::string& id) const
	{
		return rtti::ObjectPtr<M>(findObject(id));
	}

	template<typename T>
	template<typename M>
	rtti::ObjectPtr<M> nap::Group<T>::findObjectRecursive(const std::string& id) const
	{
		return rtti::ObjectPtr<M>(findObjectRecursive(id));
	}
}


/**
 * Use this macro to register your own group.
 * ~~~~~{.cpp}
 * DEFINE_GROUP(nap::ResourceGroup)
 * ~~~~~ 
 */
#define DEFINE_GROUP(Type)																																\
	RTTI_BEGIN_CLASS(Type)																																\
		RTTI_PROPERTY(nap::group::members,	&Type::mMembers,	nap::rtti::EPropertyMetaData::Embedded | nap::rtti::EPropertyMetaData::ReadOnly)		\
		RTTI_PROPERTY(nap::group::children,	&Type::mChildren,	nap::rtti::EPropertyMetaData::Embedded | nap::rtti::EPropertyMetaData::ReadOnly)		\
	RTTI_END_CLASS
