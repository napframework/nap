#pragma once

#include "composition.h"

#include <component.h>
#include <nap/objectptr.h>

namespace nap
{
	class CompositionComponentInstance;

	/**
	 *	Determines how the compositioncomponent cycles through the available compositions
	 */
	enum class CompositionCycleMode : int
	{
		Off			= 0,			///< Composition does not cycle automatically
		Random		= 1,			///< A new composition is chosen when the active one finishes
		Sequence	= 2				///< Plays through all the compositions one by one
	};


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

		std::vector<nap::ObjectPtr<Composition>>	mCompositions;								///< Property: All compositions available to the system
		int											mIndex = 0;									///< Property: The currently selected composition
		float										mDurationScale = 1.0f;						///< Property: Acts as a scale on the duration of the composition
		CompositionCycleMode						mCycleMode = CompositionCycleMode::Off;		///< Property: How the component cycles through all the available sequences
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
		 * @return the currently selected composition instance
		 */
		CompositionInstance& getSelection()										{ return *mCompositionInstance; }

		/**
		 *	@return the currently selected composition instance
		 */
		const CompositionInstance& getSelection() const							{ return *mCompositionInstance; }

		/**
		 * Changes the duration
		 * @param scale the scale applied to the time associated with all the compositions
		 */
		void setDurationScale(float scale);

		/**
		 * Sets the current cycle mode
		 * @param mode the new cycle mode
		 */
		void setCycleMode(CompositionCycleMode mode)							{ mCycleMode = mode; }

		/**
		 *	@return the current composition cycle mode
		 */
		CompositionCycleMode getCycleMode() const								{ return mCycleMode; }

	private:
		std::vector<Composition*>				mCompositions;								///< List of all available compositions
		Composition*							mSelection = nullptr;						///< Currently selected composition
		int										mCurrentIndex = -1;							///< Currently selected composition @index
		std::unique_ptr<CompositionInstance>	mCompositionInstance;						///< CompositionInstance created when switching compositions
		CompositionCycleMode					mCycleMode = CompositionCycleMode::Off;		///< How this component cycles through the various available compositions
		bool									mSwitch = false;							///< Set to true when a composition finishes playback
		float									mDurationScale = 1.0f;						///< Scales the length of a composition

		/**
		* Occurs when a composition finishes execution
		* This causes a new composition to be selected and watched
		* @param composition the composition that finished playback
		*/
		void compositionFinised(CompositionInstance& composition);
		NSLOT(mCompositionFinishedSlot, CompositionInstance&, compositionFinised)

	};
}
