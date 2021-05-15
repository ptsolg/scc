#ifndef SHLWAPI_H
#define SHLWAPI_H

#pragma link "shlwapi.lib"

typedef int BOOL;
typedef char CHAR;
typedef const CHAR* LPCSTR;

#define PathIsDirectory PathIsDirectoryA
extern _Dllimport BOOL PathIsDirectoryA(LPCSTR pszPath) _Stdcall;

#endif
