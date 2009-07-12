#ifdef VISTA
#include "htmlhelp.h"
#else
  #ifdef _MSC_VER
    #if _MSC_VER <= 1200
      #define DWORD_PTR DWORD
    #endif
  #endif
HWND WINAPI HtmlHelp( HWND hwnd, LPCSTR helpFile, UINT action, DWORD_PTR data );
#endif
int MyHelp(HWND hwnd, LPSTR helpFile, UINT action, DWORD_PTR data);
