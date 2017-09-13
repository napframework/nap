#pragma once

#include "lasershapecomponent.h"

namespace nap
{
	class LaserDotComponentInstance;

	//////////////////////////////////////////////////////////////////////////
	// Laser DOT
	//////////////////////////////////////////////////////////////////////////

	class LaserDotComponent : public LaserShapeComponent
	{
		RTTI_ENABLE(LaserShapeComponent)
		DECLARE_COMPONENT(LaserDotComponent, LaserDotComponentInstance)
	};

	class LaserDotComponentInstance : public LaserShapeComponentInstance
	{
		RTTI_ENABLE(LaserShapeComponentInstance)
	public:
		// Constructor
		LaserDotComponentInstance(EntityInstance& entity, Component& resource) : LaserShapeComponentInstance(entity, resource) 
		{ }

		// Init
		virtual bool init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState) override;

		// Update
		virtual void update(double deltaTime) override;
	};


	//////////////////////////////////////////////////////////////////////////
	// Laser Square
	//////////////////////////////////////////////////////////////////////////

	class LaserSquareComponentInstance;

	class LaserSquareComponent : public LaserShapeComponent
	{
		RTTI_ENABLE(LaserShapeComponent)
		DECLARE_COMPONENT(LaserSquareComponent, LaserSquareComponentInstance)
	};

	class LaserSquareComponentInstance : public LaserShapeComponentInstance
	{
		RTTI_ENABLE(LaserShapeComponentInstance)
	public:
		// Constructor
		LaserSquareComponentInstance(EntityInstance& entity, Component& resource) : LaserShapeComponentInstance(entity, resource)
		{ }

		// Init
		virtual bool init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState) override;

		// Update
		virtual void update(double deltaTime) override;

	private:
		void fillSquare(float inLocX, float inLocY, float inBrightNess, float inSize);
	};


	//////////////////////////////////////////////////////////////////////////
	// Laser Circles
	//////////////////////////////////////////////////////////////////////////

	class LaserCircleComponentInstance;

	class LaserCircleComponent : public LaserShapeComponent
	{
		RTTI_ENABLE(LaserShapeComponent)
		DECLARE_COMPONENT(LaserCircleComponent, LaserCircleComponentInstance)
	public:
		// Current circle index (property)
		int mIndex = 0;
	};

	class LaserCircleComponentInstance : public LaserShapeComponentInstance
	{
		RTTI_ENABLE(LaserShapeComponentInstance)
	public:
		// Constructor
		LaserCircleComponentInstance(EntityInstance& entity, Component& resource) : LaserShapeComponentInstance(entity, resource)
		{ }

		// Init
		virtual bool init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState) override;

		// Update
		virtual void update(double deltaTime) override;

		// Current index
		int mIndex = 0;

	private:
		void FillCircle(float phase, int mode);
	};
}
