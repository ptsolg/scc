#include "scc/scl/file.h"
#include "scc/scl/sstring.h"
#include <stdio.h>

#if S_WIN
#include <Windows.h>
#include <FileApi.h>
#include <Shlwapi.h>

#if S_MSVC
#pragma comment(lib, "shlwapi.lib")
#else
#error //todo
#endif

#elif S_OSX
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h> // realpath
#else
#error todo
#endif

extern serrcode path_get_cd(char* path)
{
#if S_WIN
        if (!GetCurrentDirectory(S_MAX_PATH_LEN, path))
                return S_ERROR;
#elif S_OSX
        if (!getcwd(path, S_MAX_PATH_LEN))
                return S_ERROR;
#else
#error todo
#endif
        return path_add_trailing_slash(path);
}

extern serrcode path_add_trailing_slash(char* path)
{
        if (!path_has_trailing_slash(path))
        {
                ssize len = strlen(path);
                if (len + 1 >= S_MAX_PATH_LEN)
                        return S_ERROR;

                path[len++] = S_PATH_DELIMETER;
                path[len] = '\0';
        }
        return S_NO_ERROR;
}

extern serrcode path_join(char* path, const char* other)
{
        if (strlen(path) + strlen(other) >= S_MAX_PATH_LEN)
                return S_ERROR;

        if (!path_has_trailing_slash(path))
                if (S_FAILED(path_add_trailing_slash(path)))
                        return S_ERROR;

        strcat(path, other);
        return S_NO_ERROR;
}

extern void path_goto_parent_dir(char* path)
{
        char* file = path_get_file(path);
        if (file == path)
                return;
        *(file - 1) = '\0';
        path_strip_file(path);
}

extern bool path_has_trailing_slash(const char* path)
{
        ssize len = strlen(path);
        return len && path[len - 1] == S_PATH_DELIMETER;
}

extern void path_strip_file(char* path)
{
        char* file = path_get_file(path);
        *file = '\0';
}

extern bool path_is_dir(const char* path)
{
#if S_WIN
        return PathIsDirectory(path);
#elif S_OSX
        struct stat s;
        if (stat(path, &s) != 0)
                return false;
        return S_ISDIR(s.st_mode);
#else
#error
#endif
}

extern bool path_is_file(const char* path)
{
#if S_WIN
        DWORD att = GetFileAttributes(path);
        if (att == INVALID_FILE_ATTRIBUTES)
                return false;

        return !(att & FILE_ATTRIBUTE_DIRECTORY);
#elif S_OSX
        struct stat s;
        if (stat(path, &s) != 0)
                return false;
        return S_ISREG(s.st_mode);
#else
#error
#endif
}

extern bool path_is_valid(const char* path)
{
        return path_is_file(path) || path_is_dir(path);
}

extern char* path_get_file(char* path)
{
        char* end = sstrend(path);
        while (end != path)
        {
                end--;
                if (*end == S_PATH_DELIMETER)
                        return end + 1;
        }
        return end;
}

extern const char* path_get_cfile(const char* path)
{
        const char* end = scstrend(path);
        while (end != path)
        {
                end--;
                if (*end == S_PATH_DELIMETER)
                        return end + 1;
        }
        return end;
}

extern ssize path_get_size(const char* path)
{
        ssize res = 0;
#if S_WIN
        HANDLE h = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL,
                OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
        if (h != INVALID_HANDLE_VALUE)
        {
                LARGE_INTEGER i;
                if (GetFileSizeEx(h, &i))
                        res = (ssize)i.QuadPart;
                CloseHandle(h);
        }
        return res;

#elif S_OSX
        struct stat st;
        if (stat(path, &st) != 0)
                return 0;
        return st.st_size;
#else
#error todo
#endif
}

extern bool path_is_abs(const char* path)
{
#if S_WIN
        return !PathIsRelative(path);
#elif S_OSX
        return false;
#else
#error todo
#endif
}

extern void path_fix_delimeter(char* path)
{
        while(1)
        {
               int c = *path;
               if (!c)
                       return;

               if (c == '\\' || c == '/')
                       *path = S_PATH_DELIMETER;
               path++;
        }
}

extern serrcode path_get_abs(char* abs, const char* loc)
{
        if (path_is_abs(loc))
        {
                strcpy(abs, loc);
                return S_NO_ERROR;
        }

#if S_WIN
        if (!GetFullPathName(loc, S_MAX_PATH_LEN, abs, NULL))
                return S_ERROR;
#elif S_OSX
        if (!realpath(loc, abs))
                return S_ERROR;
#else
#error todo
#endif

        return S_NO_ERROR;
}

static ssize fread_cb_read(fread_cb* self, void* buf, ssize bytes)
{
        return fread(buf, 1, bytes, self->_in);
}

extern void fread_cb_init(fread_cb* self, FILE* in)
{
        self->_in = in;
        read_cb_init(&self->_base, &fread_cb_read);
}

static ssize fwrite_cb_write(fwrite_cb* self, const void* data, ssize bytes)
{
        return fwrite(data, 1, bytes, self->_out);
}

extern void fwrite_cb_init(fwrite_cb* self, FILE* out)
{
        self->_out = out;
        write_cb_init(&self->_base, &fwrite_cb_write);
}
