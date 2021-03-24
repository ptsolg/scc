#include "scc/core/read-write.h"
#include <string.h>

extern void write_cb_init(write_cb* self, void* write_fn)
{
        self->write = write_fn;
}

extern void writebuf_init(writebuf* self, write_cb* cb)
{
        assert(cb && cb->write);
        self->cb = cb;
        self->written = 0;
        self->pos = self->data;
}

extern void writebuf_dispose(writebuf* self)
{
        ;
}

static inline size_t writebuf_write(writebuf* self, const void* data, size_t len)
{
        size_t bytes = self->cb->write(self->cb, data, len);
        self->written += bytes;
        return bytes;
}

extern size_t writebuf_flush(writebuf* self)
{
        size_t bytes = writebuf_write(self, self->data, self->pos - self->data);
        self->pos = self->data;
        return bytes;
}

static inline void writebuf_fill(writebuf* self, const void* b, size_t len)
{
        size_t free = WB_SIZE - (self->pos - self->data);
        assert(len <= free);

        memcpy(self->pos, b, len);
        self->pos += len;
}

extern size_t writebuf_write_bytes(writebuf* self, const void* b, size_t len)
{
        if (len > WB_SIZE)
        {
                size_t bytes = writebuf_flush(self);
                bytes += writebuf_write(self, b, len);
                return bytes;
        }

        size_t free = WB_SIZE - (self->pos - self->data + 1);
        if (len <= free)
        {
                writebuf_fill(self, b, len);
                return 0;
        }

        writebuf_fill(self, b, free);
        size_t flushed = writebuf_flush(self);

        size_t remain = len - free;
        writebuf_fill(self, (char*)b + free, remain);
        return flushed;
}

extern size_t writebuf_writes(writebuf* self, const char* s)
{
        return writebuf_write_bytes(self, s, strlen(s));
}

extern size_t writebuf_writec(writebuf* self, int c)
{
        size_t bytes = 0;
        if (self->pos - self->data + 1 == WB_SIZE)
                bytes = writebuf_flush(self);

        *self->pos++ = (char)c;
        return bytes;
}

static size_t snwrite_cb_write(snwrite_cb* self, const void* data, size_t bytes)
{
        size_t written = self->pos - self->begin;
        assert(written <= self->n);

        size_t available = self->n - written;
        if (!available)
                return 0;

        if (bytes > available)
                bytes = available;

        memcpy(self->pos, data, bytes);
        self->pos += bytes;
        *self->pos = '\0';

        return bytes + 1;
}

extern void snwrite_cb_init(snwrite_cb* self, char* buf, size_t buf_size)
{
        self->n = buf_size ? buf_size - 1 : 0; // reserve space for \0
        self->pos = buf;
        self->begin = buf;
        write_cb_init(snwrite_cb_base(self), &snwrite_cb_write);
}

extern void read_cb_init(read_cb* self, void* read_fn)
{
        self->read = read_fn;
}

extern void readbuf_init(readbuf* self, read_cb* cb)
{
        assert(cb && cb->read);
        self->cb = cb;
        self->end = self->data;
        self->pos = self->end;
        self->read = 0;
}

static inline size_t readbuf_read(readbuf* self, void* buf, size_t len)
{
        if (!len)
                return 0;

        size_t bytes = self->cb->read(self->cb, buf, len);
        self->read += bytes;
        return bytes;
}

static inline size_t readbuf_read_chunk(readbuf* self)
{
        size_t bytes = readbuf_read(self, self->data, RB_SIZE);
        self->pos = self->data;
        self->end = self->data + bytes;
        return bytes;
}

extern void readbuf_dispose(readbuf* self)
{
        ;
}

extern int readbuf_getc(const readbuf* self)
{
        return *self->pos;
}

extern int readbuf_readc(readbuf* self)
{
        if (self->pos == self->end)
                if (!readbuf_read_chunk(self))
                        return RB_ENDC;

        return *self->pos++;
}

static inline void readbuf_flush(readbuf* self, void* buf, size_t len)
{
        if (!len)
                return;

        memcpy(buf, self->pos, len);
        self->pos += len;
        assert(self->pos <= self->end);
}

extern size_t readbuf_read_bytes(readbuf* self, void* buf, size_t len)
{
        if (!len)
                return 0;

        size_t available = self->end - self->pos;
        if (len >= RB_SIZE)
        {
                readbuf_flush(self, buf, available);
                return readbuf_read(self, (char*)buf + available, len - available)
                        + available;
        }

        readbuf_flush(self, buf, available);
        size_t written = available;
        size_t remain = len - available;
        available = readbuf_read_chunk(self);

        if (available <= len)
        {
                readbuf_flush(self, buf, available);
                return written + available;
        }

        readbuf_flush(self, buf, remain);
        return len;
}

extern size_t readbuf_reads(readbuf* self, char* buf, size_t len)
{
        size_t read = readbuf_read_bytes(self, buf, len);
        buf[read] = '\0';
        return read;
}

static size_t sread_cb_read(sread_cb* self, void* buf, size_t bytes)
{
        size_t rem = self->end - self->pos;
        if (bytes > rem)
                bytes = rem;
        memcpy(buf, self->pos, bytes);
        self->pos += bytes;
        return bytes;
}

extern void sread_cb_init(sread_cb* self, const char* text)
{
        self->pos = text;
        self->end = text + strlen(text);
        read_cb_init(&self->base, &sread_cb_read);
}
