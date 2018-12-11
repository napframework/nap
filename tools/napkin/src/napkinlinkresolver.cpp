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