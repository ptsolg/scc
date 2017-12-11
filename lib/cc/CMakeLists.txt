add_scc_lib(cc
	cc.c
	cc-args.c
	cc-run.c
	tool.c
	llvm.c
	linker.c

	DEPENDS
	c
	codegen

	INCLUDE
	${SCC_INC_DIR}
)
