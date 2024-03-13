#include "portalitemlayout.h"

RTTI_BEGIN_CLASS(nap::PortalItemPadding)
    RTTI_PROPERTY("Top", &nap::PortalItemPadding::mTop, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Bottom", &nap::PortalItemPadding::mBottom, nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

RTTI_BEGIN_ENUM(nap::EPortalItemFontWeight)
    RTTI_ENUM_VALUE(nap::EPortalItemFontWeight::THIN, "Thin"),
    RTTI_ENUM_VALUE(nap::EPortalItemFontWeight::EXTRA_LIGHT, "Extra Light"),
    RTTI_ENUM_VALUE(nap::EPortalItemFontWeight::LIGHT, "Light"),
    RTTI_ENUM_VALUE(nap::EPortalItemFontWeight::NORMAL, "Normal"),
    RTTI_ENUM_VALUE(nap::EPortalItemFontWeight::MEDIUM, "Medium"),
    RTTI_ENUM_VALUE(nap::EPortalItemFontWeight::SEMI_BOLD, "Semi Bold"),
    RTTI_ENUM_VALUE(nap::EPortalItemFontWeight::BOLD, "Bold"),
    RTTI_ENUM_VALUE(nap::EPortalItemFontWeight::EXTRA_BOLD, "Extra Bold")
RTTI_END_ENUM

RTTI_BEGIN_ENUM(nap::EPortalItemAlignment)
    RTTI_ENUM_VALUE(nap::EPortalItemAlignment::Left,    "Left"),
    RTTI_ENUM_VALUE(nap::EPortalItemAlignment::Right,   "Right")
RTTI_END_ENUM

RTTI_BEGIN_STRUCT(nap::PortalItemLayout)
    RTTI_PROPERTY("Padding", &nap::PortalItemLayout::mPadding, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Width", &nap::PortalItemLayout::mWidth, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Enabled", &nap::PortalItemLayout::mEnabled, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Visible", &nap::PortalItemLayout::mVisible, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Selected", &nap::PortalItemLayout::mSelected, nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

RTTI_BEGIN_STRUCT(nap::PortalItemTextLayout)
    RTTI_PROPERTY("FontWeight", &nap::PortalItemTextLayout::mFontWeight, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Color", &nap::PortalItemTextLayout::mColor, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("FontSize", &nap::PortalItemTextLayout::mFontSize, nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

namespace nap
{
    std::vector<float> PortalItemPadding::toVector() const
    {
        return { mTop, mBottom };
    }
}