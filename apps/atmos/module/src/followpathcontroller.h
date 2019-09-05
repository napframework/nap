#pragma once

#include <component.h>
#include <componentptr.h>
#include <polyline.h>
#include <inputevent.h>
#include <transformcomponent.h>
#include <nap/resourceptr.h>
#include <map>

namespace nap
{
	class FollowPathControllerInstance;

	/**
	 * path controller
	 */
	class NAPAPI FollowPathController : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(FollowPathController, FollowPathControllerInstance)
	public:

		/**
		 * Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		 * @param components the components this object depends on
		 */
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		ComponentPtr<TransformComponent> mPathTransform;			///< Transform associated with the path
		ResourcePtr<PolyLine> mPath;								///< Path to place the camera on
		bool		mLoop = false;									///< If the camera loops around the path
		bool		mMove = false;									///< If the camera moves automatically (with the given speed) over the path
		bool		mCalculateUpVector = true;						///< When set to true, the up vector is automatically calculated based on the line slope and normal. 
		float		mPosition;										///< Position of the camera (0 - 1) along the path
		float		mMovementSpeed = 0.1;							///< Speed (in seconds) at which the camera is allowed to move
		glm::vec3	mRotation = { 0.0f, 0.0f, 0.0f };				///< Rotation away from line tangent
		glm::vec3	mOffset = { 0.0f, 0.0f, 0.0f };					///< Transformation offset from line
		glm::vec3	mUpVector = { 0.0f, 1.0f, 0.0f };				///< up-vector used when 'mCalculateUpVector' is turned off
	};


	/**
	 * path controller Instance	
	 */
	class NAPAPI FollowPathControllerInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		FollowPathControllerInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize pathcontrollerInstance based on the path controller resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the pathcontrollerInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update pathcontrollerInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

		/**
		 * Enable the component while setting the transform.
		 * @param translate Camera translation to set.
		 * @param rotate Camera rotation to set.
		 */
		void enable(float position);

		/**
		* Enable the component, keeping the current transform.
		*/
		void enable();

		/**
		 * Disables this camera controller
		 */
		void disable();

		/**
		 * Sets the path the camera follows
		 */
		void setPath(const nap::PolyLine& line);

		/**
		 * @return the path the camera currently follows
		 */
		const nap::PolyLine& getPath() const;

		/**
		 * Sets the position of the camera on the line.
		 */
		void setPosition(float position);

		/**
		 * @return the current position of the camera on the path
		 */
		float getPosition() const;

		/**
		 * Sets the camera movement speed when 'move' is enabled
		 */
		void setSpeed(float speed);

		/**
		 * @return current camera movement speed along path
		 */
		float getSpeed() const;

		/**
		 * Sets the camera offset from the path
		 */
		void setOffset(const glm::vec3& offset);

		/**
		 * @return current camera offset from path
		 */
		glm::vec3 getOffset() const;

		/**
		 * Sets the camera rotation from the path in degrees
		 */
		void setRotation(const glm::vec3& rotation);

		/**
		 * @return current camera rotation from path in degrees
		 */
		glm::vec3 getRotation() const;

		/**
		 * Set if the camera loops over the path or not
		 */
		void loop(bool enable);

		/**
		 * @return if the camera loops over the path
		 */
		bool isLooping() const;

	private:
		ComponentInstancePtr<TransformComponent>mLineTransform = initComponentInstancePtr(this, &FollowPathController::mPathTransform);
		TransformComponentInstance*				mCameraTransform = nullptr;			///< Transform associated with camera
		bool									mEnabled = true;					///< If this controller updates the camera transform
		float									mPosition = 0.0f;					///< Position of the camera along the line (0-1)
		float									mSpeed = 0.0f;						///< Movement speed of the camera
		std::map<float, int>					mPathDistanceMap;					///< Holds distance from point to point of current line
		const PolyLine*							mPath = nullptr;					///< Current path
		glm::vec3								mOffset = { 0.0f, 0.0f, 0.0f };		///< Object offset from line
		glm::vec3								mRotation = { 0.0f, 0.0f, 0.0f };	///< Rotation offset
		bool									mLoop = false;						///< If the camera loops automatically
		bool									mMove = false;						///< If the camera moves automatically
		bool									mCalcUpVector = true;				///< If the up vector is automatically calculated
		glm::vec3								mUpVector = { 0.0f, 1.0f, 0.0f };	///< Up vector used when calculate up vector is turned off
	};
}
