#ifndef SCC_CORE_CMD_H
#define SCC_CORE_CMD_H

#include "common.h"

typedef struct _cmd_parser cmd_parser;

typedef struct
{
        const char* prefix;
        void(*oncmd)(void*, cmd_parser*);
        void* data;
} cmd_handler;

#define CMD_HANDLER_INIT(PREFIX, CB, DATA) \
        { PREFIX, ((void(*)(void*, cmd_parser*))CB), DATA }

extern void cmd_handler_init(cmd_handler* self,
        const char* prefix, void(*oncmd)(void*, cmd_parser*), void* data);

typedef struct _cmd_parser
{
        int argc;
        int pos;
        const char** argv;
} cmd_parser;

extern void cmd_parser_init(cmd_parser* self, int argc, const char** argv);
                
extern void cmd_parser_run(
        cmd_parser* self, cmd_handler* handlers, size_t nhandlers, cmd_handler* def);
extern int cmd_parser_cmds_remain(const cmd_parser* self);
extern const char* cmd_parser_get_string(cmd_parser* self);
extern errcode cmd_parser_get_int(cmd_parser* self, int* result);

extern int arg_to_cmd(char* buffer, size_t buffer_size, const char* arg);
extern int argv_to_cmd(char* buffer, size_t buffer_size, int argc, const char** argv);

extern errcode execute(const char* path, int* code, int argc, const char** argv);

#endif
