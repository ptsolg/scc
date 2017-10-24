#include "read-write.h"
#include "sstring.h"

extern void write_cb_init(write_cb* self, ssize(*fn)(void*, const void*, ssize))
{
        self->_write = fn;
}

extern void writebuf_init(writebuf* self, write_cb* cb)
{
        S_ASSERT(cb && cb->_write);
        self->_cb      = cb;
        self->_written = 0;
        self->_pos     = self->_data;
}

extern void writebuf_dispose(writebuf* self)
{
        ;
}

static inline ssize writebuf_write(writebuf* self, const void* data, ssize len)
{
        if (!len)
                return 0;

        ssize bytes = self->_cb->_write(self->_cb, data, len);
        self->_written += bytes;
        return bytes;
}

extern ssize writebuf_flush(writebuf* self)
{
        ssize bytes = writebuf_write(self, self->_data, self->_pos - self->_data);
        self->_pos  = self->_data;
        return bytes;
}

static inline writebuf_fill(writebuf* self, const void* b, ssize len)
{
        ssize free = WB_SIZE - (self->_pos - self->_data);
        S_ASSERT(len <= free);

        memcpy(self->_pos, b, len);
        self->_pos += len;
}

extern ssize writebuf_write_bytes(writebuf* self, const void* b, ssize len)
{
        if (len > WB_SIZE)
        {
                ssize bytes = writebuf_flush(self);
                bytes      += writebuf_write(self, b, len);
                return bytes;
        }

        ssize free = WB_SIZE - (self->_pos - self->_data + 1);
        if (len <= free)
        {
                writebuf_fill(self, b, len);
                return 0;
        }

        writebuf_fill(self, b, free);
        ssize flushed = writebuf_flush(self);

        ssize remain = len - free;
        writebuf_fill(self, (char*)b + free, remain);
        return flushed;
}

extern ssize writebuf_writes(writebuf* self, const char* s)
{
        return writebuf_write_bytes(self, s, strlen(s));
}

extern ssize writebuf_writec(writebuf* self, int c)
{
        ssize bytes = 0;
        if (self->_pos - self->_data + 1 == WB_SIZE)
                bytes = writebuf_flush(self);

        *self->_pos++ = c;
        return bytes;
}

extern void read_cb_init(read_cb* self, ssize(*read)(void*, void*, ssize))
{
        self->_read = read;
}

extern void readbuf_init(readbuf* self, read_cb* cb)
{
        S_ASSERT(cb && cb->_read);
        self->_cb   = cb;
        self->_end  = self->_data;
        self->_pos  = self->_end;
        self->_read = 0;
}

static inline ssize readbuf_read(readbuf* self, void* buf, ssize len)
{
        if (!len)
                return 0;

        ssize bytes = self->_cb->_read(self->_cb, buf, len);
        self->_read += bytes;
        return bytes;
}

static inline ssize readbuf_read_chunk(readbuf* self)
{
        ssize bytes = readbuf_read(self, self->_data, RB_SIZE);
        self->_pos  = self->_data;
        self->_end  = self->_data + bytes;
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

static inline void readbuf_flush(readbuf* self, void* buf, ssize len)
{
        if (!len)
                return;

        memcpy(buf, self->_pos, len);
        self->_pos += len;
        S_ASSERT(self->_pos <= self->_end);
}

extern ssize readbuf_read_bytes(readbuf* self, void* buf, ssize len)
{
        if (!len)
                return 0;

        ssize available = self->_end - self->_pos;
        if (len >= RB_SIZE)
        {
                readbuf_flush(self, buf, available);
                return readbuf_read(self, (char*)buf + available, len - available)
                        + available;
        }

        readbuf_flush(self, buf, available);
        ssize written = available;
        ssize remain  = len - available;
        available     = readbuf_read_chunk(self);

        if (available <= len)
        {
                readbuf_flush(self, buf, available);
                return written + available;
        }

        readbuf_flush(self, buf, remain);
        return len;
}

extern ssize readbuf_reads(readbuf* self, char* buf, ssize len)
{
        ssize read = readbuf_read_bytes(self, buf, len);
        buf[read]  = '\0';
        return read;
}

static ssize sread_cb_read(sread_cb* self, void* buf, ssize bytes)
{
        ssize rem = self->_end - self->_pos;
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