#pragma once

// Local Includes
#include "meshcomponent.h"

namespace nap
{
	/**
	 * Highly efficient plane that holds a link to generic
	 * plane mesh. Use this plane for quick drawing but
	 * note that this component shares it's vertex data with 
	 * other plane components. TODO: It's better to introduce
	 * a mesh resource and let the mesh component link to it
	 */
	class PlaneComponent : public MeshComponent
	{
		RTTI_ENABLE_DERIVED_FROM(MeshComponent)
	public:
		// Default constructor
		PlaneComponent() = default;

		/**
		 * @return the plane mesh
		 * Note that this mesh is shared between every instance
		 * of the plane component
		 */
		virtual opengl::Mesh* getMesh() const override;  
	};
}
