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

#ifndef JSONRPC_LEAN_JSONWRITER_H
#define JSONRPC_LEAN_JSONWRITER_H

#include "writer.h"
#include "json.h"
#include "util.h"
#include "value.h"
#include "jsonformatteddata.h"

#define RAPIDJSON_NO_SIZETYPEDEFINE
namespace rapidjson { typedef ::std::size_t SizeType; }

#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

namespace jsonrpc {

    class JsonWriter final : public Writer {
    public:
        JsonWriter() : myRequestData(new JsonFormattedData()) {
        }

        // Writer
        std::shared_ptr<FormattedData> GetData() override {
            return std::static_pointer_cast<FormattedData>(myRequestData);
        }

        void StartDocument() override {
            // Empty
        }

        void EndDocument() override {
            // Empty
        }

        void StartRequest(const std::string& methodName, const Value& id) override {
            myRequestData->Writer.StartObject();

            myRequestData->Writer.Key(json::JSONRPC_NAME, sizeof(json::JSONRPC_NAME) - 1);
            myRequestData->Writer.String(json::JSONRPC_VERSION_2_0, sizeof(json::JSONRPC_VERSION_2_0) - 1);

            myRequestData->Writer.Key(json::METHOD_NAME, sizeof(json::METHOD_NAME) - 1);
            myRequestData->Writer.String(methodName.data(), methodName.size(), true);

            WriteId(id);

            myRequestData->Writer.Key(json::PARAMS_NAME, sizeof(json::PARAMS_NAME) - 1);
            myRequestData->Writer.StartArray();
        }

        void EndRequest() override {
            myRequestData->Writer.EndArray();
            myRequestData->Writer.EndObject();
        }

        void StartParameter() override {
            // Empty
        }

        void EndParameter() override {
            // Empty
        }

        void StartResponse(const Value& id) override {
            myRequestData->Writer.StartObject();

            myRequestData->Writer.Key(json::JSONRPC_NAME, sizeof(json::JSONRPC_NAME) - 1);
            myRequestData->Writer.String(json::JSONRPC_VERSION_2_0, sizeof(json::JSONRPC_VERSION_2_0) - 1);

            WriteId(id);

            myRequestData->Writer.Key(json::RESULT_NAME, sizeof(json::RESULT_NAME) - 1);
        }

        void EndResponse() override {
            myRequestData->Writer.EndObject();
        }

        void StartFaultResponse(const Value& id) override {
            myRequestData->Writer.StartObject();

            myRequestData->Writer.Key(json::JSONRPC_NAME, sizeof(json::JSONRPC_NAME) - 1);
            myRequestData->Writer.String(json::JSONRPC_VERSION_2_0, sizeof(json::JSONRPC_VERSION_2_0) - 1);

            WriteId(id);
        }

        void EndFaultResponse() override {
            myRequestData->Writer.EndObject();
        }

        void WriteFault(int32_t code, const std::string& string) override {
            myRequestData->Writer.Key(json::ERROR_NAME, sizeof(json::ERROR_NAME) - 1);
            myRequestData->Writer.StartObject();

            myRequestData->Writer.Key(json::ERROR_CODE_NAME, sizeof(json::ERROR_CODE_NAME) - 1);
            myRequestData->Writer.Int(code);

            myRequestData->Writer.Key(json::ERROR_MESSAGE_NAME, sizeof(json::ERROR_MESSAGE_NAME) - 1);
            myRequestData->Writer.String(string.data(), string.size(), true);

            myRequestData->Writer.EndObject();
        }

        void StartArray() override {
            myRequestData->Writer.StartArray();
        }

        void EndArray() override {
            myRequestData->Writer.EndArray();
        }

        void StartStruct() override {
            myRequestData->Writer.StartObject();
        }

        void EndStruct() override {
            myRequestData->Writer.EndObject();
        }

        void StartStructElement(const std::string& name) override {
            myRequestData->Writer.Key(name.data(), name.size(), true);
        }

        void EndStructElement() override {
            // Empty
        }

        void WriteBinary(const char* data, size_t size) override {
            myRequestData->Writer.String(data, size, true);
        }

        void WriteNull() override {
            myRequestData->Writer.Null();
        }

        void Write(bool value) override {
            myRequestData->Writer.Bool(value);
        }

        void Write(double value) override {
            myRequestData->Writer.Double(value);
        }

        void Write(int32_t value) override {
            myRequestData->Writer.Int(value);
        }

        void Write(int64_t value) override {
            myRequestData->Writer.Int64(value);
        }

        void Write(const std::string& value) override {
            myRequestData->Writer.String(value.data(), value.size(), true);
        }

        void Write(const tm& value) override {
            Write(util::FormatIso8601DateTime(value));
        }

    private:
        void WriteId(const Value& id) {
            if (id.IsString() || id.IsInteger32() || id.IsInteger64() || id.IsNil()) {
                myRequestData->Writer.Key(json::ID_NAME, sizeof(json::ID_NAME) - 1);
                if (id.IsString()) {
                    myRequestData->Writer.String(id.AsString().data(), id.AsString().size(), true);
                } else if (id.IsInteger32()) {
                    myRequestData->Writer.Int(id.AsInteger32());
                } else if (id.IsInteger64()) {
                    myRequestData->Writer.Int64(id.AsInteger64());
                } else {
                    myRequestData->Writer.Null();
                }
            }
        }

        std::shared_ptr<JsonFormattedData> myRequestData;
    };

} // namespace jsonrpc

#endif // JSONRPC_LEAN_JSONWRITER_H
