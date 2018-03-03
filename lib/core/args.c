#include "scc/core/args.h"
#include "scc/core/string.h"
#include "scc/core/file.h"
#include <stdlib.h> // strtoi
#include <stdio.h> // strtoi

extern void arg_handler_init(arg_handler* self,
        const char* prefix, void(*cb)(void*, aparser*), void* data)
{
        self->_prefix = prefix;
        self->_cb = cb;
        self->_data = data;
}

extern void aparser_init(aparser* self, int argc, const char** argv)
{
        self->_argv = argv;
        self->_argc = argc;
        self->_pos = 1;
}

extern void aparse(aparser* self, arg_handler* handlers, size_t nhandlers, arg_handler* def)
{
        while (self->_pos < self->_argc)
        {
                const char* arg = self->_argv[self->_pos++];

                arg_handler* handler = NULL;
                for (size_t i = 0; i < nhandlers; i++)
                {
                        const char* prefix = handlers[i]._prefix;
                        if (*prefix && strncmp(arg, prefix, strlen(prefix)) == 0)
                        {
                                handler = handlers + i;
                                break;
                        }
                }

                if (!handler)
                {
                        handler = def;
                        self->_pos--;
                }

                handler->_cb(handler->_data, self);
        }
}

extern int aparser_args_remain(const aparser* self)
{
        int n = self->_argc - self->_pos;
        return n < 0 ? 0 : n;
}

extern const char* aparser_get_string(aparser* self)
{
        return aparser_args_remain(self)
                ? self->_argv[self->_pos++]
                : NULL;
}

extern errcode aparser_get_int(aparser* self, int* pint)
{
        const char* num = aparser_get_string(self);
        if (!num)
                return EC_ERROR;

        *pint = atoi(num);
        return EC_NO_ERROR;
}

typedef struct
{
        int total_len;
        int actual_len;
        int max_len;
        char* buffer;
} cmd_data;

static void cmd_data_init(cmd_data* self, char* buffer, size_t buffer_size)
{
        self->buffer = buffer;
        self->total_len = 0;
        self->actual_len = 0;
        self->max_len = buffer_size ? (int)(buffer_size - 1) : 0;
}

static void cmd_data_append(cmd_data* self, int c)
{
        int rem = self->max_len - self->actual_len;
        assert(rem >= 0);

        self->total_len++;

        if (rem)
        {
                self->actual_len++;
                self->buffer[self->actual_len - 1] = c;
        }

        self->buffer[self->actual_len] = '\0';
}

static void _arg_to_cmd(cmd_data* cmd, const char* arg)
{
#if OS_WIN

        const char* quote_pos = strchr(arg, '"');
        const char* space_pos = strchr(arg, ' ');
        bool has_spaces = quote_pos
                ? space_pos && space_pos < quote_pos
                : space_pos != NULL;
        if (has_spaces)
                cmd_data_append(cmd, '"');
        while (*arg)
                cmd_data_append(cmd, *arg++);
        if (has_spaces)
                cmd_data_append(cmd, '"');

#elif OS_OSX
        while (*arg)
        {
                int c = *arg++;
                if (!c)
                        break;

                if (c == ' ')
                        cmd_data_append(cmd, '\\');
                cmd_data_append(cmd, c);
        }
#else
#error todo
#endif
}

extern int arg_to_cmd(char* buffer, size_t buffer_size, const char* arg)
{
        cmd_data cmd;
        cmd_data_init(&cmd, buffer, buffer_size);
        _arg_to_cmd(&cmd, arg);
        return cmd.total_len;
}

static int _argv_to_cmd(cmd_data* cmd, const char* first, int argc, const char** argv)
{
#if OS_WIN
        cmd_data_append(cmd, '"');
#endif

        if (first)
        {
                _arg_to_cmd(cmd, first);
                if (argc > 0)
                        cmd_data_append(cmd, ' ');
        }

        for (int i = 0; i < argc; i++)
        {
                _arg_to_cmd(cmd, argv[i]);
                if (i + 1 != argc)
                        cmd_data_append(cmd, ' ');
        }

#if OS_WIN
        cmd_data_append(cmd, '"');
#endif

        return cmd->total_len;
}

extern int argv_to_cmd(char* buffer, size_t buffer_size, int argc, const char** argv)
{
        cmd_data cmd;
        cmd_data_init(&cmd, buffer, buffer_size);
        return _argv_to_cmd(&cmd, NULL, argc, argv);
}

extern errcode execute(const char* path, int* code, int argc, const char** argv)
{
        char cmd[MAX_CMD_SIZE];

        cmd_data data;
        cmd_data_init(&data, cmd, MAX_CMD_SIZE);

        if (_argv_to_cmd(&data, path, argc, argv) > MAX_CMD_SIZE)
                return EC_ERROR;

        int exit_code = system(cmd);
        if (code)
                *code = exit_code;

        return EC_NO_ERROR;       
}