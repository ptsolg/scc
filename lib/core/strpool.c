#include "scc/core/strpool.h"

#include "scc/core/hash.h"

#include <string.h> // memcmp

static struct strentry empty;

void init_strpool(struct strpool* self)
{
        hashmap_init(&self->map);
        init_stack_alloc(&self->alloc);
}

void drop_strpool(struct strpool* self)
{
        hashmap_drop(&self->map);
        drop_stack_alloc(&self->alloc);
}

int strpool_has(const struct strpool* self, unsigned ref)
{
        return !ref ? 1 : hashmap_has(&self->map, ref);
}

unsigned strpool_insert(struct strpool* self, const void* data, size_t size)
{
        if (!size)
                return 0;
        unsigned ref = hash(data, size);
        for (unsigned i = 1; ; i++) {
                struct strentry* entry = strpool_lookup(self, ref);
                if (entry && entry->size == size && memcmp(entry->data, data, size) == 0)
                        return ref;
                else if (hashmap_key_ok(ref) && ref != 0)
                        break;
                ref += i;
        }

        struct strentry* copy = stack_alloc(&self->alloc, sizeof(struct strentry) + size);
        copy->size = size;
        memcpy(copy->data, data, size);
        hashmap_insert(&self->map, ref, copy);
        return ref;
}

struct strentry* strpool_lookup(const struct strpool* self, unsigned ref)
{
        if (!ref)
                return &empty;
        struct hashmap_entry* e = hashmap_lookup(&self->map, ref);
        return e ? e->value : 0;
}
