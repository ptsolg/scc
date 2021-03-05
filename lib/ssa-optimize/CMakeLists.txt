add_scc_lib(ssa-optimize
	alloca-promotion.c
	constant-fold.c
	dead-code-elimination.c
	inline.c
	optimize.c

	DEPENDS
	ssa

	INCLUDE
	${SCC_INC_DIR}
)
