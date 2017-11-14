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

Original source:
https://github.com/res2k/Windows10Colors

*/
#include "Windows10Colors.h"

#include <comdef.h>
#include <wrl.h>

#include <windows.ui.viewmanagement.h>

namespace windows10colors
{

namespace WindowsUI = ABI::Windows::UI;
using namespace Microsoft::WRL;

/// Helper macro to exit early in case of failure HRESULTs.
#define CHECKED(X)     do { HRESULT hr = (X); if (FAILED(hr)) return hr; } while (false)

namespace
{
    /// Wrapper for the few WinRT functions we need to use
    class WinRT
    {
        bool modules_loaded = false;
        HMODULE winrt = 0;
        HMODULE winrt_string = 0;

        typedef HRESULT (STDAPICALLTYPE *pfnWindowsCreateStringReference)(
            PCWSTR sourceString, UINT32 length, HSTRING_HEADER* hstringHeader, HSTRING* string);
        pfnWindowsCreateStringReference pWindowsCreateStringReference = nullptr;
        typedef HRESULT (WINAPI *pfnRoActivateInstance)(HSTRING activatableClassId, IInspectable** instance);
        pfnRoActivateInstance pRoActivateInstance = nullptr;
    protected:
        WinRT ()
        {}
        ~WinRT ()
        {
            if (winrt) FreeLibrary (winrt);
            if (winrt_string) FreeLibrary (winrt);
        }

        static WinRT instance;

        /// Load DLLs
        bool init ()
        {
            if (!modules_loaded)
            {
                modules_loaded = true;
                winrt = LoadLibraryW (L"api-ms-win-core-winrt-l1-1-0.dll");
                if (winrt)
                {
                    pRoActivateInstance = reinterpret_cast<pfnRoActivateInstance> (
                        GetProcAddress (winrt, "RoActivateInstance"));
                }
                winrt_string = LoadLibraryW (L"api-ms-win-core-winrt-string-l1-1-0.dll");
                if (winrt_string)
                {
                    pWindowsCreateStringReference = reinterpret_cast<pfnWindowsCreateStringReference> (
                        GetProcAddress (winrt_string, "WindowsCreateStringReference"));
                }
            }
            return winrt && winrt_string;
        }

        /// Wrap WindowsCreateStringReference
        inline HRESULT WindowsCreateStringReferenceImpl (PCWSTR sourceString, UINT32 length, HSTRING_HEADER* hstringHeader, HSTRING* string)
        {
            if (!init () || !pWindowsCreateStringReference) return E_NOTIMPL;
            return pWindowsCreateStringReference (sourceString, length, hstringHeader, string);
        }
        /// Wrap RoActivateInstance
        inline HRESULT RoActivateInstanceImpl (HSTRING activatableClassId, IInspectable** instance)
        {
            if (!init () || !pRoActivateInstance) return E_NOTIMPL;
            return pRoActivateInstance (activatableClassId, instance);
        }
    public:
        /// Dynamically loaded WindowsCreateStringReference, if available
        static HRESULT WindowsCreateStringReference (PCWSTR sourceString, UINT32 length, HSTRING_HEADER* hstringHeader, HSTRING* string)
        {
            return instance.WindowsCreateStringReferenceImpl (sourceString, length, hstringHeader, string);
        }
        /// Dynamically loaded RoActivateInstance, if available
        static HRESULT RoActivateInstance (HSTRING activatableClassId, IInspectable** newInstance)
        {
            return instance.RoActivateInstanceImpl (activatableClassId, newInstance);
        }
    };

    WinRT WinRT::instance;

    /// Wrapper class for WinRT string reference
    class HStringRef
    {
        HSTRING hstr;
        HSTRING_HEADER str_header;
    public:
        HStringRef () : hstr (nullptr) {}
        // String ref doesn't need dtor

        template<size_t N>
        HRESULT Set (const wchar_t (&str)[N])
        {
            return WinRT::WindowsCreateStringReference (str, N - 1, &str_header, &hstr);
        }

        operator HSTRING() const { return hstr; }
    };

