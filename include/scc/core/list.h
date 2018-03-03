#ifndef SCC_CORE_LIST_H
#define SCC_CORE_LIST_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

typedef struct _list_node
{
        struct _list_node* _next;
        struct _list_node* _prev;
} list_node;

static inline list_node* list_node_next(const list_node* self)
{
        return self->_next;
}

static inline list_node* list_node_prev(const list_node* self)
{
        return self->_prev;
}

static inline void list_node_init(list_node* self)
{
        self->_next = NULL;
        self->_prev = NULL;
}

// inserts other node after self node
// if other node is already in list the behaviour is undefined
static inline void list_node_add_after(list_node* self, list_node* other)
{
        list_node* next = self->_next;
        if (next)
                next->_prev = other;

        other->_next = next;
        other->_prev = self;
        self->_next = other;
}

// inserts other node before self node
// if other node is already in list the behaviour is undefined
static inline void list_node_add_before(list_node* self, list_node* other)
{
        list_node* prev = self->_prev;
        if (prev)
                prev->_next = other;

        other->_prev = prev;
        other->_next = self;
        self->_prev = other;
}

static inline list_node* list_node_remove(list_node* self)
{
        self->_prev->_next = self->_next;
        self->_next->_prev = self->_prev;
        return self;
}

typedef struct _list_head
{
        union
        {
                list_node _base;
                struct
                {
                        list_node* _first;
                        list_node* _last;
                };
        };
} list_head;

static inline list_node* list_base(list_head* self)
{
        return &self->_base;
}

static inline const list_node* list_cbase(const list_head* self)
{
        return &self->_base;
}

static inline list_node* list_begin(const list_head* self)
{
        return self->_first;
}

static inline list_node* list_last(const list_head* self)
{
        return self->_last;
}

static inline list_node* list_end(list_head* self)
{
        return list_base(self);
}

static inline const list_node* list_cend(const list_head* self)
{
        return list_cbase(self);
}

static inline void list_init(list_head* self)
{
        self->_first = list_end(self);
        self->_last = list_end(self);
}

static inline list_head list_create(list_node* first, list_node* last)
{
        list_head h = { first, last };
        return h;
}

static inline bool list_empty(const list_head* self)
{
        return list_begin(self) == list_cend(self);
}

static inline void list_push_back(list_head* self, list_node* node)
{
        list_node_add_before(list_end(self), node);
        if (list_empty(self))
        {
                self->_first = node;
                node->_prev = list_end(self);
        }
}

static inline void list_push_front(list_head* self, list_node* node)
{
        list_node_add_after(list_end(self), node);
        if (list_empty(self))
        {
                self->_last = node;
                node->_next = list_end(self);
        }
}

static inline void list_push_back_list(list_head* self, list_head* other)
{
        if (list_empty(other))
                return;

        list_node* first = list_begin(other);
        list_node* last = list_last(other);

        first->_prev = self->_last;
        last->_next = list_end(self);
        self->_last->_next = first;
        self->_last = last;
}

static inline list_node* list_pop_back(list_head* self)
{
        return list_node_remove(self->_last);
}

static inline list_node* list_pop_front(list_head* self)
{
        return list_node_remove(self->_first);
}

static inline void list_init_array(list_head* self, list_node nodes[], size_t count)
{
        list_init(self);
        for (size_t i = 0; i < count; i++)
        {
                list_node_init(nodes + i);
                list_push_back(self, nodes + i);
        }
}

#define LIST_FOREACH(PLIST, ITTYPE, ITNAME) \
        for (ITTYPE ITNAME = (ITTYPE)list_begin(((const list_head*)PLIST)); \
                ITNAME != (ITTYPE)list_cend(((const list_head*)PLIST)); \
                ITNAME = (ITTYPE)list_node_next(((const list_node*)ITNAME)))

#ifdef __cplusplus
}
#endif

#endif
