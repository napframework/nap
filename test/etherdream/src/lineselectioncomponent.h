#pragma once

// External includes
#include <nap/component.h>
#include <nap/objectptr.h>
#include <renderablemeshcomponent.h>
#include <polyline.h>

namespace nap
{
	class LineSelectionComponentInstance;

	/**
	 * LineSelectionComponent
	 */
	class LineSelectionComponent : public Component
	{
		RTTI_ENABLE(Component)
	public:
		virtual const rtti::TypeInfo getInstanceType() const override
		{
			return RTTI_OF(LineSelectionComponentInstance);
		}

		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override
		{
			components.emplace_back(RTTI_OF(RenderableMeshComponent));
		}

		// Property: list of selectable poly lines
		std::vector<ObjectPtr<nap::PolyLine>> mLines;

		// property: index of the line
		int mIndex = 0;
	};


	class LineSelectionComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		LineSelectionComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)
		{}

		// Init selection component
		virtual bool init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState) override;

		/**
		 * @return the currently selected line
		 */
		const nap::PolyLine& getLine() const;

		/**
		 *	@return the current line index
		 */
		int getIndex() const					{ return mIndex; }

		/**
		 * Sets the current line index
		 * @param index the new line index
		 */
		void setIndex(int index);

		/**
		 *	@return the number of lines to select from
		 */
		int getCount() const					{ return static_cast<int>(mLines.size()); }

	private:
		void verifyIndex(int index);

		// Property: list of selectable poly-lines
		std::vector<RenderableMesh> mLines;

		// property: index
		int mIndex = 0;

		// Store pointer to transform, set during init
		nap::RenderableMeshComponentInstance* mMeshComponentInstance;
	};
}