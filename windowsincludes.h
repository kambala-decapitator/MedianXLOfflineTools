#ifndef WINDOWSINCLUDES_H
#define WINDOWSINCLUDES_H

#include <Shlobj.h>
#include <sdkddkver.h>

#if defined(NTDDI_VISTA) || defined(_WIN32_WINNT_VISTA)
#define WIN_VISTA_OR_LATER
#include <Shobjidl.h>
#if defined(NTDDI_WIN7) || defined(_WIN32_WINNT_WIN7)
#define WIN_7_OR_LATER
#include <Shellapi.h>
#endif // 7
#endif // Vista

#endif // WINDOWSINCLUDES_H
