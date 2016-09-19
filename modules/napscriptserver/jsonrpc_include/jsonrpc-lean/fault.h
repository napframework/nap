// This file is derived from xsonrpc Copyright (C) 2015 Erik Johansson <erik@ejohansson.se>
// This file is part of jsonrpc-lean, a c++11 JSON-RPC client/server library.
//
// Modifications and additions Copyright (C) 2015 Adriano Maia <tony@stark.im>
//
// This library is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation; either version 2.1 of the License, or (at your
// option) any later version.
//
// This library is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
// for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library; if not, write to the Free Software Foundation,
// Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

#ifndef JSONRPC_LEAN_FAULT_H
#define JSONRPC_LEAN_FAULT_H

#include <cassert>
#include <cstdint>
#include <exception>
#include <string>

namespace jsonrpc {

    class Fault : public std::exception {
    public:
        enum ReservedCodes : int32_t {
            RESERVED_CODE_MIN = -32768,
            RESERVED_CODE_MAX = -32000,
            SERVER_ERROR_CODE_MIN = -32099,
            SERVER_ERROR_CODE_MAX = -32000,
            PARSE_ERROR = -32700,
            INVALID_REQUEST = -32600,
            METHOD_NOT_FOUND = -32601,
            INVALID_PARAMETERS = -32602,
            INTERNAL_ERROR = -32603,
        };

        Fault(std::string faultString, int32_t faultCode = 0)
            : myFaultString(std::move(faultString)),
            myFaultCode(faultCode) {
            // Applications are not allowed to set these fault codes
            assert(myFaultCode < RESERVED_CODE_MIN || myFaultCode > RESERVED_CODE_MAX);
            if (myFaultCode >= RESERVED_CODE_MIN && myFaultCode <= RESERVED_CODE_MAX) {
                myFaultCode = 0;
            }
        }

        int32_t GetCode() const { return myFaultCode; }
        const std::string& GetString() const { return myFaultString; }

        const char* what() const noexcept override
        {
            return myFaultString.c_str();
        }

    private:
        Fault(int32_t faultCode, std::string faultString)
            : myFaultString(std::move(faultString)),
            myFaultCode(faultCode) {
        }

        std::string myFaultString;
        int32_t myFaultCode;

        friend class PreDefinedFault;
    };

    class PreDefinedFault : public Fault {
    protected:
        PreDefinedFault(int32_t faultCode, std::string faultString)
            : Fault(faultCode, std::move(faultString)) {
        }

        friend class Response;
    };

    class ParseErrorFault : public PreDefinedFault {
    public:
        ParseErrorFault(std::string string = "Parse error")
            : PreDefinedFault(PARSE_ERROR, std::move(string)) {
        }
    };

    class InvalidRequestFault : public PreDefinedFault {
    public:
        InvalidRequestFault(std::string string = "Invalid request")
            : PreDefinedFault(INVALID_REQUEST, std::move(string)) {
        }
    };

    class MethodNotFoundFault : public PreDefinedFault {
    public:
        MethodNotFoundFault(std::string string = "Method not found")
            : PreDefinedFault(METHOD_NOT_FOUND, std::move(string)) {
        }
    };

    class InvalidParametersFault : public PreDefinedFault {
    public:
        InvalidParametersFault(std::string string = "Invalid parameters")
            : PreDefinedFault(INVALID_PARAMETERS, std::move(string)) {
        }
    };

    class InternalErrorFault : public PreDefinedFault {
    public:
        InternalErrorFault(std::string string = "Internal error")
            : PreDefinedFault(INTERNAL_ERROR, std::move(string)) {
        }
    };

    class ServerErrorFault : public PreDefinedFault {
    public:
        ServerErrorFault(int32_t code, std::string string)
            : PreDefinedFault(code, std::move(string)) {
            assert(code >= SERVER_ERROR_CODE_MIN && code <= SERVER_ERROR_CODE_MAX);
        }
    };

} // namespace jsonrpc

#endif //JSONRPC_LEAN_FAULT_H
