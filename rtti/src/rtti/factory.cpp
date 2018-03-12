#include "factory.h"


namespace nap
{
	void rtti::Factory::addObjectCreator(std::unique_ptr<IObjectCreator> objectCreator)
	{
		mCreators.insert(std::make_pair(objectCreator->getTypeToCreate(), std::move(objectCreator)));
	}


	rtti::Object* rtti::Factory::create(rtti::TypeInfo typeInfo)
	{
		CreatorMap::iterator creator = mCreators.find(typeInfo);
		if (creator == mCreators.end())
			return typeInfo.create<Object>();

		return creator->second->create();
	}

	bool rtti::Factory::canCreate(rtti::TypeInfo typeInfo) const
	{
		CreatorMap::const_iterator creator = mCreators.find(typeInfo);
		if (creator == mCreators.end())
			return typeInfo.can_create_instance();

		return true;
	}
}
