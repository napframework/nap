#pragma once

// internal includes
#include "framemesh.h"
#include "flexblockmesh.h"
#include "flexblockdata.h"
#include "visualizenormalsmesh.h"
#include "flexdevice.h"

// external includes
#include <component.h>
#include <boxmesh.h>
#include <renderablemeshcomponent.h>
#include <renderable2dtextcomponent.h>
#include <componentptr.h>
#include <perspcameracomponent.h>
#include <maccontroller.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	class FlexBlockComponentInstance;

	/**
	 * FlexBlockComponent controls a flexblock mesh, frame mesh and provides input to the flexblock algorithm.
	 * FlexBlockComponent needs a flexblock shape definition. See FlexBlockShape and flexblockdata.h
	 */
	class NAPAPI FlexBlockComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(FlexBlockComponent, FlexBlockComponentInstance)
	public:
		
		ResourcePtr<FrameMesh>			mFrameMesh;			///< Property: 'FrameMesh' Reference to the frame mesh
		ResourcePtr<FlexBlockMesh>		mFlexBlockMesh;		///< Property: 'FlexBlockMesh' Reference to the FlexBlockMesh 
		ResourcePtr<FlexBlockShape>		mFlexBlockShape;	///< Property: 'FlexBlockShape' Reference to the shape definition of the block 
		nap::ResourcePtr<FlexDevice>	mFlexBlockDevice;	///< Property: 'FlexBlockDevice' Reference to the flexblock device 

	};

	//////////////////////////////////////////////////////////////////////////

	/**
	 * Runtime version of flexblock component
	 */
	class NAPAPI FlexBlockComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		FlexBlockComponentInstance(EntityInstance& entity, Component& resource) : ComponentInstance(entity, resource) { }
		
		virtual ~FlexBlockComponentInstance();

		/**
		 * Initialize FlexBlockComponentInstance based on the FlexBlockComponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the FlexBlockComponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update FlexBlockComponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

		/**
		* Update motor speed [0..8]
		* @param index index of motor [0..8]
		* @param value motor input 0-1
		*/
		void setInput(int index, float value);

		/**
		 * Set slack parameter
		 * @param value slack value, typically between -0.5 and 0.5
		 */
		void setSlack(float value);

		/**
		 * @return the current slack value, including scale and offset
		 */
		float getSlack() const;

		/**
		 * Set override
		 * @param override range index, motor index will be in flexblock space
		 * @param value between 0..1 , will be multiplied by override range 
		 */
		void setOverride(int index, float value);

		/**
		 * Sets frequency of override
		 * @param value between 0..1 , will be multiplied with sinus frequency
		 */
		void setSinusFrequency(float value);

		/**
		 * Sets sinus amplitude
		 * @param value between 0..1 , will be multiplied with amplitude
		 */
		void setSinusAmplitude(float value);

		void getRopeLengths(std::vector<float>& output);

		/**
		 * @return returns cube mesh points in local space
		 */
		const std::vector<glm::vec3>& getObjectPoints() const		{ return mObjectPoints; }

		/**
		 * @return returns frame mesh points in local space
		 */
		const std::vector<glm::vec3>& getFramePoints() const		{ return mFramePoints; }


		/**
		 * @param index of motor in flexblock space
		 * @return returns motor override
		 */
		float getMotorOverride(const int index) const;

		/**
		 * @return returns motor override minimum
		 */
		float getMotorOverrideMinimum() const						{ return mFlexblockDevice->mOverrideMin; }

		/**
		 * @return returns motor override range
		 */
		float getMotorOverrideRange() const							{ return mFlexblockDevice->mOverrideScale; }

		/**
		 * @return returns motor amplitude range
		 */
		float getSinusAmplitudeRange() const						{ return mFlexblockDevice->mAmplitudeRange; }

		/**
		 * @return returns motor frequency range
		 */
		float getSinusFrequencyRange() const						{ return mFlexblockDevice->mFrequencyRange; }

		float getSlackMinimum() const { return mFlexblockDevice->mSlackMin; }

		float getSlackRange() const {
			return mFlexblockDevice->mSlackScale;
		}

	protected:

		// Resources / Devices
		FrameMesh*		mFrameMesh		 = nullptr;
		FlexBlockMesh*	mFlexBlockMesh	 = nullptr;
		FlexDevice*		mFlexblockDevice = nullptr;

		// Meshes
		std::vector<glm::vec3> mObjectPoints = std::vector<glm::vec3>(8);
		std::vector<glm::vec3> mFramePoints = std::vector<glm::vec3>(8);
	};
}
