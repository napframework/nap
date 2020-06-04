#include "MultiSwitch.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::MultiSwitchNode)
    RTTI_PROPERTY("audioOutput", &nap::audio::MultiSwitchNode::audioOutput, nap::rtti::EPropertyMetaData::Embedded)
    RTTI_FUNCTION("select", &nap::audio::MultiSwitchNode::select)
    RTTI_FUNCTION("connect", &nap::audio::MultiSwitchNode::connect)
    RTTI_FUNCTION("getInputCount", &nap::audio::MultiSwitchNode::getInputCount)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::audio::MultiSwitch)
    RTTI_PROPERTY("Inputs", &nap::audio::MultiSwitch::mInputs, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("Selection", &nap::audio::MultiSwitch::mSelection, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS
