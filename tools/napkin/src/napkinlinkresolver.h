/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <rtti/defaultlinkresolver.h>

namespace napkin
{
	/**
	 * Napkin-specific implementation of DefaultLinkResolver that ignores invalid unresolved links
	 */
	class NapkinLinkResolver : public nap::rtti::DefaultLinkResolver
	{
	public:
		NapkinLinkResolver(const nap::rtti::OwnedObjectList& objects);

		/**
		 * Convenience function that internally just constructs a NapkinLinkResolver and passes on the arguments
		 */
		static bool sResolveLinks(const nap::rtti::OwnedObjectList& objects, const nap::rtti::UnresolvedPointerList& unresolvedPointers, nap::utility::ErrorState& errorState);

	private:
		virtual EInvalidLinkBehaviour onInvalidLink(const nap::rtti::UnresolvedPointer& unresolvedPointer) override;
	};
}
