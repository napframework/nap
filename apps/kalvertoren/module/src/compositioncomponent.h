#pragma once

#include "composition.h"
#include "compositioncontainer.h"

#include <component.h>
#include <nap/resourceptr.h>
#include <utility/datetimeutils.h>
#include <unordered_map>

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

		ResourcePtr<CompositionContainer>		mMonday = nullptr;							///< Compositions associated with monday
		ResourcePtr<CompositionContainer>		mTuesday = nullptr;							///< Compositions associated with tuesday
		ResourcePtr<CompositionContainer>		mWednesday = nullptr;						///< Compositions associated with wednesday
		ResourcePtr<CompositionContainer>		mThursday = nullptr;						///< Compositions associated with thursday
		ResourcePtr<CompositionContainer>		mFriday = nullptr;							///< Compositions associated with friday
		ResourcePtr<CompositionContainer>		mSaturday = nullptr;						///< Compositions associated with saturday
		ResourcePtr<CompositionContainer>		mSunday = nullptr;							///< Compositions associated with sunday
		int										mIndex = 0;									///< Property: The currently selected composition
		float									mDurationScale = 1.0f;						///< Property: Acts as a scale on the duration of the composition
		CompositionCycleMode					mCycleMode = CompositionCycleMode::Off;		///< Property: How the component cycles through all the available sequences
	};


	/**
	 * compositioncomponentInstance	
	 */
	class NAPAPI CompositionComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		enum class EMode : int
		{
			Automatic	= 0,			///< Active day is based on current date / time
			Manual		= 1				///< Active day is manually chosen
		};

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
		 * Sets the current day to use, enables manual mode
		 * @param day the new day to select compositions from
		 */
		void selectDay(nap::utility::EDay day);

		/**
		 *	@return the currently active day
		 */
		const utility::EDay getDay() const;

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

		/**
		 *	@return the total number of available compositions in the current selection
		 */
		int getCount() const;

		/**
		 *	Select a new composition based on the available compositions in the current set
		 */
		void select(int index);

		/**
		 * Turn automatic mode on / off
		 * @param automatic if the selection is acquired using the system date or manually selected
		 */
		void switchMode(EMode mode);

	private:
		int										mCurrentIndex = -1;							///< Currently selected composition @index
		CompositionContainer*					mCurrentContainer = nullptr;				///< Container associated with active day
		utility::EDay							mCurrentDay = utility::EDay::Monday;		///< Currently active day
		std::unique_ptr<CompositionInstance>	mCompositionInstance;						///< CompositionInstance created when switching compositions
		CompositionCycleMode					mCycleMode = CompositionCycleMode::Off;		///< How this component cycles through the various available compositions
		bool									mSwitch = false;							///< Set to true when a composition finishes playback
		float									mDurationScale = 1.0f;						///< Scales the length of a composition
		std::unordered_map<nap::utility::EDay, CompositionContainer*> mWeekMap;				///< Contains a composition container for every day in the week, populated on init
		EMode									mMode = EMode::Automatic;					///< When set to true, the day associated with the selection is derived from the current date / time
		utility::EDay							mDay = utility::EDay::Monday;				///< Currently selected day

		/**
		 *	@return the set of compositions associated with this week-day
		 */
		const CompositionContainer& getContainer(utility::EDay day ) const;

		/**
		 * @return the set of compositions associated with a specific day in the week
		 */
		CompositionContainer& getContainer(utility::EDay day);

		/**
		 * Select the composition at index in the associated container
		 * This becomes the active selection
		 * @param index the new composition to select
		 */
		void select(int index, CompositionContainer& container);

		/**
		* Occurs when a composition finishes execution
		* This causes a new composition to be selected and watched
		* @param composition the composition that finished playback
		*/
		void compositionFinised(CompositionInstance& composition);
		NSLOT(mCompositionFinishedSlot, CompositionInstance&, compositionFinised)

	};
}
