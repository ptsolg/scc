add_scc_lib(ssa-emit
	emit-const.c
	emit-decl.c
	emit-expr.c
	emit-stmt.c
	emitter.c
	emitter.h
	finish-module.c

	DEPENDS
	ssa
	ssa-optimize

	INCLUDE
	${SCC_INC_DIR}
)
