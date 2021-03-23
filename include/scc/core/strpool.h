#ifndef STRPOOL_H
#define STRPOOL_H

#include "allocator.h"
#include "hashmap.h"

struct strentry
{
        size_t size;
        char data[];
};

struct strpool
{
        struct hashmap map;
        struct stack_alloc alloc;
};

void init_strpool(struct strpool* self);
void drop_strpool(struct strpool* self);
int strpool_has(const struct strpool* self, unsigned ref);
unsigned strpool_insert(struct strpool* self, const void* data, size_t size);
struct strentry* strpool_lookup(const struct strpool* self, unsigned ref);

#endif
