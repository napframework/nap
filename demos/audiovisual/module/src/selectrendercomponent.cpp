/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "selectrendercomponent.h"

// External Includes
#include <entity.h>
#include <renderablemeshcomponent.h>
#include <mathutils.h>

// nap::SelectRenderComponent run time class definition 
RTTI_BEGIN_CLASS(nap::SelectRenderComponent)
	RTTI_PROPERTY("RenderComponents",	&nap::SelectRenderComponent::mRenderComponents,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Index",				&nap::SelectRenderComponent::mIndex,				nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::SelectRenderComponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SelectRenderComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	bool SelectRenderComponentInstance::init(utility::ErrorState& errorState)
	{
		// Copy over meshes
		nap::SelectRenderComponent* resource = getComponent<SelectRenderComponent>();

		// Select the mesh to display based on resource index
		mIndexParam = resource->mIndex.get();
		mIndexParam->setRange(0, mRenderComponents.size()-1);
		onIndexChanged(mIndexParam->mValue);

		mIndexParam->valueChanged.connect(mIndexChangedSlot);

		return true;
	}


	void SelectRenderComponentInstance::onIndexChanged(int index)
	{
		mIndex = math::clamp<int>(index, 0, mRenderComponents.size() - 1);
		for (int i = 0; i < mRenderComponents.size(); i++)
			mRenderComponents[i]->setVisible(i==mIndex);
	}
}
