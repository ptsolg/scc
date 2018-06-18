#ifndef PTHREAD_H
#define PTHREAD_H

typedef void* pthread_t;
typedef void* pthread_attr_t;

extern int pthread_create(
	pthread_t* thread, const pthread_attr_t* att, void*(*entry)(void*), void* arg);

extern int pthread_join(pthread_t thread, void** value_ptr);

#endif
