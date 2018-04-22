#pragma once

// External Includes
#include <nap/service.h>

namespace nap
{
	class NAPAPI @MODULE_NAME_PASCALCASE@Service : public Service
	{
		RTTI_ENABLE(Service)

	public:
		// Default Constructor
		@MODULE_NAME_PASCALCASE@Service() = default;

		// Disable copy
		@MODULE_NAME_PASCALCASE@Service(const @MODULE_NAME_PASCALCASE@Service& rhs) = delete;
		@MODULE_NAME_PASCALCASE@Service& operator=(const @MODULE_NAME_PASCALCASE@Service& rhs) = delete;

	protected:
		
		/**
		 *
		 */
		virtual void getDependentServices(std::vector<rtti::TypeInfo>& dependencies) override;
		
		/**
		 * Initializes the service
		 * @param errorState contains the error message on failure
		 * @return if the video service was initialized correctly
		 */
		virtual bool init(nap::utility::ErrorState& errorState) override;
		
		/**
		 *
		 */
		virtual void update(double deltaTime) override;
		
	private:
		
	};
}
