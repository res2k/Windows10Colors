#include "targetver.h"
#include "Windows10Colors.h"

#include <iostream>
#include <string>

static std::string output_RGBA(windows10colors::RGBA rgba)
{
    char buf[19];
    snprintf(buf, sizeof(buf), "%d, %d, %d, %d", rgba & 0xff, (rgba >> 8) & 0xff, (rgba >> 16) & 0xff, (rgba >> 24) & 0xff);
    return buf;
}

static std::string output_HRESULT(HRESULT hr)
{
    if(hr == S_OK)
        return {};

    char buf[19];
    snprintf(buf, sizeof(buf), "result: 0x%08x", hr);
    return buf;
}

static std::ostream& operator<<(std::ostream& stream, windows10colors::SysPartsMode sys_parts)
{
    switch(sys_parts)
    {
    case windows10colors::SysPartsMode::AccentColor:
        stream << "AccentColor";
        break;
    case windows10colors::SysPartsMode::Dark:
        stream << "Dark";
        break;
    case windows10colors::SysPartsMode::Light:
        stream << "Light";
        break;
    }
    return stream;
}

int main()
{
    CoInitializeEx (nullptr, COINIT_APARTMENTTHREADED);

    std::cout << std::boolalpha;

    windows10colors::AccentColor ac;
    HRESULT hr = windows10colors::GetAccentColor(ac);
    std::cout << "Accent color:  " << output_RGBA(ac.accent) << " " << output_HRESULT(hr) << std::endl;

    bool dark;
    hr = windows10colors::GetAppDarkModeEnabled(dark);
    std::cout << "App dark mode: " << dark << " " << output_HRESULT(hr) << std::endl;
    hr = windows10colors::GetSysPartsDarkModeEnabled(dark);
    std::cout << "Sys dark mode: " << dark << " " << output_HRESULT(hr) << std::endl;
    windows10colors::SysPartsMode sys_parts;
    hr = windows10colors::GetSysPartsMode(sys_parts);
    std::cout << "Sys parts:     " << sys_parts << " " << output_HRESULT(hr) << std::endl;

    return 0;
}

