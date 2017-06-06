#pragma once

#include "meshresource.h"

namespace nap
{
	/**
	 * Predefined sphere mesh
	 */
	class SphereMeshResource : public MeshResource
	{
		RTTI_ENABLE(MeshResource)

	public:
		virtual bool init(utility::ErrorState& errorState) override;

	public:
		float mRadius = 1.0f;
		float mRings = 50.0f;
		float mSectors = 50.0f;
	};
}
