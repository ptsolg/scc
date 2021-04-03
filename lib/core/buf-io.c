#include "scc/core/buf-io.h"
#include "scc/core/alloc.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define BUF_SIZE 4096

static void init_writer(struct buf_writer* self, int is_file, void* f)
{
        self->buf = alloc(BUF_SIZE);
        self->pos = self->buf;
        self->capacity = BUF_SIZE;
        self->is_file = is_file;
        self->f = f;
}

static inline size_t buf_rem(const struct buf_writer* self)
{
        return self->capacity - (self->pos - self->buf);
}

static inline size_t write(struct buf_writer* self, const void* data, size_t size)
{
        return self->is_file
                ? fwrite(data, 1, size, self->f)
                : self->w(self, data, size);
}

void init_buf_writer(struct buf_writer* self, FILE* f)
{
        init_writer(self, 1, f);
}

void init_custom_buf_writer(struct buf_writer* self, write_fn w)
{
        init_writer(self, 0, w);
}

void drop_buf_writer(struct buf_writer* self)
{
        flush_buf_writer(self);
        dealloc(self->buf);
        self->buf = self->pos = 0;
        self->capacity = 0;
        if (self->is_file)
                fflush(self->f);
        self->f = 0;
}

size_t buf_write(struct buf_writer* self, const void* data, size_t size)
{
        if (size <= self->capacity)
        {
                if (size > buf_rem(self) && flush_buf_writer(self))
                        return 0;
                memcpy(self->pos, data, size);
                self->pos += size;
                return size;
        }
        return flush_buf_writer(self)
                ? 0 : write(self, data, size);
}

size_t buf_write_char(struct buf_writer* self, int c)
{
        size_t n = 0;
        if (!buf_rem(self) && flush_buf_writer(self))
                return 0;
        *self->pos++ = c;
        return 1;
}

size_t buf_write_str(struct buf_writer* self, const char* s)
{
        return buf_write(self, s, strlen(s));
}

int flush_buf_writer(struct buf_writer* self)
{
        size_t len = self->pos - self->buf;
        if (!len)
                return 0;
        if (write(self, self->buf, len) != len)
                return -1;
        self->pos = self->buf;
        return 0;
}


static void init_reader(struct buf_reader* self, int is_file, void* f)
{
        self->buf = alloc(BUF_SIZE);
        self->pos = self->max_pos = self->buf;
        self->capacity = BUF_SIZE;
        self->is_file = is_file;
        self->f = f;
}

void init_buf_reader(struct buf_reader* self, FILE* f)
{
        init_reader(self, 1, f);
}

void init_custom_buf_reader(struct buf_reader* self, read_fn r)
{
        init_reader(self, 0, r);
}

static size_t read_none(void* self, void* buf, size_t n)
{
        return 0;
}

void drop_buf_reader(struct buf_reader* self)
{
        dealloc(self->buf);
        self->buf = self->pos = self->max_pos = 0;
        self->capacity = 0;
        self->f = 0;
}

static inline size_t read(struct buf_reader* self, void* dst, size_t size)
{
        return self->is_file
                ? fread(dst, 1, size, self->f)
                : self->r(self, dst, size);
}

static void buf_reader_maybe_fill(struct buf_reader* self)
{
        if (self->pos != self->max_pos)
                return;
        size_t nread = read(self, self->buf, self->capacity);
        self->pos = self->buf;
        self->max_pos = self->buf + nread;
}

size_t buf_read(struct buf_reader* self, void* data, size_t size)
{
        buf_reader_maybe_fill(self);
        size_t rem = self->max_pos - self->pos;
        if (size > rem)
        {
                memcpy(data, self->pos, rem);
                self->pos = self->max_pos;
                data += rem;
                size -= rem;
                return rem + read(self, data, size);
        }

        memcpy(data, self->pos, size);
        self->pos += size;
        return size;
}

int buf_read_char(struct buf_reader* self)
{
        buf_reader_maybe_fill(self);
        return self->pos == self->max_pos
                ? -1 : *self->pos++;
}
