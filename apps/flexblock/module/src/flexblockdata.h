#pragma once

// External Includes
#include <nap/resource.h>
#include <nap/resourceptr.h>
#include <glm/glm.hpp>
#include <stddef.h>
#include <string>
#include <vector>
#include <memory>

namespace nap
{
	/**
	* FlexBlockShapeSizeValues
	* Contains information about the size of this shape and its frame
	*/
	class NAPAPI FlexBlockShapeSizeValues : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		virtual ~FlexBlockShapeSizeValues();

		virtual bool init(utility::ErrorState& errorState) override;

		glm::vec3 mObject;	///< Property: 'Object' size of the object shape
		glm::vec3 mFrame;	///< Property: 'Frame' size of the frame shape
	};

	/**
	* FlexBlockShapeSize
	* A description of the size of the shape ( will be multiplied with FlexBlockShapePoints in flex algorithm )
	*/
	class NAPAPI FlexBlockShapeSize : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		virtual ~FlexBlockShapeSize();

		virtual bool init(utility::ErrorState& errorState) override;

		std::string								mName;		///< Property: 'Name' name of the shape size
		ResourcePtr<FlexBlockShapeSizeValues>	mValues;	///< Property: 'Values' reference to shape size values
	};

	/**
	* FlexBlockShapePoints
	* A description of how the shape is constructed
	*/
	class NAPAPI FlexBlockShapePoints : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		virtual ~FlexBlockShapePoints();

		virtual bool init(utility::ErrorState& errorState) override;

		std::vector<glm::vec3> mObject; ///< Property: 'Object' points of object, togethers with shape size values will determine the size of the block
		std::vector<glm::vec3> mFrame;	///< Property: 'Frame' points of frame, togethers with shape size values will determine the size of the frame
	};

	/**
	* FlexBlockElements
	* Contains lists of indices, indicating how corners of the frame, object are connected with themselves and eachother
	*/
	class NAPAPI FlexBlockElements : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		virtual ~FlexBlockElements();

		virtual bool init(utility::ErrorState& errorState) override;

		std::vector<std::vector<int>> mObject;			///< Property: 'Object Element Connections' connections of motor points to sides
		std::vector<std::vector<int>> mObject2Frame;	///< Property: 'Object Element Connections With Frame' connections of frame points to object sides
		std::vector<std::vector<int>> mFrame;			///< Property: 'Frame Element Connections' connections of frame points to object sides
	};

	/**
	 * FlexBlockShape
	 * Contains the information needed to create and render a flexblock shape and execute the flex algorithm
	 */
	class NAPAPI FlexBlockShape : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		virtual ~FlexBlockShape();

		virtual bool init(utility::ErrorState& errorState) override;

		std::string										mName;			///< Property: 'Name' name of shape
		int												mMotorCount;	///< Property: 'MotorCount' amount of motors
		ResourcePtr<FlexBlockShapeSize>					mSize;			///< Property: 'Size' Definition of this shape
		ResourcePtr<FlexBlockElements>					mElements;		///< Property: 'FlexBlockElements' FlexBlockElements
		ResourcePtr<FlexBlockShapePoints>				mPoints;		///< Property: 'Points' FlexBlockPoints
	};
}
