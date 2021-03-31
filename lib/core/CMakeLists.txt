add_scc_lib(core
	alloc.c
	allocator.c
	cmd.c
	file.c
	num.c
	read-write.c
	strpool.c

	INCLUDE
	${SCC_INC_DIR}

	LINK
	shlwapi
)