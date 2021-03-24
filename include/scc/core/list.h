#ifndef LIST_H
#define LIST_H

struct list
{
        struct list* next, * prev;
};

#define LIST_FOREACH(PLIST, ITTYPE, ITNAME) \
        for (ITTYPE ITNAME = (ITTYPE)((const struct list*)PLIST)->next;\
                ITNAME != (ITTYPE)((const struct list*)PLIST);\
                ITNAME = (ITTYPE)((const struct list*)ITNAME)->next)

static void init_list(struct list* self)
{
        self->next = self;
        self->prev = self;
}

static int is_list_empty(const struct list* self)
{
        return self->next == self;
}

static struct list* list_remove(struct list* self) 
{
        self->prev->next = self->next;
        self->next->prev = self->prev;
        return self;
}

static void list_push(struct list* self, struct list* other)
{
        struct list* tail = self->prev;
        other->prev = tail;
        tail->next = other;
        self->prev = other;
        other->next = self;
}

static struct list* list_pop(struct list* self) 
{
        return list_remove(self->prev);
}

static void list_shift(struct list* self, struct list* other) 
{
        struct list* head = self->next;
        other->next = head;
        head->prev = other;
        self->next = other;
        other->prev = self;
}

static struct list* list_unshift(struct list* self) 
{
        return list_remove(self->next);
}

static void list_append(struct list* self, struct list* other) 
{
        if (is_list_empty(other))
                return;
        struct list* head = other->next;
        struct list* tail = other->prev;
        self->prev->next = head;
        head->prev = self->prev;
        tail->next = self;
        self->prev = tail;
        init_list(other);
}

#endif
