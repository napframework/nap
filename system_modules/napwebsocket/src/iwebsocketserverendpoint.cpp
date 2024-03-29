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

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::IWebSocketServerEndPoint)
        RTTI_PROPERTY("AllowPortReuse",			&nap::IWebSocketServerEndPoint::mAllowPortReuse,				nap::rtti::EPropertyMetaData::Default)
        RTTI_PROPERTY("LogConnectionUpdates",	&nap::IWebSocketServerEndPoint::mLogConnectionUpdates,		nap::rtti::EPropertyMetaData::Default)
        RTTI_PROPERTY("Port",					&nap::IWebSocketServerEndPoint::mPort,						nap::rtti::EPropertyMetaData::Required)
        RTTI_PROPERTY("IPAddress",				&nap::IWebSocketServerEndPoint::mIPAddress,					nap::rtti::EPropertyMetaData::Default)
        RTTI_PROPERTY("AccessMode",				&nap::IWebSocketServerEndPoint::mMode,						nap::rtti::EPropertyMetaData::Default)
        RTTI_PROPERTY("ConnectionLimit",		&nap::IWebSocketServerEndPoint::mConnectionLimit,			nap::rtti::EPropertyMetaData::Default)
        RTTI_PROPERTY("LibraryLogLevel",		&nap::IWebSocketServerEndPoint::mLibraryLogLevel,			nap::rtti::EPropertyMetaData::Default)
        RTTI_PROPERTY("AllowControlOrigin",		&nap::IWebSocketServerEndPoint::mAccessAllowControlOrigin,	nap::rtti::EPropertyMetaData::Default)
        RTTI_PROPERTY("Clients",				&nap::IWebSocketServerEndPoint::mClients,					nap::rtti::EPropertyMetaData::Default | nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

namespace nap
{ }
