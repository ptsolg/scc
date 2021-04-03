add_scc_lib(core
	alloc.c
	allocator.c
	buf-io.c
	cmd.c
	file.c
	num.c
	strpool.c

	INCLUDE
	${SCC_INC_DIR}

	LINK
	shlwapi
)