#pragma once

// External Includes
#include <nap/service.h>
#include <nap/attribute.h>

namespace nap
{
	/**
	 * Holds a reference to all drawable objects
	 */
	class RenderService : public Service
	{
		RTTI_ENABLE_DERIVED_FROM(Service)

	public:
		RenderService() = default;

		/**
		 * Call this in your app loop to emit a render call
		 * When called the draw bang is emitted
		 */
		void render();

		/**
		 * The draw signal that is emitted every render call
		 * Register to this event to receive draw calls
		 */
		SignalAttribute draw = { this, "draw" };

		protected:
		/**
		 * Type registration
		 */
		virtual void registerTypes(nap::Core& core) override;
	};
} // nap

RTTI_DECLARE(nap::RenderService)
