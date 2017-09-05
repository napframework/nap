#include "lasershapes.h"
#include <nap/entity.h>
#include <nap/core.h>
#include <iostream>
#include <mathutils.h>
#include <etherdreaminterface.h>

#ifndef M_PI
	#define M_PI 3.14159265358979323846
#endif

RTTI_BEGIN_CLASS(nap::LaserDotComponent)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::LaserDotComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////

RTTI_BEGIN_CLASS(nap::LaserSquareComponent)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::LaserSquareComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////

RTTI_BEGIN_CLASS(nap::LaserCircleComponent)
	RTTI_PROPERTY("Index", &nap::LaserCircleComponent::mIndex, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::LaserCircleComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////

/**
@brief Calculate color value
**/
static int16_t colorsin(float pos)
{
	int max_value = nap::etherMaxValue;
	int min_value = nap::etherMinValue;

	// Get color value
	int res = (sin(pos) + 1) * max_value;
	res = res > max_value ? max_value : res;
	res = res < min_value ? min_value : res;
	return res;
}

//////////////////////////////////////////////////////////////////////////

using namespace nap::math;

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Laser DOT
	//////////////////////////////////////////////////////////////////////////

	bool LaserDotComponentInstance::init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
	{
		// Call base class implementation
		return LaserShapeComponentInstance::init(entityCreationParams, errorState);
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

		// Calculate color value
		int16_t color_value = lerp<int16_t>(0, max_value, mShapeProperties.mBrightness);

		// Now fill buffer
		for (int i = 0; i < mShapeProperties.mNumberOfPoints; i++)
		{
			EtherDreamPoint* current_point = &(mPoints[i]);
			current_point->X = x_center;
			current_point->Y = y_center;
			current_point->R = color_value;
			current_point->G = color_value;
			current_point->B = color_value;
			current_point->I = color_value;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Laser Square
	//////////////////////////////////////////////////////////////////////////

	bool LaserSquareComponentInstance::init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
	{
		// Make sure the square has an equal amount of points
		LaserShapeComponent* laser_shape_resource = getComponent<LaserShapeComponent>();
		if (laser_shape_resource->mShapeProperties.mNumberOfPoints % 4 != 0)
		{
			return errorState.check(false, "Square laser points can not be divided by 4");
		}

		// Call base class implementation
		return LaserShapeComponentInstance::init(entityCreationParams, errorState);
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
		fillSquare(sy, sx, mShapeProperties.mBrightness, mShapeProperties.mSize);
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

	//////////////////////////////////////////////////////////////////////////
	// Laser Circle
	//////////////////////////////////////////////////////////////////////////

	bool LaserCircleComponentInstance::init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
	{
		LaserShapeComponentInstance::init(entityCreationParams, errorState);

		// Copy index
		LaserCircleComponent* circle_resource = getComponent<LaserCircleComponent>();
		mIndex = circle_resource->mIndex;

		return true;
	}

	void LaserCircleComponentInstance::update(double deltaTime)
	{
		LaserShapeComponentInstance::update(deltaTime);

		// Fill the circle
		FillCircle(mCurrentTime, mIndex);
	}

	void LaserCircleComponentInstance::FillCircle(float phase, int mode)
	{
		int i;
		int max_value = std::numeric_limits<int16_t>::max();

		for (i = 0; i < mShapeProperties.mNumberOfPoints; i++) 
		{
			struct EtherDreamPoint *pt = &mPoints[i];
			float ip = (float)i * 2.0 * M_PI / (float)(mShapeProperties.mNumberOfPoints);
			float ipf = fmod(ip + phase, 2.0 * M_PI);;

			switch (mode) {
			default:
			case 0: {
				float cmult = .05 * sin(30 * (ip - phase / 3));
				pt->X = sin(ip) * 20000 * (1 + cmult) * mShapeProperties.mSize;
				pt->Y = cos(ip) * 20000 * (1 + cmult) * mShapeProperties.mSize;
				break;
			}
			case 1: {
				float cmult = .10 * sin(10 * (ip - phase / 3));
				pt->X = sin(ip) * 20000 * (1 + cmult) * mShapeProperties.mSize;
				pt->Y = cos(ip) * 20000 * (1 + cmult) * mShapeProperties.mSize;
				break;
			}
			case 2: {
				ip *= 3;
				float R = 5;
				float r = 3;
				float D = 5;

				pt->X = (2500 * ((R - r)*cos(ip + phase) + D*cos((R - r)*ip / r))) * mShapeProperties.mSize;
				pt->Y = (2500 * ((R - r)*sin(ip + phase) - D*sin((R - r)*ip / r))) * mShapeProperties.mSize;
				break;
			}
			case 3: {
				int n = 5;
				float R = 5 * cos(M_PI / n) / cos(fmod(ip, (2 * M_PI / n)) - (M_PI / n));
				pt->X = 3500 * R*cos(ip + phase) * mShapeProperties.mSize;
				pt->Y = 3500 * R*sin(ip + phase) * mShapeProperties.mSize;
				break;
			}
			case 4: {
				float Xo = sin(ip);
				pt->X = 20000 * Xo * cos(phase / 4) * mShapeProperties.mSize;
				pt->Y = 20000 * Xo * -sin(phase / 4) * mShapeProperties.mSize;
				ipf = fmod(((Xo + 1) / 2.0) + phase / 3, 1.0) * 2 * M_PI;
			}
			}

			pt->R = lerp<int16_t>(0, colorsin(ipf), mShapeProperties.mBrightness);
			pt->G = lerp<int16_t>(0, colorsin(ipf + (2.0 * M_PI / 3.0)), mShapeProperties.mBrightness);
			pt->B = lerp<int16_t>(0, colorsin(ipf + (4.0 * M_PI / 3.0)), mShapeProperties.mBrightness);
			pt->I = mShapeProperties.mBrightness;
		}
	}
}