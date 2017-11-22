#pragma once

// External Includes
#include <component.h>
#include <unordered_map>
#include <entity.h>
#include <perspcameracomponent.h>
#include <renderservice.h>
#include <renderwindow.h>
#include <rendertarget.h>

namespace nap
{
	class LaserControlInstanceComponent;

	/**
	 * Holds laser specific configuration
	 */
	class LaserConfiguration
	{
	public:
		// property: Render Target
		nap::ObjectPtr<RenderTarget> mTarget = nullptr;

		// property: Laser id
		int mLaserID = 0;
	};


	/**
	 *	lasercontroller
	 */
	class LaserControlComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(LaserControlComponent, LaserControlInstanceComponent)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		// property: holds all the laser configurations, creates a laser instance for every compound
		std::vector<LaserConfiguration> mLaserConfigurations;
	};


	/**
	 * lasercontrollerInstance	
	 */
	class LaserControlInstanceComponent : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		LaserControlInstanceComponent(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize lasercontrollerInstance based on the lasercontroller resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the lasercontrollerInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 *	@return the laser entity associated with @id, nullptr if not found
		 */
		nap::EntityInstance* getLaserEntity(int id);

		/**
		 *	Renders all available lasers to their respective buffers
		 */
		void renderToLaserBuffers(nap::PerspCameraComponentInstance& camera, RenderService& renderer);

		/**
		 *	Render the buffers on to their planes
		 */
		void renderFrames(nap::RenderWindow& window, nap::PerspCameraComponentInstance& camera, RenderService& renderer);

	private:
		// All the laser compounds associated with this controller
		std::vector<LaserConfiguration> mLaserConfigurations;

		// All the instantiated laser prototypes
		std::unordered_map<int, nap::EntityInstance*> mLaserEntityMap;

		// All the laser compounds
		std::unordered_map<int, nap::LaserConfiguration> mLaserConfigurationMap;

		// All the laser frames
		std::unordered_map<int, nap::EntityInstance*> mLaserFrameMap;
	};
}
