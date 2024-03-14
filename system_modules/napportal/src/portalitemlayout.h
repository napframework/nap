#pragma once

#include <nap/core.h>
#include "color.h"

namespace nap
{
    /**
     * Padding for a portal item
     */
    struct NAPAPI PortalItemPadding
    {
        PortalItemPadding() = default;
        PortalItemPadding(float top, float bottom)
            : mTop(top), mBottom(bottom){}

        // properties
        float mTop       = 0.0f; ///< Property : "Top" The padding at the top
        float mBottom    = 0.0f; ///< Property : "Bottom" The padding at the bottom

        /**
         * @return the padding as an std::vector<float>
         * Order is top, bottom
         */
        std::vector<float> toVector() const;

        //////////////////////////////////////////////////////////////////////////
        /// overloads
        //////////////////////////////////////////////////////////////////////////

        bool operator == (const PortalItemPadding &c) const
        {
            if (mTop == c.mTop && mBottom == c.mBottom)
                return true;

            return false;
        }

        bool operator != (const PortalItemPadding &c) const
        {
            return !(*this == c);
        }
    };


    /**
     * Font weight for a portal item label
     */
    enum NAPAPI EPortalItemFontWeight : int
    {
       THIN         = 100,
       EXTRA_LIGHT  = 200,
       LIGHT        = 300,
       NORMAL       = 400,
       MEDIUM       = 500,
       SEMI_BOLD    = 600,
       BOLD         = 700,
       EXTRA_BOLD   = 800
    };

    /**
     * Portal item alignment, whether to use the left or right column of the table
     */
    enum NAPAPI EPortalItemAlignment : int
    {
        Left = 0,
        Right = 1
    };

    /**
     * Layout for a portal item
     */
    struct NAPAPI PortalItemLayout
    {
        // properties
        PortalItemPadding mPadding = { 0, 0 }; ///< Property : "Padding" The padding of the portal item
        bool mEnabled = true; ///< Property : "Enabled" If the item is enabled
        bool mVisible = true; ///< Property : "Visible" If the item is visible
        bool mSelected = false; ///< Property : "Selected" If the item is selected
        float mWidth = -1.0f; ///< Property : "Width" The width of the item

        //////////////////////////////////////////////////////////////////////////
        /// operator overloads
        //////////////////////////////////////////////////////////////////////////

        bool operator == (const PortalItemLayout &c) const
        {
            if (mPadding == c.mPadding &&
                mEnabled == c.mEnabled &&
                mVisible == c.mVisible &&
                mSelected == c.mSelected &&
                mWidth == c.mWidth)
                return true;

            return false;
        }

        bool operator != (const PortalItemLayout &c) const
        {
            return !(*this == c);
        }
    };

    /**
     * Layout for a portal item label
     */
    struct NAPAPI PortalItemTextLayout
    {
        // properties
        EPortalItemFontWeight mFontWeight = EPortalItemFontWeight::NORMAL; ///< Property : "FontWeight" The weight of the font
        RGBColor8 mColor = {205, 205, 195}; ///< Property : "Color" The color of the text
        int mFontSize = 14; ///< Property : "FontSize" The size of the font

        //////////////////////////////////////////////////////////////////////////
        /// operator overloads
        //////////////////////////////////////////////////////////////////////////

        bool operator == (const PortalItemTextLayout &c) const
        {
            if (mFontWeight == c.mFontWeight && mColor == c.mColor && mFontSize == c.mFontSize)
                return true;

            return false;
        }

        bool operator != (const PortalItemTextLayout &c) const
        {
            return !(*this == c);
        }
    };
}