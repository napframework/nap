#pragma once

// External Includes
#include <shader.h>

namespace nap
{
	// Forward declares
	Core;

	/**
	 * Video shader
	 */
	class NAPAPI VideoShader : public Shader
	{
		RTTI_ENABLE(Shader)
	public:
		VideoShader(Core& core);

		/**
		 * Initialize this object after de-serialization
		 * @param errorState contains the error message when initialization fails
		 */
		virtual bool init(utility::ErrorState& errorState) override;
	};
}
