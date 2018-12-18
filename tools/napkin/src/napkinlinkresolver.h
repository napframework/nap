#pragma once

#include "rtti/defaultlinkresolver.h"

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
