#include "scc/core/cmd.h"
#include "scc/core/string.h"
#include "scc/core/file.h"
#include <stdlib.h> // strtoi
#include <stdio.h> // strtoi

extern void cmd_handler_init(cmd_handler* self,
        const char* prefix, void(*oncmd)(void*, cmd_parser*), void* data)
{
        self->prefix = prefix;
        self->oncmd = oncmd;
        self->data = data;
}

extern void cmd_parser_init(cmd_parser* self, int argc, const char** argv)
{
        self->argv = argv;
        self->argc = argc;
        self->pos = 1;
}

extern void cmd_parser_run(
        cmd_parser* self, cmd_handler* handlers, size_t nhandlers, cmd_handler* def)
{
        while (self->pos < self->argc)
        {
                const char* arg = self->argv[self->pos++];

                cmd_handler* handler = NULL;
                for (size_t i = 0; i < nhandlers; i++)
                {
                        const char* prefix = handlers[i].prefix;
                        if (*prefix && strncmp(arg, prefix, strlen(prefix)) == 0)
                        {
                                handler = handlers + i;
                                break;
                        }
                }

                if (!handler)
                {
                        handler = def;
                        self->pos--;
                }

                handler->oncmd(handler->data, self);
        }
}

extern int cmd_parser_cmds_remain(const cmd_parser* self)
{
        int n = self->argc - self->pos;
        return n < 0 ? 0 : n;
}

extern const char* cmd_parser_get_string(cmd_parser* self)
{
        return cmd_parser_cmds_remain(self) ? self->argv[self->pos++] : NULL;
}

extern errcode cmd_parser_get_int(cmd_parser* self, int* result)
{
        const char* num = cmd_parser_get_string(self);
        if (!num)
                return EC_ERROR;

        *result = atoi(num);
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