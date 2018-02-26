#pragma once

// External Includes
#include <glm/glm.hpp>

namespace opengl
{
	/**
	* Flags used to choose what part of render target to clear
	* Can be used as a bitmask, ie: EClearFlags::Color | EClearFlags::Depth
	*/
	enum class EClearFlags : uint8_t
	{
		Color	= 1,					///< Clears color buffer
		Depth	= 2,					///< Clears depth buffer
		Stencil = 4						///< Clears stencil buffer
	};

	inline EClearFlags operator&(EClearFlags a, EClearFlags b)
	{
		return static_cast<EClearFlags>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
	}
	inline EClearFlags operator|(EClearFlags a, EClearFlags b)
	{
		return static_cast<EClearFlags>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
	}


	/**
	 * Base class for TextureRenderTargets and BackBufferRenderTargets.
	 */
	class RenderTarget
	{
	public:
		// Constructor
		RenderTarget() = default;

		// Destructor
		virtual ~RenderTarget() = default;

		// Don't support copy
		RenderTarget(const RenderTarget&) = delete;
		RenderTarget& operator=(const RenderTarget&) = delete;

		/**
		* Binds the render target so it can be used by subsequent render calls
		*/
		virtual bool bind() = 0;

		/**
		* Unbinds the render target. TODO: decide how to proceeed with unbinding render targets.
		*/
		virtual bool unbind() = 0;

		/**
		* Clears color, depth and stencil depending on flags. Uses ClearColor.
		*/
		virtual void clear(EClearFlags flags);

		/**
		 * Returns size of render target
		 */
		virtual const glm::ivec2 getSize() const = 0;

		/**
		* Sets the clear color to be used by clear.
		*/
		void setClearColor(const glm::vec4& color)		{ mClearColor = color; }
		const glm::vec4& getClearColor() const			{ return mClearColor; }

	private:
		glm::vec4 mClearColor;			// Clear color, used for clearing the color buffer
	};

} // opengl
