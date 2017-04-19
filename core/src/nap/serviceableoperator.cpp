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
    void ServiceableOperator::registerWithService(const Object& object)
    {
        nap::Entity* entity = getPatch()->getComponent()->getParent();
        assert(entity);
        
        RTTI::TypeInfo type_info = getTypeInfo();
        nap::Service* c_service = entity->getCore().getServiceForType(type_info);
        if (c_service == nullptr)
        {
            Logger::warn("Unable to register object of type: " + type_info.getName() + " with service, type not registered");
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