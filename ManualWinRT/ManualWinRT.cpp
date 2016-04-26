// ManualWinRT.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "Windows10Colors.h"

#include <dwmapi.h>
#include <stdio.h>


int main()
{
    CoInitializeEx (nullptr, COINIT_APARTMENTTHREADED);

    windows10colors::AccentColor accent;
    HRESULT hr = windows10colors::GetAccentColor (accent);
    if (FAILED (hr))
    {
        printf ("error %0.8x\n", hr);
        return hr;
    }

    printf ("%d,%d,%d\n", GetRValue (accent.accent), GetGValue (accent.accent), GetBValue (accent.accent));

    {
        DWORD cc;
        BOOL co;
        if (SUCCEEDED (DwmGetColorizationColor (&cc, &co)))
        {
            for (int i = 0; i < 4; i++)
            {
                printf ("%d ", cc & 0xff);
                cc = cc >> 8;
            }
            printf (" %d\n", co);
        }
    }

    if (OpenClipboard (NULL))
    {
        char text[64];
        snprintf (text, sizeof(text), "%d\t%d\t%d", GetRValue (accent.light), GetGValue (accent.light), GetBValue (accent.light));
        size_t text_len = strlen (text);
        HGLOBAL hdata = GlobalAlloc (GMEM_MOVEABLE, text_len + 1);
        if (hdata)
        {
            {
                void* data_p = GlobalLock (hdata);
                memcpy (data_p, text, text_len + 1);
                GlobalUnlock (hdata);
            }
            EmptyClipboard ();
            SetClipboardData (CF_TEXT, hdata);
            CloseClipboard ();
        }
    }

    return 0;
}

