#include <nap/serviceableoperator.h>
#include <nap/entity.h>
#include <nap/service.h>
#include <nap/patch.h>

namespace nap
{
    // Constructor
    ServiceableOperator::ServiceableOperator()
    {
        added.connect(mAdded);
    }
    
    
    // Registers itself with the correct associated service
    void ServiceableOperator::registerWithService(Object& object)
    {
        nap::Entity* entity = getPatch()->getComponent()->getParent();
        assert(entity);
        
        rtti::TypeInfo type_info = get_type();
        nap::Service* c_service = entity->getCore().getServiceForType(type_info);
        if (c_service == nullptr)
        {
            Logger::warn("Unable to register object of type: %s with service, type not registered", type_info.get_name().data());
            return;
        }
        
        // Register
        c_service->registerObject(*this);
        
        // Set service
        mService = c_service;
        
        // Call that we've been registered
        registered();
    }
}

RTTI_DEFINE_BASE(nap::ServiceableOperator)