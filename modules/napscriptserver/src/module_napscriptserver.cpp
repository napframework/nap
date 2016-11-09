#include <nap/module.h>
#include "jsonrpcserver.h"

NAP_MODULE_BEGIN(NapScriptServer)
{
    NAP_REGISTER_COMPONENT(nap::JSONRPCServerComponent)
}
NAP_MODULE_END(NapScriptServer)

