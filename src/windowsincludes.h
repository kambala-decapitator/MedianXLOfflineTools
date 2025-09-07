#ifndef WINDOWSINCLUDES_H
#define WINDOWSINCLUDES_H

#include <Shlobj.h>
#include <sdkddkver.h>

#if NTDDI_VERSION >= NTDDI_VISTA
#  define WIN_VISTA_OR_LATER
#  include <Shobjidl.h>

#  if defined(NTDDI_WIN7) || defined(_WIN32_WINNT_WIN7)
#    define WIN_7_OR_LATER
#    include <Shellapi.h>
#  endif // 7
#endif // Vista

#define LPWSTR_TO_QSTRING(wstr) QString::fromUtf16(reinterpret_cast<const ushort *>(wstr))
#define QSTRING_TO_LPCWSTR(s) reinterpret_cast<LPCWSTR>(s.utf16())

#endif // WINDOWSINCLUDES_H
