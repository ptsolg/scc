#ifndef SCC_CORE_READ_WRITE_H
#define SCC_CORE_READ_WRITE_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

typedef struct _write_cb
{ 
        // takes itself as first argument, returns bytes written
        ssize(*_write)(void*, const void*, ssize);
} write_cb;

extern void write_cb_init(write_cb* self, void* write_fn);

#ifndef WB_SIZE
#define WB_SIZE 4095
#endif

// Struct that reduces write callbacks by writing data to buffer and then calling a function
typedef struct _writebuf
{
        write_cb* _cb;
        char* _pos;
        char _data[WB_SIZE];
        ssize _written;
} writebuf;

extern void writebuf_init(writebuf* self, write_cb* cb);
extern void writebuf_dispose(writebuf* self);

// note: functions below return bytes written, it means that any of these functions
// can return 0 (since data can be written to the buffer)
// or the amount of bytes flushed (if the buffer is full)

extern ssize writebuf_flush(writebuf* self);
extern ssize writebuf_write_bytes(writebuf* self, const void* b, ssize len);
extern ssize writebuf_writes(writebuf* self, const char* s);
extern ssize writebuf_writec(writebuf* self, int c);

// returns total bytes written
static inline ssize writebuf_get_bytes_written(const writebuf* self)
{
        return self->_written;
}

typedef struct _snwrite_cb
{
        write_cb _base;
        char* _pos;
        char* _begin;
        ssize _n;
} snwrite_cb;

extern void snwrite_cb_init(snwrite_cb* self, char* buf, ssize n);

static inline write_cb* snwrite_cb_base(snwrite_cb* self)
{
        return &self->_base;
}

#ifndef RB_SIZE
#define RB_SIZE 4095
#endif

typedef struct _read_cb
{
        // takes itself as first argument, returns bytes read
        ssize(*_read)(void*, void*, ssize);
} read_cb;

extern void read_cb_init(read_cb* self, void* read_fn);

typedef struct _readbuf
{
        read_cb* _cb;
        char* _pos;
        char* _end;
        char _data[RB_SIZE];
        ssize _read;
} readbuf;

extern void readbuf_init(readbuf* self, read_cb* cb);
extern void readbuf_dispose(readbuf* self);

#define RB_ENDC (-1)

// returns RB_ENDC if end of input was reached
extern int readbuf_getc(const readbuf* self);
extern int readbuf_readc(readbuf* self);
// returns bytes written
extern ssize readbuf_read_bytes(readbuf* self, void* buf, ssize len);
// assuming that buffer is at least len bytes long
extern ssize readbuf_reads(readbuf* self, char* buf, ssize len);

// returns total bytes read
static inline ssize readbuf_get_bytes_read(const readbuf* self)
{
        return self->_read;
}

typedef struct _sread_cb
{
        read_cb _base;
        const char* _pos;
        const char* _end;
} sread_cb;

extern void sread_cb_init(sread_cb* self, const char* text);

static inline read_cb* sread_cb_base(sread_cb* self)
{
        return &self->_base;
}

#ifdef __cplusplus
}
#endif

#endif
