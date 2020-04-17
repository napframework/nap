#pragma once
// Local Includes
#include "sequenceplayer.h"

// External Includes
#include <nap/service.h>
#include <entity.h>
#include <nap/datetime.h>

namespace nap
{
	class SequenceEventReceiver;
	class SequencePlayerParameterSetterBase;

	class NAPAPI SequenceService : public Service
	{
		friend class SequenceEventReceiver;
		friend class SequencePlayerParameterSetterBase;

		RTTI_ENABLE(Service)
	public:
		// Default Constructor
		SequenceService(ServiceConfiguration* configuration);

		// Default Destructor
		virtual ~SequenceService();

	protected:
		/**
		* Registers all objects that need a specific way of construction
		* @param factory the factory to register the object creators with
		*/
		virtual void registerObjectCreators(rtti::Factory& factory) override;

		virtual bool init(nap::utility::ErrorState& errorState) override;

		virtual void update(double deltaTime) override;
	private:
		void registerEventReceiver(SequenceEventReceiver& receiver);

		void removeEventReceiver(SequenceEventReceiver& receiver);

		void registerParameterSetter(SequencePlayerParameterSetterBase& setter);

		void removeParameterSetter(SequencePlayerParameterSetterBase& setter);

		std::vector<SequenceEventReceiver*>			mEventReceivers;
		std::vector<SequencePlayerParameterSetterBase*> mParameterSetters;
	};
}