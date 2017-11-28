#pragma once
#include "artnetmeshfromfile.h"
#include "selectcolormethodcomponent.h"

#include <component.h>

namespace nap
{
	class ApplyColorComponentInstance;

	/**
	 *	colormeshcomponent
	 */
	class NAPAPI ApplyColorComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(ApplyColorComponent, ApplyColorComponentInstance)
	public:
		/**
		 *	The mesh it should color
		 */
		ObjectPtr<ArtnetMeshFromFile> mMesh = nullptr;
		ComponentPtr<SelectColorMethodComponent> mSelector = nullptr;
	};


	/**
	 * colormeshcomponentInstance	
	 */
	class NAPAPI ApplyColorComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		ApplyColorComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize colormeshcomponentInstance based on the colormeshcomponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the colormeshcomponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 *	Update this component, only when it's active
		 */
		virtual void update(double deltaTime) override final;

		/**
		 *	@return if this color component is active
		 */
		bool isActive() const											{ return mActive; }

		/**
		 * @return the mesh to color
		 */
		ArtnetMeshFromFile& getMesh()									{ return *mMesh; }

		/**
		 *	Enables this coloring component
		 */
		void enable()													{ mActive = true; }

		/**
		 *	Disables this coloring component
		 */
		void disable()													{ mActive = false; }

		/**
		 *	The selector to register this object with
		 */
		ComponentInstancePtr<SelectColorMethodComponent> mSelector =	{ this, &ApplyColorComponent::mSelector };

	protected:
		/**
		 *	Called when enabled, implement color method here
		 */
		virtual void applyColor(double deltaTime) = 0;

	private:
		bool mActive = false;										///< If the component should color stuff
		ArtnetMeshFromFile* mMesh = nullptr;						///< The mesh to color
	};
}
