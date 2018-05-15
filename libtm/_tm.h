#ifndef _TM_H
#define _TM_H

#if __SCC__
#define thread_local _Thread_local
#endif

extern thread_local unsigned _tm_transaction_id;

extern int _tm_read(const void* source, void* dest, unsigned n);
extern void _tm_write(const void* source, void* dest, unsigned n);

struct _tm_transaction
{
        unsigned readset_pos;
        unsigned writeset_pos;
        unsigned locks_acquired;
        unsigned read_version;
};

extern void _tm_transaction_init(struct _tm_transaction* self);
extern int _tm_transaction_commit(struct _tm_transaction* self);
extern void _tm_transaction_reset(struct _tm_transaction* self);
extern void _tm_transaction_cancel(struct _tm_transaction* self);

#endif