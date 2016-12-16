#include <nap/module.h>
#include "jsonrpcservice.h"

NAP_MODULE_BEGIN(NapScriptServer)
{
    NAP_REGISTER_DATATYPE(nap::JsonRpcService)
}
NAP_MODULE_END(NapScriptServer)

