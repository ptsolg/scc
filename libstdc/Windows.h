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

#if _WIN32
typedef unsigned SIZE_T;
#else
typedef unsigned long long SIZE_T;
#endif

extern _Dllimport HANDLE CreateThread(
	LPSECURITY_ATTRIBUTES atts,
	SIZE_T stackSize,
	LPTHREAD_START_ROUTINE entry,
	LPVOID param,
	DWORD flags,
	LPDWORD threadId) _Stdcall;

#define INFINITE 0xFFFFFFFF

extern _Dllimport DWORD WaitForSingleObject(HANDLE handle, DWORD ms) _Stdcall;

extern _Dllimport void ExitThread(DWORD code) _Stdcall;

#endif