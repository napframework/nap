#include "selectcolormethodcomponent.h"
#include "applycolorcomponent.h"

// External Includes
#include <entity.h>
#include <mathutils.h>
#include <nap/logger.h>

// nap::selectcolormethodcomponent run time class definition 
RTTI_BEGIN_CLASS(nap::SelectColorMethodComponent)
RTTI_END_CLASS

// nap::selectcolormethodcomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SelectColorMethodComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void SelectColorMethodComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{

	}


	bool SelectColorMethodComponentInstance::init(utility::ErrorState& errorState)
	{
		return true;
	}


	void SelectColorMethodComponentInstance::registerMeshColorComponent(ApplyColorComponentInstance& colorer)
	{
		mPaintComponents.emplace_back(&colorer);
	}


	void SelectColorMethodComponentInstance::select(int index)
	{
		mCurrentSelection = math::clamp<int>(index, 0, getSelectionCount()-1);
	}


	void SelectColorMethodComponentInstance::select(nap::rtti::TypeInfo type)
	{
		for (int i = 0; i < getSelectionCount(); i++)
		{
			if (mPaintComponents[i]->get_type() == type)
			{
				select(i);
				return;
			}
		}
		nap::Logger::warn("painter type doesn't exist: %s", type.get_name().data());
	}


	void SelectColorMethodComponentInstance::update(double deltaTime)
	{
		int count = 0;
		for (auto& it : mPaintComponents)
		{
			if (count == mCurrentSelection)
				it->enable();
			else
				it->disable();
			count++;
		}
	}
}