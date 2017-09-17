#pragma once

// External includes
#include <nap/component.h>
#include <nap/objectptr.h>
#include <renderablemeshcomponent.h>
#include <polyline.h>
#include <transformcomponent.h>
#include <nap/signalslot.h>

namespace nap
{
	class LineSelectionComponentInstance;

	/**
	 * LineSelectionComponent
	 */
	class LineSelectionComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(LineSelectionComponent, LineSelectionComponentInstance)
	public:
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

		// Property: list of selectable poly-lines
		std::vector<ObjectPtr<nap::PolyLine>> mLines;

		/**
		 * @return the currently selected line
		 */
		const nap::PolyLine& getLine() const;

		/**
		 *	@return the currently selected line (non const)
		 */
		nap::PolyLine& getLine();

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

		// Signal that is emitted when the index changes
		nap::Signal<const LineSelectionComponentInstance&> mIndexChanged;

	private:
		void verifyIndex(int index);

		// property: index
		int mIndex = 0;
	};
}