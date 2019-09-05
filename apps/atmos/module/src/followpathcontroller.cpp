#include "followpathcontroller.h"

// External Includes
#include <entity.h>
#include <transformcomponent.h>
#include <inputcomponent.h>
#include <lineutils.h>
#include <nap/logger.h>
#include <glm/gtc/matrix_transform.hpp>

// nap::pathcontroller run time class definition 
RTTI_BEGIN_CLASS(nap::FollowPathController)
	RTTI_PROPERTY("Loop",				&nap::FollowPathController::mLoop,					nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Move",				&nap::FollowPathController::mMove,					nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("CalculateUpVector",	&nap::FollowPathController::mCalculateUpVector,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Path",				&nap::FollowPathController::mPath,					nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("PathTransform",		&nap::FollowPathController::mPathTransform,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Position",			&nap::FollowPathController::mPosition,				nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("MovementSpeed",		&nap::FollowPathController::mMovementSpeed,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Rotation",			&nap::FollowPathController::mRotation,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Offset",				&nap::FollowPathController::mOffset,					nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("UpVector",			&nap::FollowPathController::mUpVector,				nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::pathcontrollerInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::FollowPathControllerInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void FollowPathController::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(TransformComponent));
	}


	bool FollowPathControllerInstance::init(utility::ErrorState& errorState)
	{
		// TransformComponent is required to move the entity
		mCameraTransform = getEntityInstance()->findComponent<TransformComponentInstance>();
		if (!errorState.check(mCameraTransform != nullptr, "%s: missing transform component", mID.c_str()))
			return false;

		// Copy over properties
		mSpeed = getComponent<FollowPathController>()->mMovementSpeed;
		mPosition = getComponent<FollowPathController>()->mPosition;
		mLoop = getComponent<FollowPathController>()->mLoop;
		mRotation = getComponent<FollowPathController>()->mRotation;
		mOffset = getComponent<FollowPathController>()->mOffset;
		mMove = getComponent<FollowPathController>()->mMove;
		mCalcUpVector = getComponent<FollowPathController>()->mCalculateUpVector;
		mUpVector = getComponent<FollowPathController>()->mUpVector;

		// Set path to use
		setPath(*(getComponent<FollowPathController>()->mPath.get()));

		return true;
	}


	void FollowPathControllerInstance::update(double deltaTime)
	{
		// Don't do anything when not active
		if (!mEnabled)
			return;

		// Calculate new position if auto move is turned on
		if (mMove)
		{
			mPosition += (deltaTime * mSpeed);
			mPosition = mLoop ? fmod(mPosition, 1.0f) : math::clamp<float>(mPosition, 0.0f, 1.0f);
		}

		// Get vertex interpolation value
		int vert_min, vert_max;
		float lerp_v = math::getVertexLerpValue(mPathDistanceMap, mPosition, vert_min, vert_max);

		// Get the values to interpolate
		glm::vec3 lower_pos_value = mPath->getPositionAttr()[vert_min];
		glm::vec3 upper_pos_value = mPath->getPositionAttr()[vert_max];

		// Get current line transform
		const glm::mat4& line_xform = mLineTransform->getGlobalTransform();

		// Get line vector transpose matrix, allows for converting normal from object to world space
		glm::mat3 trans_matrix = glm::transpose(glm::inverse(glm::mat3(line_xform)));

		// Get actual position on line in world space
		glm::vec3 new_pos = math::lerp<glm::vec3>(lower_pos_value, upper_pos_value, lerp_v);
		new_pos = glm::translate(line_xform, new_pos)[3];
		new_pos += mOffset;
		mCameraTransform->setTranslate(new_pos);

		// Get line normal at sample location
		glm::vec3 lower_norm_value = mPath->getNormalAttr()[vert_min];
		glm::vec3 upper_norm_value = mPath->getNormalAttr()[vert_max];
		glm::vec3 line_normal = math::lerp<glm::vec3>(lower_norm_value, upper_norm_value, lerp_v);
		line_normal = glm::normalize(trans_matrix * (line_normal * -1.0f));

		glm::vec3 up_vec = mUpVector;
		if (mCalcUpVector)
		{
			// Get direction normal based on last and first vertex position values
			// This is used to auto calculate an up vector for the camera view matrix
			glm::vec3 dir_vector = upper_pos_value - lower_pos_value;
			assert(!glm::isnan(dir_vector));
			dir_vector = glm::normalize(trans_matrix * dir_vector);
			up_vec = glm::cross(dir_vector, line_normal);
		}

		// Get tangent
		glm::vec3 tangent = glm::cross(line_normal, up_vec);

		// Apply rotation to tangent based on given rotation values
		glm::mat4 rotationMat(1); // Creates a identity matrix
		rotationMat = glm::rotate(rotationMat, glm::radians(mRotation.x), line_normal);
		rotationMat = glm::rotate(rotationMat, glm::radians(mRotation.y), up_vec);
		rotationMat = glm::rotate(rotationMat, glm::radians(mRotation.z), tangent);
		tangent = rotationMat * glm::vec4(tangent,0.0f);

		// Compute rotation value using glm look-at 
		glm::mat4 rotation_mat = glm::inverse(glm::lookAt(new_pos, new_pos + tangent, up_vec));
		glm::quat rotation = glm::quat_cast(rotation_mat);
		
		/* WHEN APPLYING ROTATATION VALUE AFTERWARDS EFFECTS ARE COOL)
		rotation = glm::rotate(rotation, math::radians(mRotation.x), line_normal);
		rotation = glm::rotate(rotation, math::radians(mRotation.y), up_vec);
		rotation = glm::rotate(rotation, math::radians(mRotation.z), tangent);
		*/

		mCameraTransform->setRotate(rotation);
	}


	void FollowPathControllerInstance::enable()
	{
		mEnabled = true;
	}


	void FollowPathControllerInstance::enable(float position)
	{
		mEnabled = true;
		setPosition(position);
	}


	void FollowPathControllerInstance::disable()
	{
		mEnabled = false;
	}


	void FollowPathControllerInstance::setPath(const nap::PolyLine& line)
	{
		mPath = &line;
		mPath->getDistances(mPathDistanceMap);
	}


	const nap::PolyLine& FollowPathControllerInstance::getPath() const
	{
		return *mPath;
	}


	void FollowPathControllerInstance::setPosition(float position)
	{
		mPosition = math::clamp<float>(position, 0.0f, 1.0f);
	}


	float FollowPathControllerInstance::getPosition() const
	{
		return mPosition;
	}


	void FollowPathControllerInstance::setSpeed(float speed)
	{
		mSpeed = speed;
	}


	float FollowPathControllerInstance::getSpeed() const
	{
		return mSpeed;
	}


	void FollowPathControllerInstance::setOffset(const glm::vec3& offset)
	{
		mOffset = offset;
	}


	glm::vec3 FollowPathControllerInstance::getOffset() const
	{
		return mOffset;
	}


	void FollowPathControllerInstance::setRotation(const glm::vec3& rotation)
	{
		mRotation = rotation;
	}


	glm::vec3 FollowPathControllerInstance::getRotation() const
	{
		return mRotation;
	}


	void FollowPathControllerInstance::loop(bool enable)
	{
		mLoop = enable;
	}

	
	bool FollowPathControllerInstance::isLooping() const
	{
		return mLoop;
	}
}