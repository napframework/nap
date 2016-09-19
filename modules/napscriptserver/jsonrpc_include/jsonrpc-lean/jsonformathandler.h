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
#pragma once
#ifndef JSONRPC_LEAN_JSONFORMATHANDLER_H
#define JSONRPC_LEAN_JSONFORMATHANDLER_H

#include "formathandler.h"
#include "jsonreader.h"
#include "jsonwriter.h"

#include <memory>

namespace jsonrpc {

    const char APPLICATION_JSON[] = "application/json";

    class JsonFormatHandler : public FormatHandler {
    public:
        explicit JsonFormatHandler() {}

        // FormatHandler
        bool CanHandleRequest(const std::string& contentType) override {
            return contentType == APPLICATION_JSON;
        }

        std::string GetContentType() override {
            return APPLICATION_JSON;
        }

        bool UsesId() override {
            return true;
        }

        std::unique_ptr<Reader> CreateReader(const std::string& data) override {
            return std::unique_ptr<Reader>(std::make_unique<JsonReader>(std::move(data)));
        }

        std::unique_ptr<Writer> CreateWriter() override {
            return std::unique_ptr<Writer>(std::make_unique<JsonWriter>());
        }

    private:

    };

} // namespace jsonrpc

#endif // JSONRPC_LEAN_JSONFORMATHANDLER_H
