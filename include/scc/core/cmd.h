#ifndef CMD_H
#define CMD_H

#include "common.h"

struct arg_handler
{
        const char* prefix;
        void(*fn)(void*);
};

#define ARG_HANDLER(P, FN) { (const char*)P, (void(*)(void*))FN }

struct arg_parser
{
        int pos;
        int argc;
        const char** argv;
        const char* merged_arg;
};

void init_arg_parser(struct arg_parser* self, int argc, const char** argv);
const char* arg_parser_next_str(struct arg_parser* self);
int* arg_parser_next_int(struct arg_parser* self, int* result);
void run_arg_parser(struct arg_parser* self,
        const struct arg_handler* handlers,
        unsigned num_handlers,
        const struct arg_handler* unknown);

int execute(const char* path, int argc, const char** argv);

#endif
