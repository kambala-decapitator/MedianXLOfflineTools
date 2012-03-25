#include "application.h"

#include <Windows.h>


void Application::activateWindowWinAPI()
{
    // maybe re-register window class instead of this hack?
    extern const QString qt_getRegisteredWndClass();
    const QString windowClassName = qt_getRegisteredWndClass(); // usually returns "QWidget"

    // thanks to Notepad++ sources for the idea
    HWND existingWindowHandle = NULL;
    for (int i = 0; i < 10; ++i)
    {
        HWND qtWindowHandle = ::FindWindow(windowClassName.utf16(), NULL);
        if (qtWindowHandle)
        {
            WCHAR captionWstr[100];
            if (::GetWindowText(qtWindowHandle, captionWstr, 100))
            {
                QString caption = QString::fromWCharArray(captionWstr);
                qDebug("found Qt window with caption %s", qPrintable(caption));
                if (caption.endsWith(appName))
                {
                    qDebug("this is our window");
                    existingWindowHandle = qtWindowHandle;
                    break;
                }
            }
            else
                qDebug("failed to get caption of window %#x: %u", qtWindowHandle, ::GetLastError());
        }
        else
        {
            qDebug("WTF?!?!?!");
            break;
        }
        Sleep(100);
    }

    if (existingWindowHandle)
    {
        ::ShowWindow(existingWindowHandle, ::IsIconic(existingWindowHandle) ? SW_RESTORE : SW_SHOW);
        ::SetForegroundWindow(existingWindowHandle);
    }
}