    /// Call RoActivateInstance and query an interface
    template<typename IF>
    static HRESULT ActivateInstance (HSTRING classId, ComPtr<IF>& instance)
    {
        ComPtr<IInspectable> inspectable;
        CHECKED (WinRT::RoActivateInstance (classId, &inspectable));
        return inspectable.As (&instance);
    }
}

static inline RGBA ToRGBA (WindowsUI::Color color)
{
    return RGB (color.R, color.G, color.B) | ((color.A) << 24);
}

HRESULT GetAccentColor (AccentColor& color)
{
    HStringRef classId;
    CHECKED(classId.Set (L"Windows.UI.ViewManagement.UISettings"));
    ComPtr<WindowsUI::ViewManagement::IUISettings> settings;
    CHECKED (ActivateInstance (classId, settings));

#if !defined(____x_ABI_CWindows_CUI_CViewManagement_CIUISettings3_INTERFACE_DEFINED__)
    #pragma message("WARNING: Windows 10 SDK not present. GetWindows10AccentColor() will always fail at run time.")
    return E_NOTIMPL;
#else
    ComPtr<WindowsUI::ViewManagement::IUISettings3> settings3;
    CHECKED(settings.As (&settings3));
    if (!settings3) return E_FAIL;

    WindowsUI::Color ui_color;
    CHECKED(settings3->GetColorValue (WindowsUI::ViewManagement::UIColorType_AccentDark3, &ui_color));
    color.darkest = ToRGBA (ui_color);
    CHECKED(settings3->GetColorValue (WindowsUI::ViewManagement::UIColorType_AccentDark2, &ui_color));
    color.darker = ToRGBA (ui_color);
    CHECKED(settings3->GetColorValue (WindowsUI::ViewManagement::UIColorType_AccentDark1, &ui_color));
    color.dark = ToRGBA (ui_color);
    CHECKED(settings3->GetColorValue (WindowsUI::ViewManagement::UIColorType_Accent, &ui_color));
    color.accent = ToRGBA (ui_color);
    CHECKED(settings3->GetColorValue (WindowsUI::ViewManagement::UIColorType_AccentLight1, &ui_color));
    color.light = ToRGBA (ui_color);
    CHECKED(settings3->GetColorValue (WindowsUI::ViewManagement::UIColorType_AccentLight2, &ui_color));
    color.lighter = ToRGBA (ui_color);
    CHECKED(settings3->GetColorValue (WindowsUI::ViewManagement::UIColorType_AccentLight3, &ui_color));
    color.lightest = ToRGBA (ui_color);

    return S_OK;
#endif
}

struct DwmColors
{
    RGBA ColorizationColor;
    int ColorizationColorBalance;
};

template<typename T>
static LONG QueryFromDWORD (HKEY key, const wchar_t* value, T& dest)
{
    DWORD v = 0;
    DWORD dataSize = sizeof (v);
    LONG result = RegGetValueW (key, nullptr, value, RRF_RT_REG_DWORD, nullptr, &v, &dataSize);
    if (result == ERROR_SUCCESS)
    {
        dest = static_cast<T> (v);
    }
    return result;
}

/* Obtain DWM colors from Registry, via undocumented keys.
   Although there's also an API to get these, it's undocumented as well... */
static HRESULT GetDwmColors (DwmColors& colors)
{
    HKEY keyDWM;
    LONG result = RegOpenKeyExW (HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\DWM", 0, KEY_READ, &keyDWM);
    if (result != ERROR_SUCCESS) return HRESULT_FROM_WIN32 (result);

    HRESULT hr = S_OK;
    DWORD c;
    result = QueryFromDWORD (keyDWM, L"ColorizationColor", c);
    if (result == ERROR_SUCCESS)
    {
        // Stored in the registry as BGRA
        colors.ColorizationColor =
            RGB ((c >> 16) & 0xff, (c >> 8) & 0xff, c & 0xff)
            | (c & (0xff << 24));
    }
    else
    {
        hr = HRESULT_FROM_WIN32 (result);
    }
    result = QueryFromDWORD (keyDWM, L"ColorizationColorBalance", colors.ColorizationColorBalance);
    if (result != ERROR_SUCCESS) hr = HRESULT_FROM_WIN32 (result);

    RegCloseKey (keyDWM);
    return hr;
}

static RGBA BlendRGBA (RGBA a, RGBA b, float f)
{
    float a_factor = 1.f - f;
    float b_factor = f;
    BYTE alpha_a = (a >> 24) & 0xff;
    BYTE alpha_b = (b >> 24) & 0xff;
    return RGB (static_cast<int> (GetRValue (a)* a_factor + GetRValue (b)*b_factor),
                static_cast<int> (GetGValue (a)* a_factor + GetGValue (b)*b_factor),
                static_cast<int> (GetBValue (a)* a_factor + GetBValue (b)*b_factor))
        | (static_cast<int> (alpha_a* a_factor + alpha_b*b_factor) << 24);
}

static HRESULT GetSystemFrameColors (FrameColors& color)
{
    color.activeCaptionBG = GetSysColor (COLOR_ACTIVECAPTION) | 0xff000000;
    color.activeCaptionText = GetSysColor (COLOR_CAPTIONTEXT) | 0xff000000;
    color.activeFrame = GetSysColor (COLOR_CAPTIONTEXT) | 0xff000000;
    color.inactiveCaptionBG = GetSysColor (COLOR_INACTIVECAPTION) | 0xff000000;
    RGBA rawInactiveCaptionText = GetSysColor (COLOR_INACTIVECAPTIONTEXT) | 0xff000000;
    color.inactiveFrame = GetSysColor (COLOR_INACTIVECAPTIONTEXT) | 0xff000000;
    color.inactiveCaptionText = BlendRGBA (rawInactiveCaptionText, color.inactiveCaptionBG, 0.6f);

    return S_OK;
}

namespace
{
    // RAII-ish wrapper for HKEYs
    class HKEYWrapper
    {
        HKEY key;
        void Close ()
        {
            if (key)
            {
                RegCloseKey (key);
                key = NULL;
            }
        }
    public:
        HKEYWrapper() : key (NULL) {}
        ~HKEYWrapper () { Close (); }

        operator HKEY const() { return key; }
        HKEY* operator&() { return &key; }
    };
}

// Returns whether title bars are colored with the accent color (Windows 10)
static bool ColoredTitleBars ()
{
    // Key on Windows 10 version 1607
    {
        HKEYWrapper keyDWM;
        LONG result = RegOpenKeyExW (HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\DWM",
                                     0, KEY_READ, &keyDWM);
        if (result == ERROR_SUCCESS)
        {
            DWORD prevalenceFlag = 0;
            if (QueryFromDWORD (keyDWM, L"ColorPrevalence", prevalenceFlag) == ERROR_SUCCESS)
                return prevalenceFlag != 0;
        }
    }
    // Key on Windows 10 version 1511. After 1607 this is the start/taskbar colorization only
    {
        HKEYWrapper keyPersonalize;
        LONG result = RegOpenKeyExW (HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                                     0, KEY_READ, &keyPersonalize);
        if (result == ERROR_SUCCESS)
        {
            DWORD prevalenceFlag = 0;
            if (QueryFromDWORD (keyPersonalize, L"ColorPrevalence", prevalenceFlag) == ERROR_SUCCESS)
                return prevalenceFlag != 0;
        }
    }
    return false;
}

static HRESULT GetAccentedFrameColors (FrameColors& color, bool glassEffect)
{
    HRESULT hr;

    bool useAccentColor = ColoredTitleBars();
    AccentColor ac;
    hr = GetAccentColor (ac);
    if (FAILED (hr)) return hr;

    const RGBA glassFillColor = 0xffffffff; // Effective color; maybe "transparent" is more sensible?

    if (glassEffect)
    {
        color.activeCaptionBG = glassFillColor;
    }
    else if (useAccentColor)
    {
        color.activeCaptionBG = ac.accent;
    }
    else
    {
        color.activeCaptionBG = 0xffffffff;
    }
    // Formula is documented here: https://docs.microsoft.com/en-us/windows/uwp/design/style/color
    bool textIsBright = (GetRValue (color.activeCaptionBG) * 2 + GetGValue (color.activeCaptionBG) * 5 + GetBValue (color.activeCaptionBG)) <= 1024;
    color.activeCaptionText = textIsBright ? 0xffffffff : 0xff000000; // Colors seem static

    if (glassEffect)
    {
        color.activeFrame = glassFillColor;
    }
    else
    {
        DwmColors dwmColors;
        if (SUCCEEDED (GetDwmColors (dwmColors)))
        {
            const RGBA activeFrameBaseColor = 0xffd9d9d9;
            // Frame color is based on DWM colors, though those usually coincide or are based on the accent color
            color.activeFrame = BlendRGBA (activeFrameBaseColor,
                                            dwmColors.ColorizationColor | 0xff000000,
                                            dwmColors.ColorizationColorBalance * 0.01f);
        }
        else
        {
            // Fallback
            color.activeFrame = ac.accent;
        }
    }

    color.inactiveCaptionBG = 0xffffffff;
    RGBA rawInactiveCaptionText = 0xff000000;
    // inactive frame: Probably a 0.5 blend of 0xffaaaaaa and 0. Maybe 0xffaaaaaa is itself a blend.
    color.inactiveFrame = 0x7f565656;
    color.inactiveCaptionText = BlendRGBA (rawInactiveCaptionText, color.inactiveCaptionBG, 0.6f);

    return S_OK;
}

HRESULT GetFrameColors (FrameColors& color, bool glassEffect)
{
    // High contrast colors -> use GetSysColors
    HIGHCONTRAST hc = { sizeof (HIGHCONTRAST) };
    bool use_sys_colors = SystemParametersInfo (SPI_GETHIGHCONTRAST, sizeof (HIGHCONTRAST), &hc, 0)
        && ((hc.dwFlags & HCF_HIGHCONTRASTON) != 0);

    if (use_sys_colors)
        return GetSystemFrameColors (color);

    return GetAccentedFrameColors (color, glassEffect);
}

} // namespace windows10colors
