add_scc_lib(compiler
	args.c
	instance.c

	DEPENDS
	c
	ssa

	INCLUDE
	${SCC_INC_DIR}
)
