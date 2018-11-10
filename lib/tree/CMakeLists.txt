add_scc_lib(tree
	type.c
	decl.c
	context.c
	stmt.c
	expr.c
	module.c
	target.c
	eval.c

	DEPENDS
	core

	INCLUDE
	${SCC_INC_DIR}
)
