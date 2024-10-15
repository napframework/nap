/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External includes
#include <component.h>
#include <mesh.h>
#include <renderablemesh.h>
#include <nap/resourceptr.h>
#include <renderablemeshcomponent.h>
#include <parameternumeric.h>

namespace nap
{
	class SelectRenderComponentInstance;

	/**
	 * Selects a render component based on an index.
	 */
	class NAPAPI SelectRenderComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(SelectRenderComponent, SelectRenderComponentInstance)
	public:
		std::vector<ComponentPtr<RenderableMeshComponent>> mRenderComponents;			///< Property: "RenderComponents" link to render components
		ResourcePtr<ParameterInt> mIndex;												///< Property: "Index" current index
	};


	/**
	 * Instance (runtime version) of the render component selector
	 */
	class NAPAPI SelectRenderComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		SelectRenderComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)	{ }

		/**
		 * Initialize SelectRenderComponentInstance based on the SelectRenderComponent resource
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return the number of selectable render components
		 */
		int getCount() const											{ return mRenderComponents.size(); }

		/**
		 * @return current index
		 */
		int getIndex() const											{ return mIndex; }

		/**
		 * set current index
		 */
		void setIndex(int index)										{ return mIndexParam->setValue(index); }

	private:
		/**
		 * Called when the index has changed
		 * @param index new index, clamped to range
		 */
		void onIndexChanged(int index);
		nap::Slot<int> mIndexChangedSlot = { this, &SelectRenderComponentInstance::onIndexChanged };

		std::vector<ComponentInstancePtr<RenderableMeshComponent>> mRenderComponents = initComponentInstancePtr(this, &SelectRenderComponent::mRenderComponents);

		ParameterInt* mIndexParam = nullptr;
		int mIndex = 0;									//< Current index
	};
}
