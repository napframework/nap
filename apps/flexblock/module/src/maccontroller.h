#pragma once

// External Includes
#include <nap/device.h>
#include <ethercatmaster.h>

namespace nap
{
	/**
	 * JVL CAM motor ethercat controller
	 */
	class NAPAPI MACController : public EtherCATMaster
	{
		RTTI_ENABLE(EtherCATMaster)
	public:

		// Stops the device
		virtual ~MACController() override;

	protected:

		/**
		 * Called when a slave reaches the pre-operational stage on the network.
		 * Note that this function can be called from multiple threads.
		 * SDO communication is possible. No PDO communication.
		 *
		 * Override this call to register a slave setup function, for example:
		 * void MyMaster::onPreOperational(void* slave)
		 * {
		 *		reinterpret_cast<ec_slavet*>(slave)->PO2SOconfig = &MAC400_SETUP;
		 * }
		 *
		 * You typically use the setup function to create your own custom PDO mapping.
		 * @param slave ec_slavet* pointer to the slave on the network.
		 * @param index slave index into SOEM ec_slave array.
		 */
		virtual void onPreOperational(void* slave, int index) override;

		/**
		 * Called when a slave reaches the safe operational stage on the network.
		 * Note that this function can be called from multiple threads.
		 * PDO transmission is operational (slave sends data to master).
		 * @param slave ec_slavet* pointer to the slave on the network.
		 * @param index slave index into SOEM ec_slave array.
		 */
		virtual void onSafeOperational(void* slave, int index) override;

		/**
		 * Called from the processing thread at a fixed interval defined by the cycle time property.
		 * Allows for real-time interaction with an ether-cat slave on the network by
		 * reading and writing to the slave's inputs or outputs.
		 * At this stage no SDO operations should take place, only operations that
		 * involve the access and modification of the PDO map.
		 */
		virtual void onProcess() override;

	};
}
