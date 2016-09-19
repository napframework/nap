/************************************************************************************
*                                                                                   *
*   Copyright (c) 2013 Axel Menzel <info@axelmenzel.de>                             *
*                                                                                   *
*   This file is part of the Runtime Type Reflection System (RTTI).                 *
*                                                                                   *
*   Permission is hereby granted, free of charge, to any person obtaining           *
*   a copy of this software and associated documentation files (the "Software"),    *
*   to deal in the Software without restriction, including without limitation       *
*   the rights to use, copy, modify, merge, publish, distribute, sublicense,        *
*   and/or sell copies of the Software, and to permit persons to whom the           *
*   Software is furnished to do so, subject to the following conditions:            *
*                                                                                   *
*   The above copyright notice and this permission notice shall be included in      *
*   all copies or substantial portions of the Software.                             *
*                                                                                   *
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR      *
*   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,        *
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE     *
*   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER          *
*   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,   *
*   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE   *
*   SOFTWARE.                                                                       *
*                                                                                   *
*************************************************************************************/

#include "typeinfo.h"

#include <algorithm>
#include <map>
#include <typeinfo>
#include <assert.h>

using namespace std;

typedef map<const string, const RTTI::TypeInfo::TypeId> NameToTag;

#define RTTI_MAX_TYPE_COUNT 32767
#define RTTI_MAX_INHERIT_TYPES_COUNT 100



static std::string* g_nameList = NULL;
static RTTI::TypeInfo::TypeId* g_inheritList = NULL;
static RTTI::TypeInfo::TypeId* g_rawTypeList = NULL;
static RTTI::CreateFunction* g_createFunctionList = NULL;

struct TypeInfoData {
	TypeInfoData() : globalIDCounter(0)
	{
		g_nameList = nameList;
		g_inheritList = inheritList;
		g_rawTypeList = rawTypeList;
		g_createFunctionList = createFunctionList;
	}

	static TypeInfoData& instance()
	{
		static TypeInfoData obj;
		return obj;
	}

	NameToTag nameToTagMap;
	RTTI::TypeInfo::TypeId globalIDCounter;
	std::string nameList[RTTI_MAX_TYPE_COUNT];
	RTTI::TypeInfo::TypeId inheritList[RTTI_MAX_TYPE_COUNT * RTTI_MAX_INHERIT_TYPES_COUNT];
	RTTI::TypeInfo::TypeId rawTypeList[RTTI_MAX_TYPE_COUNT];
	RTTI::CreateFunction createFunctionList[RTTI_MAX_TYPE_COUNT];
};

namespace RTTI
{

	/////////////////////////////////////////////////////////////////////////////////////////

	std::string TypeInfo::getName() const
	{
		if (isValid())
			return g_nameList[m_id];
		else
			return std::string();
	}

	/////////////////////////////////////////////////////////////////////////////////////////

	TypeInfo TypeInfo::getRawType() const
	{
		if (isValid())
			return TypeInfo(g_rawTypeList[m_id]);
		else
			return TypeInfo();
	}

    bool TypeInfo::canCreateInstance() const {
        return (bool) g_createFunctionList[m_id];
    }

	void* TypeInfo::createInstance() const
	{
		assert(canCreateInstance());
        return g_createFunctionList[m_id]();
	}

	/////////////////////////////////////////////////////////////////////////////////////////

	void* TypeInfo::create(const std::string& name)
	{
		TypeInfo type_info = TypeInfo::getByName(name);
		if (type_info.isValid()) return type_info.createInstance();
		return nullptr;
	}

	/////////////////////////////////////////////////////////////////////////////////////////


	bool TypeInfo::isKindOf(const TypeInfo& other) const
	{
		const TypeInfo::TypeId thisRawId = g_rawTypeList[m_id];
		const TypeInfo::TypeId otherRawId = g_rawTypeList[other.m_id];
		if (thisRawId == otherRawId) return true;

		const int row = RTTI_MAX_INHERIT_TYPES_COUNT * thisRawId;
		for (int i = 0; i < RTTI_MAX_INHERIT_TYPES_COUNT; ++i) {
			const TypeInfo::TypeId currId = g_inheritList[row + i];
			if (currId == otherRawId) return true;
			if (currId == 0) // invalid id
				return false;
		}
		return false;
	}

	std::vector<TypeInfo> TypeInfo::getRawTypes(const TypeInfo &kind)
	{
		std::vector<TypeInfo> typeInfos;
		TypeInfoData& data = TypeInfoData::instance();
		for (const auto& itr : data.nameToTagMap) {
			TypeInfo typeInfo = RTTI::impl::getType(itr.first.c_str()).getRawType();

			if (typeInfo == kind) continue;
			if (!typeInfo.isKindOf(kind)) continue;
			if (std::find(typeInfos.begin(), typeInfos.end(), typeInfo) != typeInfos.end()) continue;

			typeInfos.push_back(typeInfo);
		}
		return typeInfos;
	}


	std::vector<TypeInfo> TypeInfo::getRawTypes() {
		std::vector<TypeInfo> typeInfos;
		TypeInfoData& data = TypeInfoData::instance();
		for (const auto& itr : data.nameToTagMap) {
			TypeInfo typeInfo = RTTI::impl::getType(itr.first.c_str()).getRawType();
			if (std::find(typeInfos.begin(), typeInfos.end(), typeInfo) != typeInfos.end()) continue;
			typeInfos.push_back(typeInfo);
		}
		return typeInfos;
	}




	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////

	namespace impl
	{

		/////////////////////////////////////////////////////////////////////////////////////////

		TypeInfo registerOrGetType(const char* name, const TypeInfo& rawTypeInfo,
								   const std::vector<TypeInfo>& baseClassList, CreateFunction createFunction)
		{
			TypeInfoData& data = TypeInfoData::instance();
			{
				NameToTag::const_iterator itr = data.nameToTagMap.find(name);
				if (itr != data.nameToTagMap.end()) return TypeInfo(itr->second);
			}

			const pair<NameToTag::iterator, bool> ret =
				data.nameToTagMap.insert(make_pair(name, ++data.globalIDCounter));
			if (ret.second) {
				g_nameList[data.globalIDCounter] = name;
				const TypeInfo::TypeId rawId =
					((rawTypeInfo.getId() == 0) ? data.globalIDCounter : rawTypeInfo.getId());
				g_rawTypeList[data.globalIDCounter] = rawId;
				g_createFunctionList[data.globalIDCounter] = createFunction;
				const int row = RTTI_MAX_INHERIT_TYPES_COUNT * rawId;
				int index = 0;
				// to do remove double entries
				for (std::vector<TypeInfo>::const_iterator itr = baseClassList.begin(); itr != baseClassList.end();
					 ++itr, ++index) {
					g_inheritList[row + index] = (*itr).getId();
				}
			} // else cannot happen!

			return TypeInfo(ret.first->second);
		}






		TypeInfo getType(const char* name)
		{
			TypeInfoData& data = TypeInfoData::instance();
			NameToTag::const_iterator itr = data.nameToTagMap.find(name);
			if (itr != data.nameToTagMap.end()) return TypeInfo(itr->second);
			return TypeInfo();
		}






		/////////////////////////////////////////////////////////////////////////////////////////

	} // end namespace impl

} // end namespace RTTI
