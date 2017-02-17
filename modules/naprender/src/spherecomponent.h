#pragma once

#include "meshcomponent.h"

namespace nap
{
	/**
	 * Highly efficient sphere that holds a link to a generic
	 * sphere mesh. Use this sphere for quick drawing but note
	 * that this component shares it's vertex data with other plane
	 * components.
	 */
	class SphereComponent : public MeshComponent
	{
		RTTI_ENABLE_DERIVED_FROM(MeshComponent)
	public:
		// Default constructor
		SphereComponent() = default;

		/**
		 * @return the sphere mesh
		 * Note that this mesh is shared between every sphere component instance
		 */
		virtual opengl::Mesh* getMesh() const override;
	};
}

RTTI_DECLARE(nap::SphereComponent)