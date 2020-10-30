/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "apiutils.h"

// External Includes
#include <rtti/jsonreader.h>
#include <rtti/jsonwriter.h>
#include <rtti/defaultlinkresolver.h>

namespace nap
{
	bool extractMessages(const std::string& json, rtti::DeserializeResult& result, rtti::Factory& factory, std::vector<APIMessage*>& outMessages, utility::ErrorState& error)
	{
		// De-serialize json cmd
		if (!rtti::deserializeJSON(json, rtti::EPropertyValidationMode::DisallowMissingProperties, rtti::EPointerPropertyMode::OnlyRawPointers, factory, result, error))
			return false;

		// Resolve links
		if (!rtti::DefaultLinkResolver::sResolveLinks(result.mReadObjects, result.mUnresolvedPointers, error))
			return false;

		// Fetch all api messages
		for (const auto& object : result.mReadObjects)
		{
			// Check if it's a message
			if (!object->get_type().is_derived_from(RTTI_OF(nap::APIMessage)))
				continue;

			// Cast to message and add as possible event
			nap::APIMessage* message = rtti_cast<nap::APIMessage>(object.get());
			assert(message != nullptr);
			if (message != nullptr)
				outMessages.emplace_back(message);
		}

		// Error when json doesn't contain any messages
		if (!error.check(!outMessages.empty(), "JSON doesn't contain any nap::APIMessage objects"))
			return false;

		return true;
	}
}