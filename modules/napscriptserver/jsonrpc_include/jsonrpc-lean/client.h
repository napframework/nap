// This file is part of xsonrpc, an XML/JSON RPC library.
// Copyright (C) 2015 Erik Johansson <erik@ejohansson.se>
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

#ifndef JSONRPC_LEAN_CLIENT_H
#define JSONRPC_LEAN_CLIENT_H

#include "request.h"
#include "value.h"
#include "fault.h"
#include "formathandler.h"
#include "jsonformathandler.h"
#include "reader.h"
#include "response.h"
#include "writer.h"
#include "formatteddata.h"
#include "jsonformatteddata.h"
#include "dispatcher.h"

#include <functional>
#include <string>
#include <memory>
#include <stdexcept>

namespace jsonrpc {

    class FormatHandler;

    class Client {
    public:
        Client(FormatHandler& formatHandler) : myFormatHandler(formatHandler), myId(0) {

        }

        ~Client() {}

        std::shared_ptr<FormattedData> BuildRequestData(const std::string& methodName, const Request::Parameters& params = {}) {
            return BuildRequestDataInternal(methodName, params);
        }

        template<typename FirstType, typename... RestTypes>
        typename std::enable_if<!std::is_same<typename std::decay<FirstType>::type, Request::Parameters>::value, std::shared_ptr<FormattedData>>::type
        BuildRequestData(const std::string& methodName, FirstType&& first, RestTypes&&... rest) {
            Request::Parameters params;
            params.emplace_back(std::forward<FirstType>(first));

            return BuildRequestDataInternal(methodName, params, std::forward<RestTypes>(rest)...);
        }

        std::shared_ptr<FormattedData> BuildNotificationData(const std::string& methodName, const Request::Parameters& params = {}) {
            return BuildNotificationDataInternal(methodName, params);
        }

        template<typename FirstType, typename... RestTypes>
        typename std::enable_if<!std::is_same<typename std::decay<FirstType>::type, Request::Parameters>::value, std::shared_ptr<FormattedData>>::type
        BuildNotificationData(const std::string& methodName, FirstType&& first, RestTypes&&... rest) {
            Request::Parameters params;
            params.emplace_back(std::forward<FirstType>(first));

            return BuildNotificationDataInternal(methodName, params, std::forward<RestTypes>(rest)...);
        }

        Response ParseResponse(const std::string& aResponseData) {
            return ParseResponseInternal(aResponseData);
        }

        Client(const Client&) = delete;
        Client& operator=(const Client&) = delete;
        Client(Client&&) = delete;
        Client& operator=(Client&&) = delete;

    private:
        template<typename FirstType, typename... RestTypes>
        std::shared_ptr<FormattedData> BuildRequestDataInternal(const std::string& methodName, Request::Parameters& params, FirstType&& first, RestTypes&&... rest) {
            params.emplace_back(std::forward<FirstType>(first));
            return BuildRequestDataInternal(methodName, params, std::forward<RestTypes>(rest)...);
        }

        std::shared_ptr<FormattedData> BuildRequestDataInternal(const std::string& methodName, const Request::Parameters& params) {
            auto writer = myFormatHandler.CreateWriter();
            const auto id = myId++;
            Request::Write(methodName, params, id, *writer);
            return writer->GetData();
        }

        template<typename FirstType, typename... RestTypes>
        std::shared_ptr<FormattedData> BuildNotificationDataInternal(const std::string& methodName, Request::Parameters& params, FirstType&& first, RestTypes&&... rest) {
            params.emplace_back(std::forward<FirstType>(first));
            return BuildNotificationDataInternal(methodName, params, std::forward<RestTypes>(rest)...);
        }

        std::shared_ptr<FormattedData> BuildNotificationDataInternal(const std::string& methodName, const Request::Parameters& params) {
            auto writer = myFormatHandler.CreateWriter();
            Request::Write(methodName, params, false, *writer);
            return writer->GetData();
        }

        Response ParseResponseInternal(const std::string& aResponseData) {
            auto reader = myFormatHandler.CreateReader(aResponseData);
            Response response = reader->GetResponse();
            response.ThrowIfFault();
            return std::move(response);
        }

        FormatHandler& myFormatHandler;
        int32_t myId;
    };

} // namespace jsonrpc

#endif // JSONRPC_LEAN_CLIENT_H
