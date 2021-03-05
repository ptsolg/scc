add_scc_lib(cc
	cc.c
	cc-impl.h
	cc-impl.c
	llvm.c

	DEPENDS
	lex
	syntax
	ssa-emit

	INCLUDE
	${SCC_INC_DIR}
)
