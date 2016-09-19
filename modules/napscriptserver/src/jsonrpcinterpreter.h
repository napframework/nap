#pragma once

#include "scriptinterpreter.h"

#include "jsonrpc-lean/server.h"

namespace nap {

    class JSONRPCInterpreter : public ScriptInterpreter
    {
        RTTI_ENABLE_DERIVED_FROM(ScriptInterpreter)
    public:
        JSONRPCInterpreter();


        virtual std::string evalScript(const std::string &cmd) override;

    private:
        jsonrpc::Server mServer;
        jsonrpc::JsonFormatHandler mFormatHandler;
    };

}

RTTI_DECLARE(nap::JSONRPCInterpreter)