#ifndef SCC_CORE_LIST_H
#define SCC_CORE_LIST_H

#include "common.h"

typedef struct _list_node
{
        struct _list_node* next;
        struct _list_node* prev;
} list_node;

static inline void list_node_init(list_node* self)
{
        self->next = NULL;
        self->prev = NULL;
}

// inserts other node after self node
// if other node is already in list the behaviour is undefined
static inline void list_node_add_after(list_node* self, list_node* other)
{
        list_node* next = self->next;
        if (next)
                next->prev = other;

        other->next = next;
        other->prev = self;
        self->next = other;
}

// inserts other node before self node
// if other node is already in list the behaviour is undefined
static inline void list_node_add_before(list_node* self, list_node* other)
{
        list_node* prev = self->prev;
        if (prev)
                prev->next = other;

        other->prev = prev;
        other->next = self;
        self->prev = other;
}

static inline list_node* list_node_remove(list_node* self)
{
        self->prev->next = self->next;
        self->next->prev = self->prev;
        return self;
}

typedef struct _list_head
{
        union
        {
                list_node as_node;
                struct
                {
                        list_node* head;
                        list_node* tail;
                };
        };
} list_head;

static inline list_node* list_end(list_head* self)
{
        return &self->as_node;
}

static inline const list_node* list_end_c(const list_head* self)
{
        return &self->as_node;
}

static inline void list_init(list_head* self)
{
        self->head = list_end(self);
        self->tail = list_end(self);
}

static inline list_head list_create(list_node* first, list_node* last)
{
        list_head h = { first, last };
        return h;
}

static inline bool list_empty(const list_head* self)
{
        return self->head == &self->as_node;
}

static inline void list_push_back(list_head* self, list_node* node)
{
        list_node_add_before(list_end(self), node);
        if (list_empty(self))
        {
                self->head = node;
                node->prev = list_end(self);
        }
}

static inline void list_push_front(list_head* self, list_node* node)
{
        list_node_add_after(list_end(self), node);
        if (list_empty(self))
        {
                self->tail = node;
                node->next = list_end(self);
        }
}

static inline void list_push_back_list(list_head* self, list_head* other)
{
        if (list_empty(other))
                return;

        list_node* head = other->head;
        list_node* tail = other->tail;

        head->prev = self->tail;
        tail->next = list_end(self);
        self->tail->next = head;
        self->tail = tail;
        list_init(other);
}

static inline list_node* list_pop_back(list_head* self)
{
        return list_node_remove(self->tail);
}

static inline list_node* list_pop_front(list_head* self)
{
        return list_node_remove(self->head);
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
        for (ITTYPE ITNAME = (ITTYPE)((const list_head*)PLIST)->head; \
                ITNAME != (ITTYPE)list_end_c(((const list_head*)PLIST)); \
                ITNAME = (ITTYPE)((const list_node*)ITNAME)->next)

#endif
