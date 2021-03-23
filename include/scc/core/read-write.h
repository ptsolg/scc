#ifndef SCC_CORE_READ_WRITE_H
#define SCC_CORE_READ_WRITE_H

#include "common.h"

typedef struct _write_cb
{ 
        // takes itself as first argument, returns bytes written
        size_t(*write)(void*, const void*, size_t);
} write_cb;

extern void write_cb_init(write_cb* self, void* write_fn);

#ifndef WB_SIZE
#define WB_SIZE 4095
#endif

// Struct that reduces write callbacks by writing data to buffer and then calling a function
typedef struct _writebuf
{
        write_cb* cb;
        char* pos;
        char data[WB_SIZE];
        size_t written;
} writebuf;

extern void writebuf_init(writebuf* self, write_cb* cb);
extern void writebuf_dispose(writebuf* self);

// note: functions below return bytes written, it means that any of these functions
// can return 0 (since data can be written to the buffer)
// or the amount of bytes flushed (if the buffer is full)

extern size_t writebuf_flush(writebuf* self);
extern size_t writebuf_write_bytes(writebuf* self, const void* b, size_t len);
extern size_t writebuf_writes(writebuf* self, const char* s);
extern size_t writebuf_writec(writebuf* self, int c);

// returns total bytes written
static inline size_t writebuf_get_bytes_written(const writebuf* self)
{
        return self->written;
}

typedef struct _snwrite_cb
{
        write_cb base;
        char* pos;
        char* begin;
        size_t n;
} snwrite_cb;

extern void snwrite_cb_init(snwrite_cb* self, char* buf, size_t buf_size);

static inline write_cb* snwrite_cb_base(snwrite_cb* self)
{
        return &self->base;
}

#ifndef RB_SIZE
#define RB_SIZE 4095
#endif

typedef struct _read_cb
{
        // takes itself as first argument, returns bytes read
        size_t(*read)(void*, void*, size_t);
} read_cb;

extern void read_cb_init(read_cb* self, void* read_fn);

typedef struct _readbuf
{
        read_cb* cb;
        char* pos;
        char* end;
        char data[RB_SIZE];
        size_t read;
} readbuf;

extern void readbuf_init(readbuf* self, read_cb* cb);
extern void readbuf_dispose(readbuf* self);

#define RB_ENDC (-1)

// returns RB_ENDC if end of input was reached
extern int readbuf_getc(const readbuf* self);
extern int readbuf_readc(readbuf* self);
// returns bytes written
extern size_t readbuf_read_bytes(readbuf* self, void* buf, size_t len);
// assuming that buffer is at least len bytes long
extern size_t readbuf_reads(readbuf* self, char* buf, size_t len);

// returns total bytes read
static inline size_t readbuf_get_bytes_read(const readbuf* self)
{
        return self->read;
}

typedef struct _sread_cb
{
        read_cb base;
        const char* pos;
        const char* end;
} sread_cb;

extern void sread_cb_init(sread_cb* self, const char* text);

static inline read_cb* sread_cb_base(sread_cb* self)
{
        return &self->base;
}

#endif
