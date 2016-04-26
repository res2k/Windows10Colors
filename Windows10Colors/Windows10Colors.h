/*
This library is licensed under the zlib license.

Copyright (C) 2016 Frank Richter

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/
#ifndef __WINDOWS10COLORS_H__
#define __WINDOWS10COLORS_H__

/**\file
 * Functions to obtain Windows 10 accent colors and colors used to paint window frames.
 */

#include <Windows.h>

namespace windows10colors
{
    /**
     * RGBA color. Red is in the LSB, Alpha in the MSB.
     * You can use GetRValue() et al to access individual components.
     */
    typedef DWORD RGBA;

    /// Accent color shades
    struct AccentColor
    {
        /// Base accent color
        RGBA accent;
        /// Darkest shade.
        RGBA darkest;
        /// Darker shade.
        RGBA darker;
        /// Dark shade.
        RGBA dark;
        /// Light shade.
        RGBA light;
        /// Lighter shade.
        RGBA lighter;
        /// Lightest shade.
        RGBA lightest;
    };

    /**
     * Return current accent color.
     * \remarks Only works on Windows 10. Check return value for success in any case.
     */
    extern HRESULT GetAccentColor (AccentColor& color);

    /// Colors for Windows 10 frame painting.
    struct FrameColors
    {
        /// Color of text for active captions.
        RGBA activeCaptionText;
        /// Background color of active captions.
        RGBA activeCaptionBG;
        /**
         * Frame color of active windows.
         * \remarks Usually the frame is drawn by DWM.
         */
        RGBA activeFrame;
        /// Color of text for inactive captions.
        RGBA inactiveCaptionText;
        /// Background color of inactive captions.
        RGBA inactiveCaptionBG;
        /**
        * Frame color of inactive windows.
        * \remarks Usually the frame is drawn by DWM.
        */
        RGBA inactiveFrame;
    };

    /**
     * Get colors used to paint window frames.
     * \remarks Only produces sensible result on Windows 10.
     *  May return success on other platforms but return colors that don't match
     *  what the user sees.
     *  Check return value for success in any case.
     */
    extern HRESULT GetFrameColors (FrameColors& color);
} // namespace windows10colors

#endif // __WINDOWS10COLORS_H__

