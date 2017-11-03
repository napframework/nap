#pragma once

// External includes
#include <nap/service.h>
#include <utility/dllexport.h>

namespace nap
{
	// Forward Declares
	class RenderService;

	class NAPAPI IMGuiService : public Service
	{
		RTTI_ENABLE(Service)
	public:
		// Default constructor
		IMGuiService() = default;

		void render();

	protected:
		/**
		* Initializes the IMGui library
		* @return if the lib was initialized successfully
		*/
		virtual bool init(utility::ErrorState& error) override;

		virtual void getDependentServices(std::vector<rtti::TypeInfo>& dependencies) override;

		virtual void resourcesLoaded() override;

		virtual void update(double deltaTime) override;

		virtual void shutdown() override;

	private:
		RenderService* mRenderer = nullptr;
	};
}