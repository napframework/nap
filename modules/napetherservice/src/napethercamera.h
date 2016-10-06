#pragma once

// NAP Includes
#include <nap/serviceablecomponent.h>
#include <nap/attribute.h>
#include <nap/link.h>

namespace nap
{
	/**
	@brief etherdream camera

	Processes and renders all etherdream spline instances
	**/
	class EtherDreamCamera : public ServiceableComponent
	{
		RTTI_ENABLE_DERIVED_FROM(Component)

	public:
		// Default constructor
		EtherDreamCamera()				{ }

		// Frustrum -> used for finding projection bounds in 2d space
		NumericAttribute<float> mFrustrumWidth		{ this, "FrustrumWidth", 500.0f, 100.0f, 1000.0f };
		NumericAttribute<float> mFrustrumHeight		{ this, "FrustrumHeight", 500.0f, 100.0f, 1000.0f };
		Attribute<bool>			mTraceMode			{ this, "TraceMode", false };

		// Utility
		void setFrustrumWidth(float inValue)  { mFrustrumWidth.setValue(inValue); }
		void setFrustrumHeight(float inValue) { mFrustrumHeight.setValue(inValue); }

		// Link to entity to render
		Link mRenderEntity;
	};
}

RTTI_DECLARE(nap::EtherDreamCamera)
