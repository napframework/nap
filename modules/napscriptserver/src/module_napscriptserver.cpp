#include <nap/module.h>
#include "scriptservercomponent.h"

NAP_MODULE_BEGIN(NapScriptServer)
{
    NAP_REGISTER_COMPONENT(nap::JSONRPCServerComponent)
    NAP_REGISTER_COMPONENT(nap::PythonServerComponent)
}
NAP_MODULE_END(NapScriptServer)

