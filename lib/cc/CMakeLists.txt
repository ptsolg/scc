add_scc_lib(cc
	cc.c
	cc-impl.h
	cc-impl.c
	llvm.c

	DEPENDS
	c
	ssa

	INCLUDE
	${SCC_INC_DIR}
)
