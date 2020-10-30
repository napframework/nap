/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "napkinlinkresolver.h"

namespace napkin
{
	NapkinLinkResolver::NapkinLinkResolver(const nap::rtti::OwnedObjectList& objects) :
		DefaultLinkResolver(objects)
	{
	}
		
	bool NapkinLinkResolver::sResolveLinks(const nap::rtti::OwnedObjectList& objects, const nap::rtti::UnresolvedPointerList& unresolvedPointers, nap::utility::ErrorState& errorState)
	{
		NapkinLinkResolver resolver(objects);
		return resolver.resolveLinks(unresolvedPointers, errorState);
	}
		

	nap::rtti::LinkResolver::EInvalidLinkBehaviour NapkinLinkResolver::onInvalidLink(const nap::rtti::UnresolvedPointer& unresolvedPointer)
	{
		return LinkResolver::EInvalidLinkBehaviour::Ignore;
	}
}