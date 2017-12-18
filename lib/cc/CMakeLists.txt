add_scc_lib(cc
	cc.c
	cc-impl.h
	cc-impl.c
	llvm.c

	DEPENDS
	c
	codegen

	INCLUDE
	${SCC_INC_DIR}
)
