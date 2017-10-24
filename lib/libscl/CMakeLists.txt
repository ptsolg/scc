create_module(libscl
	FOLDER lib

	alloc.c
	alloc.h
	common.h
	error.h
	file.c
	file.h
	hash.h
	htab.c
	htab.h
	list.h
	malloc.h
	malloc.c
	misc.h
	arg-parser.h
	arg-parser.c
	membuf.h
	membuf.c
	sstring.c
	sstring.h
	target.h
	char-info.h
	char-info.c
	heap-debug.h
	heap-debug.c
	read-write.h
	read-write.c
	objgroup.h
	objgroup.c
	bit-utils.h
)

export_module(libscl)
