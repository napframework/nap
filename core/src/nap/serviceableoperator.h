#pragma once

#include <rtti/rtti.h>

// NAP Includes
#include <nap/attribute.h>
#include <nap/operator.h>
#include <nap/signalslot.h>

namespace nap
{
    class Service;
    
    /**
     @brief Serviceable Operator
     
     A specialization of a operator that a service uses -> ie: client of a service
     This operator automatically registers itself with a service after being attached to an entity -> receives a parent
     When destructed the component de-registers itself from a service
     **/
    
    class ServiceableOperator : public Operator
    {
        RTTI_ENABLE_DERIVED_FROM(Component)
    public:
        ServiceableOperator();
        
    protected:
        // Service to which this component is a client
        Service* mService = nullptr;
        
        // Called when the component has been registered
        virtual void registered()	{ }
        
    private:
        // Slots that handle service registration / deregistration
        Slot<const Object&> mAdded = { this, &ServiceableOperator::registerWithService };
        
        // Slot Calls
        void registerWithService(const Object& object);
    };
}

RTTI_DECLARE_BASE(nap::ServiceableOperator)
