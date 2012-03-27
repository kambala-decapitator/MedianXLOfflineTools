#ifndef WINDOWSINCLUDES_H
#define WINDOWSINCLUDES_H

#include <sdkddkver.h>

#if defined(NTDDI_VISTA) || defined(_WIN32_WINNT_VISTA)
#    define WIN_VISTA_OR_LATER
#    if defined(NTDDI_WIN7) || defined(_WIN32_WINNT_WIN7)
#        define WIN_7_OR_LATER
#    endif
#endif

#include <Shlobj.h>
#ifdef WIN_VISTA_OR_LATER
#include <Shobjidl.h>
#endif

#endif // WINDOWSINCLUDES_H
