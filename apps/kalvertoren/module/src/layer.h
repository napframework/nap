#pragma once

// External Includes
#include <rtti/rttiobject.h>
#include <image.h>
#include <texture2d.h>
#include <nap/ObjectPtr.h>

namespace nap
{
	/**
	 * Base class for instance of Layer. Is created through Layer::createInstance. Can be updated and returns 'current' texture.
	 */
	class NAPAPI LayerInstance
	{
	public:
		/**
		 * Updates the LayerInstance.
		 * @param deltaTime Time between frames.
		 */
		virtual void update(double deltaTime) {}

		/**
		 * @return the texture associated with this layer
		 * Every derived class should implement it's own method;
		 */
		virtual nap::BaseTexture2D& getTexture() = 0;

		/**
		 * @return the texture associated with this layer
		 * Every derived class should implement it's own method
		 */
		virtual const nap::BaseTexture2D& getTexture() const = 0;
	};


	/**
	 * Base layer
	 */
	class NAPAPI Layer : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)
	public:
		virtual ~Layer();

		/**
		 * Initialize this object after de-serialization
		 * @param errorState contains the error message when initialization fails
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return Instance object for this layer.
		 */
		virtual std::unique_ptr<LayerInstance> createInstance() = 0;
	};
}
