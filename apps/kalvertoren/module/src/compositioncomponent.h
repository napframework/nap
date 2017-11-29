#pragma once

#include "composition.h"

#include <component.h>
#include <nap/objectptr.h>

namespace nap
{
	class CompositionComponentInstance;

	/**
	 *	compositioncomponent
	 */
	class NAPAPI CompositionComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(CompositionComponent, CompositionComponentInstance)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		std::vector<nap::ObjectPtr<Composition>> mCompositions;		///< Property: All compositions available to the system
		int mIndex = 0;												///< Property: The currently selected composition
	};


	/**
	 * compositioncomponentInstance	
	 */
	class NAPAPI CompositionComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		CompositionComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize compositioncomponentInstance based on the compositioncomponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the compositioncomponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update compositioncomponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

		/**
		 * @return the total number of available compositions
		 */
		int getCount() const													{ return mCompositions.size(); }

		/**
		 * Select the composition that is currently active
		 * @param index the new composition to select
		 */
		void select(int index);

		/**
		 * @return the currently selected composition
		 */
		Composition& getSelection()												{ return *mSelection; }

		/**
		 *	@return the currently selected composition
		 */
		const Composition& getSelection() const									{ return *mSelection; }

	private:
		std::vector<Composition*> mCompositions;								///< List of all available compositions
		Composition* mSelection = nullptr;										///< Currently selected composition
	};
}
