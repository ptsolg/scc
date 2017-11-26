add_scc_lib(cc
	cc.c
	cc-args.c

	DEPENDS
	c
	ssa
	codegen

	INCLUDE
	${SCC_INC_DIR}
)
