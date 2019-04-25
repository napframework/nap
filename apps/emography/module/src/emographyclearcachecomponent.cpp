#include "emographyclearcachecomponent.h"
#include "emographystress.h"

// External Includes
#include <entity.h>

// nap::emographyclearcomponent run time class definition 
RTTI_BEGIN_CLASS(nap::emography::ClearCacheComponent)
	RTTI_PROPERTY("DataModel", &nap::emography::ClearCacheComponent::mDataModel, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::emographyclearcomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::emography::ClearCacheComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	namespace emography
	{
		void ClearCacheComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
		{

		}


		bool ClearCacheComponentInstance::init(utility::ErrorState& errorState)
		{
			mDataModel = &(getComponent<ClearCacheComponent>()->mDataModel->getInstance());
			return true;
		}


		bool ClearCacheComponentInstance::clearCache(nap::utility::ErrorState& error)
		{
			std::vector<rtti::TypeInfo> types = mDataModel->getRegisteredTypes();
			for (const auto& type : types)
			{
				if (!mDataModel->clearData(type, error))
				{
					error.fail("Failed to clear data");
					return false;
				}
			}
			return true;
		}
	}
}