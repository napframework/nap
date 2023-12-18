#pragma once

namespace nap
{
	// Forward declares
	class RenderService;

	/**
	 * RenderCommand base class
	 * 
	 * This class can be used to queue render commands with priority in the subsequent frame. You can create a custom
	 * class that inherits from either `nap::HeadlessCommand` or `nap::ComputeCommand` depending on your requirements,
	 * and implement the `record` method with the vulkan commands you wish to execute with priority.
	 *
	 * ~~~~~{.h}
	 *	class PreRenderExampleCommand : public HeadlessCommand
	 *	{
	 *		RTTI_ENABLE(HeadlessCommand)
	 *	public:
	 *		PreRenderExampleCommand(RenderService& renderService) : HeadlessCommand() {}
	 *	protected:
	 *		virtual void record(RenderService& renderService) const override { ... }
	 *	};
	 * 
	 *	std::unique_ptr<PreRenderCubeMapsCommand> mPreRenderCubeMapsCommand;
	 * ~~~~~
	 *
	 * Calling `RenderService::queueRenderCommand` with your command object enqueues it for the subsequent frame. The queue
	 * is handled the next time `RenderService::beginHeadlessRecording` is called in case of a `nap::HeadlessCommand`, and
	 * `RenderService::beginComputeRecording` in case of `nap::HeadlessCommand`.
	 * 
	 * ~~~~~{.cpp}
	 * mRenderService->queueRenderCommand(mPreRenderCubeMapsCommand.get());
	 * ~~~~~
	 */
	class NAPAPI RenderCommand
	{
		RTTI_ENABLE()
		friend class RenderService;
	protected:
		virtual void record(RenderService& renderService) const = 0;
	};

	/**
	 * HeadlessCommand base class
	 * 
	 * Implement this class to create a custom headless rendering command recording operation that can be queued by the render service.
	 *
	 * This class can be used to queue headless commands with priority in the subsequent frame. You can create a custom
	 * class that inherits from either `nap::HeadlessCommand` or `nap::ComputeCommand` depending on your requirements,
	 * and implement the `record` method with the vulkan commands you wish to execute with priority.
	 *
	 * ~~~~~{.h}
	 *	class PreRenderExampleCommand : public HeadlessCommand
	 *	{
	 *		RTTI_ENABLE(HeadlessCommand)
	 *	public:
	 *		PreRenderExampleCommand(RenderService& renderService) : HeadlessCommand() {}
	 *	protected:
	 *		virtual void record(RenderService& renderService) const override { ... }
	 *	};
	 *
	 *	std::unique_ptr<PreRenderCubeMapsCommand> mPreRenderCubeMapsCommand;
	 * ~~~~~
	 *
	 * Calling `RenderService::queueRenderCommand` with your command object enqueues it for the subsequent frame. The queue
	 * is handled the next time `RenderService::beginHeadlessRecording` is called in case of a `nap::HeadlessCommand`, and
	 * `RenderService::beginComputeRecording` in case of `nap::HeadlessCommand`.
	 *
	 * ~~~~~{.cpp}
	 * mRenderService->queueRenderCommand(mPreRenderCubeMapsCommand.get());
	 * ~~~~~
	 */
	class NAPAPI HeadlessCommand : public RenderCommand
	{
		RTTI_ENABLE(RenderCommand)
		friend class RenderService;
	protected:
		virtual void record(RenderService& renderService) const override = 0;
	};

	/**
	 * ComputeCommand base class
	 * 
	 * Implement this class to create a custom compute command recording operation that can be queued by the render service.
	 *
	 * This class can be used to queue compute commands with priority in the subsequent frame. You can create a custom
	 * class that inherits from either `nap::HeadlessCommand` or `nap::ComputeCommand` depending on your requirements,
	 * and implement the `record` method with the vulkan commands you wish to execute with priority.
	 *
	 * ~~~~~{.h}
	 *	class PreRenderExampleCommand : public HeadlessCommand
	 *	{
	 *		RTTI_ENABLE(HeadlessCommand)
	 *	public:
	 *		PreRenderExampleCommand(RenderService& renderService) : HeadlessCommand() {}
	 *	protected:
	 *		virtual void record(RenderService& renderService) const override { ... }
	 *	};
	 *
	 *	std::unique_ptr<PreRenderCubeMapsCommand> mPreRenderCubeMapsCommand;
	 * ~~~~~
	 *
	 * Calling `RenderService::queueRenderCommand` with your command object enqueues it for the subsequent frame. The queue
	 * is handled the next time `RenderService::beginHeadlessRecording` is called in case of a `nap::HeadlessCommand`, and
	 * `RenderService::beginComputeRecording` in case of `nap::HeadlessCommand`.
	 *
	 * ~~~~~{.cpp}
	 * mRenderService->queueRenderCommand(mPreRenderCubeMapsCommand.get());
	 * ~~~~~
	 */
	class NAPAPI ComputeCommand : public RenderCommand
	{
		RTTI_ENABLE(RenderCommand)
		friend class RenderService;
	protected:
		virtual void record(RenderService& renderService) const override = 0;
	};
}
