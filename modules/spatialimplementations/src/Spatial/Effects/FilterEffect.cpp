
#include "FilterEffect.h"

RTTI_BEGIN_CLASS(nap::spatial::FilterEffect)
    RTTI_PROPERTY("FilterType", &nap::spatial::FilterEffect::mFilterChainType, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::FilterEffectProcessor)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::FilterEffectInstance)
RTTI_END_CLASS
