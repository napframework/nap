/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "iwebsocketclientendpoint.h"

// nap::websocketclientendpoint run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::IWebSocketClientEndPoint, "Manages a list of client-server connections and acts as the main portal for the client to the server")
	RTTI_PROPERTY("LogConnectionUpdates",	&nap::IWebSocketClientEndPoint::mLogConnectionUpdates,	nap::rtti::EPropertyMetaData::Default,	"Log client / server connection information to console.")
	RTTI_PROPERTY("LibraryLogLevel",		&nap::IWebSocketClientEndPoint::mLibraryLogLevel,		nap::rtti::EPropertyMetaData::Default,	"Only log information equal or higher than this log level")
RTTI_END_CLASS

namespace nap
{ }
