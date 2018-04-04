#pragma once

#include "applycolorcomponent.h"
#include "compositioncomponent.h"
#include "colorpalettecomponent.h"
#include "rendercompositioncomponent.h"
#include "videocontrolcomponent.h"

#include <component.h>
#include <componentptr.h>
#include <triangleiterator.h>

namespace nap
{
	class ApplyVideoComponentInstance;

	/**
	 *	Applies a video to the mesh
	 */
	class NAPAPI ApplyVideoComponent : public ApplyColorComponent
	{
		RTTI_ENABLE(ApplyColorComponent)
		DECLARE_COMPONENT(ApplyVideoComponent, ApplyVideoComponentInstance)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		ComponentPtr<VideoControlComponent>	mVideoController;			///< property: link to the composition component
	};


	/**
	 * Applies a video to a mesh
	 */
	class NAPAPI ApplyVideoComponentInstance : public ApplyColorComponentInstance
	{
		RTTI_ENABLE(ApplyColorComponentInstance)
	public:
		ApplyVideoComponentInstance(EntityInstance& entity, Component& resource) :
			ApplyColorComponentInstance(entity, resource)										{ }

		/**
		 * Initialize ApplyVideoComponentInstance based on the ApplyColorComponentInstance resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the applycompositioncomponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Applies bounding box colors to the mesh
		 */
		virtual void applyColor(double deltaTime) override;

		// Resolved pointer to the video controller
		ComponentInstancePtr<VideoControlComponent> mVideoController = { this, &ApplyVideoComponent::mVideoController };
	};
}
