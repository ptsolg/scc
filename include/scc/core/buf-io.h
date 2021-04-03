#ifndef BUF_IO_H
#define BUF_IO_H

#include <stddef.h>
#include <stdio.h>

typedef size_t(*write_fn)(void*, const void*, size_t);

struct buf_writer
{
        char* pos;
        char* buf;
        size_t capacity;

        int is_file;
        union
        {
                FILE* f;
                write_fn w;
        };
};

void init_buf_writer(struct buf_writer* self, FILE* f);
void init_custom_buf_writer(struct buf_writer* self, write_fn w);
void drop_buf_writer(struct buf_writer* self);
size_t buf_write(struct buf_writer* self, const void* data, size_t size);
size_t buf_write_char(struct buf_writer* self, int c);
size_t buf_write_str(struct buf_writer* self, const char* s);
int flush_buf_writer(struct buf_writer* self);

typedef size_t(*read_fn)(void*, void*, size_t);

struct buf_reader
{
        char* pos;
        char* max_pos;
        char* buf;
        size_t capacity;
        
        int is_file;
        union
        {
                FILE* f;
                read_fn r;
        };
};

void init_buf_reader(struct buf_reader* self, FILE* f);
void init_custom_buf_reader(struct buf_reader* self, read_fn r);
void drop_buf_reader(struct buf_reader* self);
size_t buf_read(struct buf_reader* self, void* data, size_t size);
int buf_read_char(struct buf_reader* self);

#endif
