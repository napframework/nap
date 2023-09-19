#pragma once

namespace nap
{
	// Forward declares
	class RenderService;

	/**
	 * RenderCommand base class
	 */
	class NAPAPI RenderCommand
	{
		RTTI_ENABLE()
		friend class RenderService;
	protected:
		virtual void record(RenderService& renderService) const = 0;
	};

	/**
	 * HeadlessCommand
	 * Implement this class to create a custom headless rendering command recording operation that can be queued by the render service
	 */
	class NAPAPI HeadlessCommand : public RenderCommand
	{
		RTTI_ENABLE(RenderCommand)
		friend class RenderService;
	protected:
		virtual void record(RenderService& renderService) const override = 0;
	};

	/**
	 * ComputeCommand
	 * Implement this class to create a custom headless rendering command recording operation that can be queued by the render service
	 */
	class NAPAPI ComputeCommand : public RenderCommand
	{
		RTTI_ENABLE(RenderCommand)
		friend class RenderService;
	protected:
		virtual void record(RenderService& renderService) const override = 0;
	};
}
