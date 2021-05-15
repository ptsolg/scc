#include "scc/core/cmd.h"
#include "scc/core/file.h"
#include <stdlib.h> // strtoi
#include <string.h>

void init_arg_parser(struct arg_parser* self, int argc, const char** argv)
{
        self->argc = argc;
        self->pos = 1;
        self->argv = argv;
        self->merged_arg = NULL;
}

static int remaining(const struct arg_parser* self)
{
        int n = self->argc - self->pos;
        return n < 0 ? 0 : n;
}

const char* arg_parser_next_str(struct arg_parser* self)
{
        if (self->merged_arg)
        {
                const char* s = self->merged_arg;
                self->merged_arg = NULL;
                return s;
        }
        return remaining(self) ? self->argv[self->pos++] : NULL;
}

int* arg_parser_next_int(struct arg_parser* self, int* result)
{
        const char* num = arg_parser_next_str(self);
        if (!num)
                return 0;
        *result = atoi(num);
        return result;
}

void run_arg_parser(struct arg_parser* self,
        const struct arg_handler* handlers,
        unsigned num_handlers,
        const struct arg_handler* unknown)
{
        while (self->pos < self->argc)
        {
                const char* arg = self->argv[self->pos++];
                size_t arg_len = strlen(arg);
                const struct arg_handler* handler = NULL;

                for (unsigned i = 0; i < num_handlers; i++)
                {
                        const char* prefix = handlers[i].prefix;
                        size_t prefix_len = strlen(prefix);

                        if (*prefix && strncmp(arg, prefix, prefix_len) == 0)
                        {
                                handler = handlers + i;
                                if (prefix_len != arg_len)
                                        self->merged_arg = arg + prefix_len;
                                break;
                        }
                }

                if (!handler)
                {
                        handler = unknown;
                        self->pos--;
                }

                handler->fn(self);
        }
}

#define MAX_CMD_SIZE 1024

struct cmdbuf
{
        char* pos;
        char buf[MAX_CMD_SIZE + 1];
};

static void append_cmd(struct cmdbuf* buf, const char* arg)
{
        unsigned rem = MAX_CMD_SIZE - (buf->pos - buf->buf);
        unsigned len = strlen(arg);
        const char* quote_pos = strchr(arg, '"');
        const char* space_pos = strchr(arg, ' ');
        bool has_spaces = quote_pos
                ? space_pos && space_pos < quote_pos
                : space_pos != NULL;
        buf->pos += snprintf(buf->pos, rem, (has_spaces ? "\"%s\" " : "%s "), arg);
}

int execute(const char* path, int argc, const char** argv)
{
        struct cmdbuf cmd;
        cmd.pos = cmd.buf;
        append_cmd(&cmd, path);
        for (int i = 0; i < argc; i++)
                append_cmd(&cmd, argv[i]);
        return system(cmd.buf);
}
