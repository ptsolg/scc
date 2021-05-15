#ifndef WINDOWS_H
#define WINDOWS_H

#pragma link "kernel32.lib"

typedef void* HANDLE;
typedef void* LPSECURITY_ATTRIBUTES;
typedef unsigned DWORD;
typedef DWORD* LPDWORD;
typedef void* LPVOID;
typedef DWORD (*PTHREAD_START_ROUTINE)(LPVOID) _Stdcall;
typedef PTHREAD_START_ROUTINE LPTHREAD_START_ROUTINE;
typedef char CHAR;
typedef CHAR* LPSTR;
typedef const CHAR* LPCSTR;
typedef long LONG;
typedef long long LONGLONG;
typedef int BOOL;

#if _WIN32
typedef unsigned SIZE_T;
typedef long LONG_PTR;
#else
typedef unsigned long long SIZE_T;
typedef long long LONG_PTR;
#endif

extern _Dllimport
HANDLE CreateThread(
        LPSECURITY_ATTRIBUTES atts,
        SIZE_T stackSize,
        LPTHREAD_START_ROUTINE entry,
        LPVOID param,
        DWORD flags,
        LPDWORD threadId) _Stdcall;

#define INFINITE 0xFFFFFFFF

extern _Dllimport DWORD WaitForSingleObject(HANDLE handle, DWORD ms) _Stdcall;

extern _Dllimport void ExitThread(DWORD code) _Stdcall;

#define GetCurrentDirectory GetCurrentDirectoryA
extern _Dllimport DWORD GetCurrentDirectoryA(DWORD nBufferLength, LPSTR lpBuffer) _Stdcall;

#define GetFullPathName GetFullPathNameA
extern _Dllimport
DWORD GetFullPathNameA(
        LPCSTR lpFileName,
        DWORD nBufferLength,
        LPSTR lpBuffer,
        LPSTR* lpFilePart) _Stdcall;

#define GetFileAttributes GetFileAttributesA
extern _Dllimport DWORD GetFileAttributesA(LPCSTR lpFileName) _Stdcall;

#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
#define FILE_ATTRIBUTE_DIRECTORY 0x00000010

#define CreateFile CreateFileA
extern _Dllimport
HANDLE CreateFileA(
        LPCSTR lpFileName,
        DWORD dwDesiredAccess,
        DWORD dwShareMode,
        LPSECURITY_ATTRIBUTES lpSecurityAttributes,
        DWORD dwCreationDisposition,
        DWORD dwFlagsAndAttributes,
        HANDLE hTemplateFile) _Stdcall;

#define GENERIC_READ 0x80000000L
#define FILE_SHARE_READ 0x00000001
#define OPEN_EXISTING 3
#define FILE_ATTRIBUTE_READONLY 0x00000001
#define INVALID_HANDLE_VALUE ((HANDLE)(LONG_PTR)-1)

typedef union _LARGE_INTEGER
{
        struct
        {
                DWORD LowPart;
                LONG HighPart;
        } DUMMYSTRUCTNAME;

        struct
        {
                DWORD LowPart;
                LONG HighPart;
        } u;

        LONGLONG QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;

extern _Dllimport BOOL GetFileSizeEx(HANDLE hFile, PLARGE_INTEGER lpFileSize) _Stdcall;

extern _Dllimport BOOL CloseHandle(HANDLE hObject) _Stdcall;

#define DeleteFile DeleteFileA
extern _Dllimport BOOL DeleteFileA(LPCSTR lpFileName) _Stdcall;

#endif
