/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "iwebsocketserverendpoint.h"

RTTI_BEGIN_ENUM(nap::IWebSocketServerEndPoint::EAccessMode)
        RTTI_ENUM_VALUE(nap::IWebSocketServerEndPoint::EAccessMode::EveryOne, "Everyone"),
        RTTI_ENUM_VALUE(nap::IWebSocketServerEndPoint::EAccessMode::Ticket, "Ticket"),
        RTTI_ENUM_VALUE(nap::IWebSocketServerEndPoint::EAccessMode::Reserved, "Reserved")
RTTI_END_ENUM

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::IWebSocketServerEndPoint, "Manages all websocket client connections")
        RTTI_PROPERTY("AllowPortReuse",			&nap::IWebSocketServerEndPoint::mAllowPortReuse,			nap::rtti::EPropertyMetaData::Default,	"If the server connection can be re-used by other processes")
        RTTI_PROPERTY("LogConnectionUpdates",	&nap::IWebSocketServerEndPoint::mLogConnectionUpdates,		nap::rtti::EPropertyMetaData::Default,	"If client / server connect information is logged to the console")
        RTTI_PROPERTY("Port",					&nap::IWebSocketServerEndPoint::mPort,						nap::rtti::EPropertyMetaData::Required,	"Port to open and listen to for client requests")
        RTTI_PROPERTY("IPAddress",				&nap::IWebSocketServerEndPoint::mIPAddress,					nap::rtti::EPropertyMetaData::Default,	"Local IP Address, when left empty the first available ethernet adapter is chosen")
        RTTI_PROPERTY("AccessMode",				&nap::IWebSocketServerEndPoint::mMode,						nap::rtti::EPropertyMetaData::Default,	"AccessMode client connection access mode")
        RTTI_PROPERTY("ConnectionLimit",		&nap::IWebSocketServerEndPoint::mConnectionLimit,			nap::rtti::EPropertyMetaData::Default,	"Number of allowed client connections at once, -1 = no limit")
        RTTI_PROPERTY("LibraryLogLevel",		&nap::IWebSocketServerEndPoint::mLibraryLogLevel,			nap::rtti::EPropertyMetaData::Default,	"Only log messages equal or higher than selected library log level")
        RTTI_PROPERTY("AllowControlOrigin",		&nap::IWebSocketServerEndPoint::mAccessAllowControlOrigin,	nap::rtti::EPropertyMetaData::Default,	"Access-Control-Allow-Origin response header value, controls if the server response can be shared with request code from the given origin")
        RTTI_PROPERTY("Clients",				&nap::IWebSocketServerEndPoint::mClients,					nap::rtti::EPropertyMetaData::Default | nap::rtti::EPropertyMetaData::Embedded, "All authorized clients when mode is set to 'Reserved'")
RTTI_END_CLASS

namespace nap
{ }
