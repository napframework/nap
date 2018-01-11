#pragma once

//#include "colormeshcomponent.h"

#include <component.h>
#include <componentptr.h>

namespace nap
{
	class SelectColorMethodComponentInstance;
	class ApplyColorComponentInstance;

	/**
	 *	selectcolormethodcomponent
	 */
	class NAPAPI SelectColorMethodComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(SelectColorMethodComponent, SelectColorMethodComponentInstance)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;
	};


	/**
	 * selectcolormethodcomponentInstance	
	 */
	class NAPAPI SelectColorMethodComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		SelectColorMethodComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize selectcolormethodcomponentInstance based on the selectcolormethodcomponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the selectcolormethodcomponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Registers a mesh color component
		 */
		void registerMeshColorComponent(ApplyColorComponentInstance& colorer);

		/**
		 * @return the total number of available color methods
		 */
		int getSelectionCount() const											{ return mPaintComponents.size(); }

		/**
		 * Change the current coloring method
		 * @param index of the new color method
		 */
		void select(int index);

		/**
		* Selects the current mesh based on it's associated rtti type
		* Seletion won't change if the type doesn't exist
		* @param type the type of painter to select
		*/
		void select(nap::rtti::TypeInfo type);

		/**
		 * Pushes selection state to registered color components
		 */
		virtual void update(double deltaTime) override;

	private:
		std::vector<ApplyColorComponentInstance*> mPaintComponents;
		int mCurrentSelection = 0;
	};
}
