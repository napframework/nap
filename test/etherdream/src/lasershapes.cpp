#include "lasershapes.h"
#include <nap/entity.h>
#include <nap/core.h>
#include <iostream>
#include <mathutils.h>
#include <etherdreaminterface.h>

RTTI_BEGIN_CLASS(nap::LaserDotComponent)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::LaserDotComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////

RTTI_BEGIN_CLASS(nap::LaserSquareComponent)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::LaserSquareComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&)
RTTI_END_CLASS


using namespace nap::math;

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Laser DOT
	//////////////////////////////////////////////////////////////////////////

	bool LaserDotComponentInstance::init(const ObjectPtr<Component>& resource, EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
	{
		// Call base class implementation
		return LaserShapeComponentInstance::init(resource, entityCreationParams, errorState);
	}


	// Update point
	void LaserDotComponentInstance::update(double deltaTime)
	{
		LaserShapeComponentInstance::update(deltaTime);

		// Get x location
		float sx = (sin(mCurrentTime) + 1.0f) / 2.0f;
		sx = lerp<float>(0.25f, 0.75f, sx);

		// Get y location
		float sy = (cos(mCurrentTime) + 1.0f) / 2.0f;
		sy = lerp<float>(0.25f, 0.75f, sy);

		float loc_x = sx > 1.0f ? 1.0f : sx < 0.0f ? 0.0f : sx;
		float loc_y = sy > 1.0f ? 1.0f : sy < 0.0f ? 0.0f : sy;

		// min / max values
		int16_t min_value = nap::etherMinValue;
		int16_t max_value = nap::etherMaxValue;

		// Calculate location in units
		int x_center = static_cast<int>(lerp<int16_t>(min_value, max_value, loc_x));
		int y_center = static_cast<int>(lerp<int16_t>(min_value, max_value, loc_y));

		// Now fill buffer
		for (int i = 0; i < mShapeProperties.mNumberOfPoints; i++)
		{
			EtherDreamPoint* current_point = &(mPoints[i]);
			current_point->X = x_center;
			current_point->Y = y_center;
			current_point->R = max_value;
			current_point->G = max_value;
			current_point->B = max_value;
			current_point->I = max_value;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Laser Square
	//////////////////////////////////////////////////////////////////////////

	bool LaserSquareComponentInstance::init(const ObjectPtr<Component>& resource, EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
	{
		// Make sure the square has an equal amount of points
		LaserShapeComponent* laser_shape_resource = rtti_cast<LaserShapeComponent>(resource.get());
		if (laser_shape_resource->mShapeProperties.mNumberOfPoints % 4 != 0)
		{
			return errorState.check(false, "Square laser points can not be divided by 4");
		}

		// Call base class implementation
		return LaserShapeComponentInstance::init(resource, entityCreationParams, errorState);
	}


	void LaserSquareComponentInstance::update(double deltaTime)
	{
		LaserShapeComponentInstance::update(deltaTime);

		// Normalize and lerp
		float sx = (sin(mCurrentTime) + 1.0f) / 2.0f;
		sx = lerp<float>(0.5f, 0.7f, sx);

		float sy = (cos(mCurrentTime) + 1.0f) / 2.0f;
		sy = lerp<float>(0.4f, 0.6f, sy);

		// Fill the square
		fillSquare(sy, sx, 1.0f, 0.25f);
	}


	void LaserSquareComponentInstance::fillSquare(float inLocX, float inLocY, float inBrightNess, float inSize)
	{
		// Get side points and increment
		assert(mShapeProperties.mNumberOfPoints % 4 == 0);

		// Clamp location
		float loc_x = inLocX > 1.0f ? 1.0f : inLocX < 0.0f ? 0.0f : inLocX;
		float loc_y = inLocY > 1.0f ? 1.0f : inLocY < 0.0f ? 0.0f : inLocY;

		// min / max values
		int16_t min_value = nap::etherMinValue;
		int16_t max_value = nap::etherMaxValue;

		// Calculate location in units
		int x_center = static_cast<int>(lerp<int16_t>(min_value, max_value, inLocX));
		int y_center = static_cast<int>(lerp<int16_t>(min_value, max_value, inLocY));

		// Get corner points
		int global_max = ((int)max_value - (int)min_value) / 2;
		int offset = (int)((float)global_max * inSize);

		int x_min_int = x_center - offset;
		int x_max_int = x_center + offset;
		int y_min_int = y_center - offset;
		int y_max_int = y_center + offset;

		int16_t x_min = (int16_t)(x_min_int < (int)min_value ? (int)min_value : x_min_int);
		int16_t x_max = (int16_t)(x_max_int > (int)max_value ? (int)max_value : x_max_int);
		int16_t y_min = (int16_t)(y_min_int < (int)min_value ? (int)min_value : y_min_int);
		int16_t y_max = (int16_t)(y_max_int > (int)max_value ? (int)max_value : y_max_int);

		int16_t color_value = lerp<int16_t>(0, max_value, inBrightNess);
		
		int side_points = mShapeProperties.mNumberOfPoints / 4;
		float inc = 1.0f / (float)(side_points);

		int idx(0);
		int counter(0);
		for (int i = side_points * 0; i < side_points * 1; i++)		//< Fill up left > up right
		{
			EtherDreamPoint* current_point = &mPoints[idx];
			current_point->X = lerp<int16_t>(x_min, x_max, (float)counter * inc);
			current_point->Y = y_max;
			current_point->R = color_value;
			current_point->B = color_value;
			current_point->G = color_value;
			current_point->I = color_value;
			idx++;
			counter++;
		}

		counter = 0;
		for (int i = side_points * 1; i < side_points * 2; i++)		//< Top right to bottom right
		{
			EtherDreamPoint* current_point = &mPoints[idx];
			current_point->X = x_max;
			current_point->Y = lerp<int16_t>(y_max, y_min, (float)counter * inc);
			current_point->R = color_value;
			current_point->B = color_value;
			current_point->G = color_value;
			current_point->I = color_value;
			idx++;
			counter++;
		}

		counter = 0;
		for (int i = side_points * 2; i < side_points * 3; i++)		//< bottom right to bottom left
		{
			EtherDreamPoint* current_point = &mPoints[idx];
			current_point->X = lerp<int16_t>(x_max, x_min, (float)counter * inc);
			current_point->Y = y_min;
			current_point->R = color_value;
			current_point->B = color_value;
			current_point->G = color_value;
			current_point->I = color_value;
			idx++;
			counter++;
		}

		counter = 0;
		for (int i = side_points * 3; i < side_points * 4; i++)		//< bottom left to top left
		{
			EtherDreamPoint* current_point = &mPoints[idx];
			current_point->X = x_min;
			current_point->Y = lerp<int16_t>(y_min, y_max, (float)counter * inc);
			current_point->R = color_value;
			current_point->B = color_value;
			current_point->G = color_value;
			current_point->I = color_value;
			idx++;
			counter++;
		}
	}

}