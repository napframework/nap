#include "factory.h"


void nap::rtti::Factory::addObjectCreator(std::unique_ptr<IObjectCreator> objectCreator)
{
	mCreators.insert(std::make_pair(objectCreator->getTypeToCreate(), std::move(objectCreator)));
}


nap::rtti::RTTIObject* nap::rtti::Factory::create(rtti::TypeInfo typeInfo)
{
		CreatorMap::iterator creator = mCreators.find(typeInfo);
		if (creator == mCreators.end())
		{
			return typeInfo.create<RTTIObject>();
		}
		return creator->second->create();
}



