#include "scc/core/read-write.h"
#include "scc/core/string.h"

extern void write_cb_init(write_cb* self, void* write_fn)
{
        self->_write = write_fn;
}

extern void writebuf_init(writebuf* self, write_cb* cb)
{
        S_ASSERT(cb && cb->_write);
        self->_cb = cb;
        self->_written = 0;
        self->_pos = self->_data;
}

extern void writebuf_dispose(writebuf* self)
{
        ;
}

static inline size_t writebuf_write(writebuf* self, const void* data, size_t len)
{
        if (!len)
                return 0;

        size_t bytes = self->_cb->_write(self->_cb, data, len);
        self->_written += bytes;
        return bytes;
}

extern size_t writebuf_flush(writebuf* self)
{
        size_t bytes = writebuf_write(self, self->_data, self->_pos - self->_data);
        self->_pos = self->_data;
        return bytes;
}

static inline void writebuf_fill(writebuf* self, const void* b, size_t len)
{
        size_t free = WB_SIZE - (self->_pos - self->_data);
        S_ASSERT(len <= free);

        memcpy(self->_pos, b, len);
        self->_pos += len;
}

extern size_t writebuf_write_bytes(writebuf* self, const void* b, size_t len)
{
        if (len > WB_SIZE)
        {
                size_t bytes = writebuf_flush(self);
                bytes += writebuf_write(self, b, len);
                return bytes;
        }

        size_t free = WB_SIZE - (self->_pos - self->_data + 1);
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
        if (self->_pos - self->_data + 1 == WB_SIZE)
                bytes = writebuf_flush(self);

        *self->_pos++ = (char)c;
        return bytes;
}

static size_t snwrite_cb_write(snwrite_cb* self, const void* data, size_t bytes)
{
        size_t written = self->_pos - self->_begin;
        S_ASSERT(written <= self->_n);

        size_t available = self->_n - written;

        if (bytes > available)
                bytes = available;

        if (!bytes)
                return 0;

        memcpy(self->_pos, data, bytes - 1);
        self->_pos += bytes;
        self->_pos[bytes] = '\0';

        return bytes;
}

extern void snwrite_cb_init(snwrite_cb* self, char* buf, size_t n)
{
        self->_n = n;
        self->_pos = buf;
        self->_begin = buf;
        write_cb_init(snwrite_cb_base(self), &snwrite_cb_write);
}

extern void read_cb_init(read_cb* self, void* read_fn)
{
        self->_read = read_fn;
}

extern void readbuf_init(readbuf* self, read_cb* cb)
{
        S_ASSERT(cb && cb->_read);
        self->_cb = cb;
        self->_end = self->_data;
        self->_pos = self->_end;
        self->_read = 0;
}

static inline size_t readbuf_read(readbuf* self, void* buf, size_t len)
{
        if (!len)
                return 0;

        size_t bytes = self->_cb->_read(self->_cb, buf, len);
        self->_read += bytes;
        return bytes;
}

static inline size_t readbuf_read_chunk(readbuf* self)
{
        size_t bytes = readbuf_read(self, self->_data, RB_SIZE);
        self->_pos = self->_data;
        self->_end = self->_data + bytes;
        return bytes;
}

extern void readbuf_dispose(readbuf* self)
{
        ;
}

extern int readbuf_getc(const readbuf* self)
{
        return *self->_pos;
}

extern int readbuf_readc(readbuf* self)
{
        if (self->_pos == self->_end)
                if (!readbuf_read_chunk(self))
                        return RB_ENDC;

        return *self->_pos++;
}

static inline void readbuf_flush(readbuf* self, void* buf, size_t len)
{
        if (!len)
                return;

        memcpy(buf, self->_pos, len);
        self->_pos += len;
        S_ASSERT(self->_pos <= self->_end);
}

extern size_t readbuf_read_bytes(readbuf* self, void* buf, size_t len)
{
        if (!len)
                return 0;

        size_t available = self->_end - self->_pos;
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
        size_t rem = self->_end - self->_pos;
        if (bytes > rem)
                bytes = rem;
        memcpy(buf, self->_pos, bytes);
        self->_pos += bytes;
        return bytes;
}

extern void sread_cb_init(sread_cb* self, const char* text)
{
        self->_pos = text;
        self->_end = text + strlen(text);
        read_cb_init(&self->_base, &sread_cb_read);
}