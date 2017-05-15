#pragma once

// RTTI includes
#include <rtti/rtti.h>

namespace nap
{
	class IObjectCreator
	{
	public:
		virtual Object* create(RTTI::TypeInfo) = 0;
	};

	class Factory
	{
	public:
		void addObjectCreator(RTTI::TypeInfo typeInfo, std::unique_ptr<IObjectCreator> objectCreator)
		{
			mCreators.insert(std::make_pair(typeInfo, std::move(objectCreator)));
		}

		Object* Create(RTTI::TypeInfo typeInfo)
		{
			CreatorMap::iterator creator = mCreators.find(typeInfo);
			if (creator == mCreators.end())
			{
				return typeInfo.create<Object>();
			}
			else
			{
				return creator->second->create(typeInfo);
			}
		}

	private:
		using CreatorMap = std::unordered_map<RTTI::TypeInfo, std::unique_ptr<IObjectCreator>>;
		CreatorMap mCreators;
	};

} //< End Namespace nap

